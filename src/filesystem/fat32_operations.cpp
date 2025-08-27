#include "../include/filesystem/fat32.h"
#include "../include/libc/string.h"

namespace uqaabOS {
namespace filesystem {

void FAT32::ls(const char *path) {
  // Handle null path
  if (path == nullptr) {
    libc::printf("Error: Null path provided   \n");
    return;
  }

  // Handle empty path - default to root
  if (path[0] == '\0') {
    list_directory(root_cluster);
    return;
  }

  // Handle root directory explicitly
  if ((path[0] == '/' && path[1] == '\0') || 
      (path[0] == '.' && path[1] == '\0') ||
      (path[0] == '.' && path[1] == '.' && path[2] == '\0')) {
    list_directory(root_cluster);
    return;
  }

  // Find the directory cluster
  uint32_t dir_cluster = find_directory_cluster(path);

  if (dir_cluster == 0) {
    libc::printf("Directory not found: ");
    libc::printf(path);
    libc::printf("  \n");
    return;
  }

  // List the directory contents
  list_directory(dir_cluster);
}

bool FAT32::mkdir(const char *path) {
  // Parse the path to separate parent directory and new directory name
  char parent_path[256];
  char dirname[256];
  parse_path(path, parent_path, dirname);

  // Find the parent directory cluster
  uint32_t parent_cluster = find_directory_cluster(parent_path);
  if (parent_cluster == 0) {
    libc::printf("Parent directory not found: ");
    libc::printf(parent_path);
    libc::printf("  \n");
    return false;
  }

  // Check if directory already exists
  DirectoryEntryFat32 existing_entry;
  uint32_t entry_cluster, entry_offset;
  if (find_file_in_directory(parent_cluster, dirname, &existing_entry,
                             &entry_cluster, &entry_offset)) {
    libc::printf("Directory already exists: ");
    libc::printf(path);
    libc::printf("  \n");
    return false;
  }

  // Find a free cluster for the new directory
  uint32_t new_cluster;
  if (!allocate_cluster(&new_cluster)) {
    libc::printf("Failed to allocate cluster for new directory  \n");
    return false;
  }

  // Create the directory entries for "." and ".."
  uint8_t dir_buffer[512 * 32]; // Buffer for the new directory cluster
  libc::memset(dir_buffer, 0, sizeof(dir_buffer));

  DirectoryEntryFat32 *dir_entries = (DirectoryEntryFat32 *)dir_buffer;

  // "." entry (points to itself)
  libc::memset(&dir_entries[0], 0, sizeof(DirectoryEntryFat32));
  dir_entries[0].name[0] = '.';
  for (int i = 1; i < 8; i++)
    dir_entries[0].name[i] = ' ';
  for (int i = 0; i < 3; i++)
    dir_entries[0].ext[i] = ' ';
  dir_entries[0].attributes = 0x10; // Directory attribute
  dir_entries[0].first_cluster_hi = (new_cluster >> 16) & 0xFFFF;
  dir_entries[0].first_cluster_low = new_cluster & 0xFFFF;

  // ".." entry (points to parent)
  libc::memset(&dir_entries[1], 0, sizeof(DirectoryEntryFat32));
  dir_entries[1].name[0] = '.';
  dir_entries[1].name[1] = '.';
  for (int i = 2; i < 8; i++)
    dir_entries[1].name[i] = ' ';
  for (int i = 0; i < 3; i++)
    dir_entries[1].ext[i] = ' ';
  dir_entries[1].attributes = 0x10; // Directory attribute
  dir_entries[1].first_cluster_hi = (parent_cluster >> 16) & 0xFFFF;
  dir_entries[1].first_cluster_low = parent_cluster & 0xFFFF;

  // Write the new directory cluster
  write_cluster(new_cluster, dir_buffer);

  // Find a free entry in the parent directory
  uint8_t parent_buffer[512 * 32];
  uint32_t current_cluster = parent_cluster;
  bool entry_found = false;
  uint32_t free_entry_cluster = 0;
  uint32_t free_entry_offset = 0;

  while (current_cluster != 0 && !entry_found) {
    // Read the current cluster
    read_cluster(current_cluster, parent_buffer);

    // Process directory entries in this cluster
    DirectoryEntryFat32 *dir_entry = (DirectoryEntryFat32 *)parent_buffer;
    for (int i = 0;
         i < (512 * bpb.sector_per_cluster) / sizeof(DirectoryEntryFat32);
         i++) {
      // Check for free entry (deleted or end of directory)
      if (dir_entry[i].name[0] == 0x00 || dir_entry[i].name[0] == 0xE5) {
        free_entry_cluster = current_cluster;
        free_entry_offset = i * sizeof(DirectoryEntryFat32);
        entry_found = true;
        break;
      }
    }

    if (!entry_found) {
      // Get next cluster in the chain
      uint32_t next_cluster = get_next_cluster(current_cluster);
      if (next_cluster == 0) {
        // Need to allocate a new cluster for the parent directory
        if (!allocate_cluster(&next_cluster)) {
          libc::printf("Failed to allocate cluster for parent directory  \n");
          free_cluster_chain(new_cluster); // Clean up
          return false;
        }
        // Link the new cluster to the chain
        set_next_cluster(current_cluster, next_cluster);
        // Initialize new cluster with zeros
        uint8_t zero_buffer[512 * 32];
        libc::memset(zero_buffer, 0, sizeof(zero_buffer));
        write_cluster(next_cluster, zero_buffer);
        // Set the new cluster as the current cluster
        current_cluster = next_cluster;
        // Retry with the new cluster
        read_cluster(current_cluster, parent_buffer);
        free_entry_cluster = current_cluster;
        free_entry_offset = 0;
        entry_found = true;
      } else {
        current_cluster = next_cluster;
      }
    }
  }

  if (!entry_found) {
    libc::printf("Failed to find free entry in parent directory  \n");
    free_cluster_chain(new_cluster); // Clean up
    return false;
  }

  // Read the cluster containing the free entry
  read_sector(cluster_to_lba(free_entry_cluster) + (free_entry_offset / 512),
              parent_buffer);

  // Create the new directory entry
  DirectoryEntryFat32 *new_entry =
      (DirectoryEntryFat32 *)(parent_buffer + (free_entry_offset % 512));
  libc::memset(new_entry, 0, sizeof(DirectoryEntryFat32));

  // Format the directory name (8.3 format)
  int name_len = 0;
  for (int i = 0; i < 8 && dirname[i] != '\0' && dirname[i] != '.'; i++) {
    new_entry->name[i] = dirname[i];
    name_len++;
  }
  for (int i = name_len; i < 8; i++) {
    new_entry->name[i] = ' ';
  }

  // Handle extension if present
  // Find the dot character manually
  char *dot = nullptr;
  for (int i = 0; dirname[i] != '\0'; i++) {
    if (dirname[i] == '.') {
      dot = const_cast<char *>(&dirname[i]);
      break;
    }
  }
  if (dot != nullptr) {
    for (int i = 0; i < 3 && dot[i + 1] != '\0'; i++) {
      new_entry->ext[i] = dot[i + 1];
    }
    for (int i = libc::strlen(dot + 1); i < 3; i++) {
      new_entry->ext[i] = ' ';
    }
  } else {
    for (int i = 0; i < 3; i++) {
      new_entry->ext[i] = ' ';
    }
  }

  new_entry->attributes = 0x10; // Directory attribute
  new_entry->first_cluster_hi = (new_cluster >> 16) & 0xFFFF;
  new_entry->first_cluster_low = new_cluster & 0xFFFF;

  // Write the updated sector back
  write_sector(cluster_to_lba(free_entry_cluster) + (free_entry_offset / 512),
               parent_buffer);

  libc::printf("Directory created: ");
  libc::printf(path);
  libc::printf("  \n");
  return true;
}

bool FAT32::touch(const char *path) {
  // Parse the path to separate parent directory and filename
  char parent_path[256];
  char filename[256];
  parse_path(path, parent_path, filename);

  // Find the parent directory cluster
  uint32_t parent_cluster = find_directory_cluster(parent_path);
  if (parent_cluster == 0) {
    libc::printf("Parent directory not found: ");
    libc::printf(parent_path);
    libc::printf("  \n");
    return false;
  }

  // Check if file already exists
  DirectoryEntryFat32 existing_entry;
  uint32_t entry_cluster, entry_offset;
  if (find_file_in_directory(parent_cluster, filename, &existing_entry,
                             &entry_cluster, &entry_offset)) {
    libc::printf("File already exists: ");
    libc::printf(path);
    libc::printf("  \n");
    return false;
  }

  // Find a free entry in the parent directory
  uint8_t parent_buffer[512 * 32];
  uint32_t current_cluster = parent_cluster;
  bool entry_found = false;
  uint32_t free_entry_cluster = 0;
  uint32_t free_entry_offset = 0;

  while (current_cluster != 0 && !entry_found) {
    // Read the current cluster
    read_cluster(current_cluster, parent_buffer);

    // Process directory entries in this cluster
    DirectoryEntryFat32 *dir_entry = (DirectoryEntryFat32 *)parent_buffer;
    for (int i = 0;
         i < (512 * bpb.sector_per_cluster) / sizeof(DirectoryEntryFat32);
         i++) {
      // Check for free entry (deleted or end of directory)
      if (dir_entry[i].name[0] == 0x00 || dir_entry[i].name[0] == 0xE5) {
        free_entry_cluster = current_cluster;
        free_entry_offset = i * sizeof(DirectoryEntryFat32);
        entry_found = true;
        break;
      }
    }

    if (!entry_found) {
      // Get next cluster in the chain
      uint32_t next_cluster = get_next_cluster(current_cluster);
      if (next_cluster == 0) {
        // Need to allocate a new cluster for the parent directory
        if (!allocate_cluster(&next_cluster)) {
          libc::printf("Failed to allocate cluster for parent directory  \n");
          return false;
        }
        // Link the new cluster to the chain
        set_next_cluster(current_cluster, next_cluster);
        // Initialize new cluster with zeros
        uint8_t zero_buffer[512 * 32];
        libc::memset(zero_buffer, 0, sizeof(zero_buffer));
        write_cluster(next_cluster, zero_buffer);
        // Set the new cluster as the current cluster
        current_cluster = next_cluster;
        // Retry with the new cluster
        read_cluster(current_cluster, parent_buffer);
        free_entry_cluster = current_cluster;
        free_entry_offset = 0;
        entry_found = true;
      } else {
        current_cluster = next_cluster;
      }
    }
  }

  if (!entry_found) {
    libc::printf("Failed to find free entry in parent directory  \n");
    return false;
  }

  // Read the sector containing the free entry
  read_sector(cluster_to_lba(free_entry_cluster) + (free_entry_offset / 512),
              parent_buffer);

  // Create the new file entry
  DirectoryEntryFat32 *new_entry =
      (DirectoryEntryFat32 *)(parent_buffer + (free_entry_offset % 512));
  libc::memset(new_entry, 0, sizeof(DirectoryEntryFat32));

  // Format the filename (8.3 format)
  int name_len = 0;
  for (int i = 0; i < 8 && filename[i] != '\0' && filename[i] != '.'; i++) {
    new_entry->name[i] = filename[i];
    name_len++;
  }
  for (int i = name_len; i < 8; i++) {
    new_entry->name[i] = ' ';
  }

  // Handle extension if present
  // Find the dot character manually
  char *dot = nullptr;
  for (int i = 0; filename[i] != '\0'; i++) {
    if (filename[i] == '.') {
      dot = const_cast<char *>(&filename[i]);
      break;
    }
  }
  if (dot != nullptr) {
    for (int i = 0; i < 3 && dot[i + 1] != '\0'; i++) {
      new_entry->ext[i] = dot[i + 1];
    }
    for (int i = libc::strlen(dot + 1); i < 3; i++) {
      new_entry->ext[i] = ' ';
    }
  } else {
    for (int i = 0; i < 3; i++) {
      new_entry->ext[i] = ' ';
    }
  }

  // File attributes (normal file)
  new_entry->attributes = 0x20; // Archive attribute
  new_entry->size = 0;          // Empty file

  // Write the updated sector back
  write_sector(cluster_to_lba(free_entry_cluster) + (free_entry_offset / 512),
               parent_buffer);

  libc::printf("File created: ");
  libc::printf(path);
  libc::printf("  \n");
  return true;
}

bool FAT32::rm(const char *path) {
  // Parse the path to separate parent directory and filename
  char parent_path[256];
  char filename[256];
  parse_path(path, parent_path, filename);

  // Find the parent directory cluster
  uint32_t parent_cluster = find_directory_cluster(parent_path);
  if (parent_cluster == 0) {
    libc::printf("Parent directory not found: ");
    libc::printf(parent_path);
    libc::printf("  \n");
    return false;
  }

  // Find the file entry
  DirectoryEntryFat32 entry;
  uint32_t entry_cluster, entry_offset;
  if (!find_file_in_directory(parent_cluster, filename, &entry, &entry_cluster,
                              &entry_offset)) {
    libc::printf("File not found: ");
    libc::printf(path);
    libc::printf("  \n");
    return false;
  }

  // Check if it's actually a file (not a directory)
  if (entry.attributes & 0x10) {
    libc::printf("Path is a directory, not a file: ");
    libc::printf(path);
    libc::printf("  \n");
    return false;
  }

  // Get the first cluster of the file
  uint32_t first_cluster = ((uint32_t)entry.first_cluster_hi << 16) |
                           ((uint32_t)entry.first_cluster_low);

  // Free the cluster chain
  if (first_cluster != 0) {
    free_cluster_chain(first_cluster);
  }

  // Mark the directory entry as deleted
  uint8_t sector_buffer[512];
  uint32_t sector_lba = cluster_to_lba(entry_cluster) + (entry_offset / 512);
  read_sector(sector_lba, sector_buffer);

  // Mark entry as deleted
  sector_buffer[entry_offset % 512] = 0xE5;

  // Write the sector back
  write_sector(sector_lba, sector_buffer);

  libc::printf("File deleted: ");
  libc::printf(path);
  libc::printf("  \n");
  return true;
}

bool FAT32::rmdir(const char *path) {
  // Parse the path to separate parent directory and directory name
  char parent_path[256];
  char dirname[256];
  parse_path(path, parent_path, dirname);

  // Find the parent directory cluster
  uint32_t parent_cluster = find_directory_cluster(parent_path);
  if (parent_cluster == 0) {
    libc::printf("Parent directory not found: ");
    libc::printf(parent_path);
    libc::printf("  \n");
    return false;
  }

  // Find the directory entry
  DirectoryEntryFat32 entry;
  uint32_t entry_cluster, entry_offset;
  if (!find_file_in_directory(parent_cluster, dirname, &entry, &entry_cluster,
                              &entry_offset)) {
    libc::printf("Directory not found: ");
    libc::printf(path);
    libc::printf("  \n");
    return false;
  }

  // Check if it's actually a directory
  if (!(entry.attributes & 0x10)) {
    libc::printf("Path is a file, not a directory: ");
    libc::printf(path);
    libc::printf("  \n");
    return false;
  }

  // Get the first cluster of the directory
  uint32_t first_cluster = ((uint32_t)entry.first_cluster_hi << 16) |
                           ((uint32_t)entry.first_cluster_low);

  // Check if directory is empty (only "." and ".." entries)
  bool is_empty = true;
  uint8_t dir_buffer[512 * 32];
  uint32_t current_cluster = first_cluster;

  while (current_cluster != 0 && is_empty) {
    // Read the current cluster
    read_cluster(current_cluster, dir_buffer);

    // Process directory entries in this cluster
    DirectoryEntryFat32 *dir_entry = (DirectoryEntryFat32 *)dir_buffer;
    for (int i = 0;
         i < (512 * bpb.sector_per_cluster) / sizeof(DirectoryEntryFat32);
         i++) {
      // Check for end of directory
      if (dir_entry[i].name[0] == 0x00) {
        break;
      }

      // Skip deleted entries
      if (dir_entry[i].name[0] == 0xE5) {
        continue;
      }

      // Skip "." and ".." entries
      if (dir_entry[i].name[0] == '.' &&
          (dir_entry[i].name[1] == ' ' || dir_entry[i].name[1] == '.')) {
        continue;
      }

      // Found a non-special entry, directory is not empty
      is_empty = false;
      break;
    }

    if (is_empty) {
      // Get next cluster in the chain
      current_cluster = get_next_cluster(current_cluster);
    }
  }

  if (!is_empty) {
    // Recursively delete all entries in the directory
    current_cluster = first_cluster;

    while (current_cluster != 0) {
      // Read the current cluster
      read_cluster(current_cluster, dir_buffer);

      // Process directory entries in this cluster
      DirectoryEntryFat32 *dir_entry = (DirectoryEntryFat32 *)dir_buffer;
      for (int i = 0;
           i < (512 * bpb.sector_per_cluster) / sizeof(DirectoryEntryFat32);
           i++) {
        // Check for end of directory
        if (dir_entry[i].name[0] == 0x00) {
          break;
        }

        // Skip deleted entries
        if (dir_entry[i].name[0] == 0xE5) {
          continue;
        }

        // Skip "." and ".." entries
        if (dir_entry[i].name[0] == '.' &&
            (dir_entry[i].name[1] == ' ' || dir_entry[i].name[1] == '.')) {
          continue;
        }

        // Format the entry name
        char entry_name[256]; // Long enough for full path
        int name_len = 0;

        // Copy name part (8 characters)
        for (int j = 0; j < 8 && dir_entry[i].name[j] != ' '; j++) {
          entry_name[name_len++] = dir_entry[i].name[j];
        }

        // Add dot if there's an extension
        if (dir_entry[i].ext[0] != ' ') {
          entry_name[name_len++] = '.';

          // Copy extension part (3 characters)
          for (int j = 0; j < 3 && dir_entry[i].ext[j] != ' '; j++) {
            entry_name[name_len++] = dir_entry[i].ext[j];
          }
        }

        entry_name[name_len] = '\0';

        // Build full path for this entry
        char full_path[512];
        if (path[0] == '/') {
          libc::strncpy(full_path, path, 511);
        } else {
          full_path[0] = '/';
          libc::strncpy(full_path + 1, path, 510);
        }

        int path_len = libc::strlen(full_path);
        if (full_path[path_len - 1] != '/') {
          full_path[path_len] = '/';
          path_len++;
        }
        libc::strncpy(full_path + path_len, entry_name, 511 - path_len);

        // Delete the entry based on its type
        if (dir_entry[i].attributes & 0x10) {
          // It's a directory
          rmdir(full_path);
        } else {
          // It's a file
          rm(full_path);
        }
      }

      // Get next cluster in the chain
      current_cluster = get_next_cluster(current_cluster);
    }
  }

  // Free the cluster chain of the directory itself
  if (first_cluster != 0) {
    free_cluster_chain(first_cluster);
  }

  // Mark the directory entry as deleted
  uint8_t sector_buffer[512];
  uint32_t sector_lba = cluster_to_lba(entry_cluster) + (entry_offset / 512);
  read_sector(sector_lba, sector_buffer);

  // Mark entry as deleted
  sector_buffer[entry_offset % 512] = 0xE5;

  // Write the sector back
  write_sector(sector_lba, sector_buffer);

  libc::printf("Directory deleted: ");
  libc::printf(path);
  libc::printf("  \n");
  return true;
}

} // namespace filesystem
} // namespace uqaabOS