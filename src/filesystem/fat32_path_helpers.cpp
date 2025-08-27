#include "../include/filesystem/fat32.h"
#include "../include/libc/string.h"

namespace uqaabOS {
namespace filesystem {

bool FAT32::parse_path(const char *path, char *parent_dir, char *filename) {
  // Validate inputs
  if (path == nullptr || parent_dir == nullptr || filename == nullptr) {
    return false;
  }

  // Initialize outputs
  parent_dir[0] = '\0';
  filename[0] = '\0';

  // Handle empty path
  if (path[0] == '\0') {
    return true;
  }

  // Find the last slash in the path
  int last_slash = -1;
  int path_len = 0;
  
  // Calculate path length and find last slash
  for (int i = 0; path[i] != '\0'; i++) {
    if (path[i] == '/') {
      last_slash = i;
    }
    path_len++;
  }

  if (last_slash == -1) {
    // No slash found, path is just a filename in root
    parent_dir[0] = '\0';
    libc::strncpy(filename, path, 255);
    filename[255] = '\0'; // Ensure null termination
  } else {
    // Copy parent directory path
    if (last_slash == 0) {
      // Path starts with '/', so parent is root
      parent_dir[0] = '\0';
    } else {
      // Limit the length to prevent buffer overflow
      int copy_len = (last_slash < 255) ? last_slash : 255;
      libc::strncpy(parent_dir, path, copy_len);
      parent_dir[copy_len] = '\0';
    }

    // Copy filename
    int filename_start = last_slash + 1;
    if (filename_start < path_len) {
      int filename_len = path_len - filename_start;
      if (filename_len > 255) filename_len = 255;
      libc::strncpy(filename, path + filename_start, filename_len);
      filename[filename_len] = '\0';
    } else {
      filename[0] = '\0'; // Empty filename
    }
  }

  return true;
}

uint32_t FAT32::find_directory_cluster(const char *path) {
  // Handle null path
  if (path == nullptr) {
    return 0; // Invalid path
  }

  // Handle root directory
  if (path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
    return root_cluster;
  }

  // Remove leading slash if present
  const char *current_path = path;
  if (path[0] == '/') {
    current_path = path + 1;
  }

  // Start with root cluster
  uint32_t current_cluster = root_cluster;

  // Parse path components
  char component[256];
  int start = 0;
  int i = 0;

  while (true) {
    if (current_path[i] == '/' || current_path[i] == '\0') {
      // Extract component
      int len = i - start;
      if (len >= 256)
        len = 255;
      if (len <= 0) {
        // Handle empty component
        if (current_path[i] == '\0')
          break;
        start = i + 1;
        i++;
        continue;
      }
      
      libc::strncpy(component, current_path + start, len);
      component[len] = '\0';

      // If component is empty, skip
      if (len == 0) {
        if (current_path[i] == '\0')
          break;
        start = i + 1;
        i++;
        continue;
      }

      // Find this component in the current directory
      DirectoryEntryFat32 entry;
      uint32_t entry_cluster, entry_offset;

      if (!find_file_in_directory(current_cluster, component, &entry,
                                  &entry_cluster, &entry_offset)) {
        return 0; // Directory not found
      }

      // Check if it's a directory
      if (!(entry.attributes & 0x10)) {
        return 0; // Not a directory
      }

      // Get the cluster of this directory
      current_cluster = ((uint32_t)entry.first_cluster_hi << 16) |
                        ((uint32_t)entry.first_cluster_low);
                        
      // Validate cluster
      if (current_cluster == 0) {
        return 0; // Invalid cluster
      }

      // If this was the last component, we're done
      if (current_path[i] == '\0') {
        break;
      }

      // Move to next component
      start = i + 1;
    }
    i++;
  }

  return current_cluster;
}

bool FAT32::find_file_in_directory(uint32_t dir_cluster, const char *name,
                                   DirectoryEntryFat32 *entry,
                                   uint32_t *entry_cluster,
                                   uint32_t *entry_offset) {
  // Validate inputs
  if (name == nullptr || entry == nullptr || entry_cluster == nullptr || entry_offset == nullptr) {
    return false;
  }

  // Buffer to hold directory cluster data
  uint8_t buffer[512 * 32]; // Assuming max 32 sectors per cluster

  // Start with the given cluster
  uint32_t current_cluster = dir_cluster;

  // Process clusters in the directory chain
  while (current_cluster != 0) {
    // Read the current cluster
    if (!read_cluster(current_cluster, buffer)) {
      return false; // Error reading cluster
    }

    // Process directory entries in this cluster
    DirectoryEntryFat32 *dir_entry = (DirectoryEntryFat32 *)buffer;
    int entries_per_cluster = (512 * bpb.sector_per_cluster) / sizeof(DirectoryEntryFat32);
    
    for (int i = 0; i < entries_per_cluster; i++) {
      // Check for end of directory
      if (dir_entry[i].name[0] == 0x00) {
        break;
      }

      // Skip deleted entries
      if (dir_entry[i].name[0] == 0xE5) {
        continue;
      }

      // Skip long filename entries
      if ((dir_entry[i].attributes & 0x0F) == 0x0F) {
        continue;
      }

      // Format the entry name
      char entry_name[13]; // 8.3 name + null terminator
      int name_len = 0;

      // Copy name part (8 characters)
      for (int j = 0; j < 8 && dir_entry[i].name[j] != ' ' && dir_entry[i].name[j] != '\0'; j++) {
        entry_name[name_len++] = dir_entry[i].name[j];
      }

      // Add dot if there's an extension
      if (dir_entry[i].ext[0] != ' ' && dir_entry[i].ext[0] != '\0') {
        entry_name[name_len++] = '.';

        // Copy extension part (3 characters)
        for (int j = 0; j < 3 && dir_entry[i].ext[j] != ' ' && dir_entry[i].ext[j] != '\0'; j++) {
          entry_name[name_len++] = dir_entry[i].ext[j];
        }
      }

      entry_name[name_len] = '\0';

      // Compare with requested name (case insensitive comparison)
      if (strcasecmp(entry_name, name)) {
        *entry = dir_entry[i];
        *entry_cluster = current_cluster;
        *entry_offset = i * sizeof(DirectoryEntryFat32);
        return true;
      }
    }

    // Get next cluster in the chain
    uint32_t next_cluster = get_next_cluster(current_cluster);
    
    // Check for invalid cluster chain
    if (next_cluster == 0xFFFFFFFF) {
      return false; // Invalid cluster chain
    }
    
    current_cluster = next_cluster;
  }

  return false;
}

void FAT32::list_directory(uint32_t dir_cluster) {
  // Validate input
  if (dir_cluster == 0) {
    libc::printf("Error: Invalid directory cluster \n");
    return;
  }

  // Buffer to hold directory cluster data
  uint8_t buffer[512 * 32]; // Assuming max 32 sectors per cluster

  // Start with the given cluster
  uint32_t current_cluster = dir_cluster;
  bool directory_empty = true;

  // Process clusters in the directory chain
  while (current_cluster != 0) {
    // Read the current cluster
    if (!read_cluster(current_cluster, buffer)) {
      libc::printf("Error: Failed to read directory cluster \n");
      return;
    }

    // Process directory entries in this cluster
    DirectoryEntryFat32 *dir_entry = (DirectoryEntryFat32 *)buffer;
    int entries_per_cluster = (512 * bpb.sector_per_cluster) / sizeof(DirectoryEntryFat32);
    
    for (int i = 0; i < entries_per_cluster; i++) {
      // Check for end of directory
      if (dir_entry[i].name[0] == 0x00) {
        break;
      }

      // Skip deleted entries
      if (dir_entry[i].name[0] == 0xE5) {
        continue;
      }

      // Skip long filename entries
      if ((dir_entry[i].attributes & 0x0F) == 0x0F) {
        continue;
      }

      // Format and print the entry name
      char entry_name[13]; // 8.3 name + null terminator
      int name_len = 0;

      // Copy name part (8 characters)
      for (int j = 0; j < 8 && dir_entry[i].name[j] != ' ' && dir_entry[i].name[j] != '\0'; j++) {
        entry_name[name_len++] = dir_entry[i].name[j];
      }

      // Add dot if there's an extension
      if (dir_entry[i].ext[0] != ' ' && dir_entry[i].ext[0] != '\0') {
        entry_name[name_len++] = '.';

        // Copy extension part (3 characters)
        for (int j = 0; j < 3 && dir_entry[i].ext[j] != ' ' && dir_entry[i].ext[j] != '\0'; j++) {
          entry_name[name_len++] = dir_entry[i].ext[j];
        }
      }

      entry_name[name_len] = '\0';

      // Skip empty names
      if (name_len == 0) {
        continue;
      }

      // Mark that we found at least one entry
      directory_empty = false;

      // Print file name, size and attributes
      libc::printf(entry_name);

      // Print directory indicator
      if (dir_entry[i].attributes & 0x10) {
        libc::printf(" [DIR]");
      } else {
        libc::printf(" ");
        // Print size in a more readable format
        libc::print_int(dir_entry[i].size);
        libc::printf(" ");
      }

      libc::printf("\n");
    }

    // Get next cluster in the chain
    uint32_t next_cluster = get_next_cluster(current_cluster);
    
    // Check for invalid cluster chain
    if (next_cluster == 0xFFFFFFFF) {
      libc::printf("Error: Invalid cluster chain detected \n");
      break;
    }
    
    current_cluster = next_cluster;
  }
  
  // If directory is empty, print a message
  if (directory_empty) {
    libc::printf("Directory is empty \n");
  }
}

} // namespace filesystem
} // namespace uqaabOS