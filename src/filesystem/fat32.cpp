#include "../include/filesystem/fat32.h"
#include "../include/libc/string.h"

namespace uqaabOS {
namespace filesystem {

FAT32::FAT32(driver::ATA* disk, uint32_t partition_lba) {
    this->disk = disk;
    this->partition_lba = partition_lba;
    
    // Initialize filesystem layout information
    this->fat_start = 0;
    this->fat_size = 0;
    this->data_start = 0;
    this->root_cluster = 0;
    
    // Initialize file descriptors
    for (int i = 0; i < FAT32_MAX_OPEN_FILES; i++) {
        file_descriptors[i].first_cluster = 0;
        file_descriptors[i].current_cluster = 0;
        file_descriptors[i].current_sector_in_cluster = 0;
        file_descriptors[i].size = 0;
        file_descriptors[i].position = 0;
        file_descriptors[i].is_open = false;
    }
}

bool FAT32::initialize() {
    // Read the BIOS Parameter Block from the first sector of the partition
    disk->read28(partition_lba, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));
    
    // Validate BPB signature
    if (bpb.boot_signature != 0x29 && bpb.boot_signature != 0x28) {
        libc::printf("Invalid FAT32 boot signature: ");
        libc::print_hex(bpb.boot_signature);
        libc::printf("\n");
        return false;
    }
    
    // Check if this is actually FAT32 by looking at the FAT type label
    if (libc::strncmp((const char*)bpb.fatType_label, "FAT32   ", 8) != 0) {
        libc::printf("Not a FAT32 filesystem. Label: '");
        for (int i = 0; i < 8; i++) {
            libc::printf("%c", bpb.fatType_label[i]);
        }
        libc::printf("'\n");
        return false;
    }
    
    // Validate BPB parameters
    if (bpb.sector_per_cluster == 0) {
        libc::printf("Invalid sectors per cluster: 0\n");
        return false;
    }
    
    if (bpb.reserved_sectors == 0) {
        libc::printf("Invalid reserved sectors: 0\n");
        return false;
    }
    
    if (bpb.fat_copies == 0) {
        libc::printf("Invalid number of FAT copies: 0\n");
        return false;
    }
    
    if (bpb.table_size == 0) {
        libc::printf("Invalid FAT table size: 0\n");
        return false;
    }
    
    // Calculate filesystem layout
    fat_start = partition_lba + bpb.reserved_sectors;
    fat_size = bpb.table_size;
    data_start = fat_start + (fat_size * bpb.fat_copies);
    root_cluster = bpb.root_cluster;
    
    // Validate root cluster
    if (root_cluster < 2) {
        libc::printf("Invalid root cluster: ");
        libc::print_hex(root_cluster);
        libc::printf("\n");
        return false;
    }
    
    libc::printf("FAT32 filesystem initialized successfully\n");
    libc::printf("  FAT start: ");
    libc::print_hex(fat_start);
    libc::printf("\n  FAT size: ");
    libc::print_hex(fat_size);
    libc::printf("\n  Data start: ");
    libc::print_hex(data_start);
    libc::printf("\n  Root cluster: ");
    libc::print_hex(root_cluster);
    libc::printf("\n");
    
    return true;
}

uint32_t FAT32::cluster_to_lba(uint32_t cluster) {
    // Validate cluster number
    if (cluster < 2) {
        libc::printf("Error: Invalid cluster number in cluster_to_lba: ");
        libc::print_hex(cluster);
        libc::printf("\n");
        return 0; // Invalid cluster
    }
    
    // Convert cluster number to LBA
    // Cluster numbers start at 2, and cluster 2 is the first cluster of the data area
    return data_start + (cluster - 2) * bpb.sector_per_cluster;
}

bool FAT32::read_sector(uint32_t lba, uint8_t* buffer) {
    // Validate input
    if (buffer == nullptr) {
        libc::printf("Error: Null buffer provided to read_sector\n");
        return false;
    }
    
    // Validate LBA
    if (lba < partition_lba) {
        libc::printf("Error: Invalid LBA provided to read_sector: ");
        libc::print_hex(lba);
        libc::printf("\n");
        return false;
    }
    
    // Read a single sector from disk
    // Note: In a real implementation, we would check the return value of read28
    // For now, we'll assume it succeeds as in the original code
    disk->read28(lba, buffer, 512);
    return true;
}

uint32_t FAT32::get_next_cluster(uint32_t cluster) {
    // Validate cluster number
    if (cluster < 2) {
        libc::printf("Error: Invalid cluster number: ");
        libc::print_hex(cluster);
        libc::printf("\n");
        return 0xFFFFFFFF; // Invalid cluster
    }
    
    // Calculate which sector of the FAT contains this cluster's entry
    uint32_t fat_sector = cluster / (512 / sizeof(uint32_t));
    uint32_t fat_offset = cluster % (512 / sizeof(uint32_t));
    
    // Bounds check
    if (fat_sector >= fat_size) {
        libc::printf("Error: FAT sector out of bounds: ");
        libc::print_hex(fat_sector);
        libc::printf("\n");
        return 0xFFFFFFFF; // Invalid sector
    }
    
    // Read the FAT sector
    uint8_t fat_buffer[512];
    if (!read_sector(fat_start + fat_sector, fat_buffer)) {
        libc::printf("Error: Failed to read FAT sector\n");
        return 0xFFFFFFFF; // Error reading sector
    }
    
    // Extract the next cluster value
    uint32_t* fat_entry = (uint32_t*)fat_buffer;
    uint32_t next_cluster = fat_entry[fat_offset] & 0x0FFFFFFF; // Mask to 28 bits
    
    // Check for end of chain markers
    if (next_cluster >= 0x0FFFFFF8) {
        return 0; // End of chain
    }
    
    return next_cluster;
}

bool FAT32::read_cluster(uint32_t cluster, uint8_t* buffer) {
    // Validate input
    if (buffer == nullptr) {
        libc::printf("Error: Null buffer provided to read_cluster\n");
        return false;
    }
    
    // Validate cluster number
    if (cluster < 2) {
        libc::printf("Error: Invalid cluster number in read_cluster: ");
        libc::print_hex(cluster);
        libc::printf("\n");
        return false;
    }
    
    // Convert cluster to LBA
    uint32_t lba = cluster_to_lba(cluster);
    
    // Read all sectors in this cluster
    for (int i = 0; i < bpb.sector_per_cluster; i++) {
        if (!read_sector(lba + i, buffer + (i * 512))) {
            libc::printf("Error: Failed to read sector in cluster: ");
            libc::print_hex(lba + i);
            libc::printf("\n");
            return false;
        }
    }
    
    return true;
}

// Helper function for case-insensitive string comparison
bool FAT32::strcasecmp(const char* str1, const char* str2) {
    while (*str1 && *str2) {
        char c1 = *str1;
        char c2 = *str2;
        
        // Convert to lowercase
        if (c1 >= 'A' && c1 <= 'Z') c1 = c1 - 'A' + 'a';
        if (c2 >= 'A' && c2 <= 'Z') c2 = c2 - 'A' + 'a';
        
        if (c1 != c2) return false;
        
        str1++;
        str2++;
    }
    
    return *str1 == *str2; // Both should be null at the end
}

bool FAT32::find_file_in_root(const char* name, DirectoryEntryFat32* entry) {
    // Validate inputs
    if (name == nullptr || entry == nullptr) {
        return false;
    }
    
    // Buffer to hold directory cluster data
    uint8_t buffer[512 * 32]; // Assuming max 32 sectors per cluster
    
    // Start with the root cluster
    uint32_t current_cluster = root_cluster;
    
    // Process clusters in the root directory chain
    while (current_cluster != 0) {
        // Read the current cluster
        if (!read_cluster(current_cluster, buffer)) {
            libc::printf("Error: Failed to read root directory cluster\n");
            return false;
        }
        
        // Process directory entries in this cluster
        DirectoryEntryFat32* dir_entry = (DirectoryEntryFat32*)buffer;
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
                return true;
            }
        }
        
        // Get next cluster in the chain
        uint32_t next_cluster = get_next_cluster(current_cluster);
        
        // Check for invalid cluster chain
        if (next_cluster == 0xFFFFFFFF) {
            libc::printf("Error: Invalid cluster chain in root directory\n");
            return false;
        }
        
        current_cluster = next_cluster;
    }
    
    return false;
}

int FAT32::open(const char* path) {
    // Handle null path
    if (path == nullptr) {
        libc::printf("Error: Null path provided\n");
        return -1; // Invalid path
    }
    
    // Handle empty path
    if (path[0] == '\0') {
        libc::printf("Error: Empty path provided\n");
        return -1; // Invalid path
    }
    
    // Handle root directory path
    if ((path[0] == '/' && path[1] == '\0') || 
        (path[0] == '.' && path[1] == '\0')) {
        libc::printf("Error: Cannot open directory as a file\n");
        return -1; // Cannot open directory as a file
    }
    
    // Parse the path to separate parent directory and filename
    char parent_dir[256];
    char filename[256];
    if (!parse_path(path, parent_dir, filename)) {
        libc::printf("Error: Failed to parse path\n");
        return -1; // Failed to parse path
    }
    
    // Handle case where filename is empty (e.g., "/test/")
    if (filename[0] == '\0') {
        libc::printf("Error: Cannot open directory as a file\n");
        return -1; // Cannot open directory as a file
    }
    
    // Find the directory cluster where the file should be located
    uint32_t dir_cluster = find_directory_cluster(parent_dir);
    if (dir_cluster == 0) {
        libc::printf("Error: Directory not found: ");
        libc::printf(parent_dir);
        libc::printf("\n");
        return -1; // Directory not found
    }
    
    // Find the file in the specified directory
    DirectoryEntryFat32 entry;
    uint32_t entry_cluster, entry_offset;
    if (!find_file_in_directory(dir_cluster, filename, &entry, &entry_cluster, &entry_offset)) {
        libc::printf("Error: File not found: ");
        libc::printf(filename);
        libc::printf("\n");
        return -1; // File not found
    }
    
    // Check if the entry is a directory - we can't open directories as files
    if (entry.attributes & 0x10) {
        libc::printf("Error: Path is a directory, not a file: ");
        libc::printf(path);
        libc::printf("\n");
        return -1; // This is a directory, not a file
    }
    
    // Find a free file descriptor
    int fd = -1;
    for (int i = 0; i < FAT32_MAX_OPEN_FILES; i++) {
        if (!file_descriptors[i].is_open) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) {
        libc::printf("Error: Maximum number of open files reached\n");
        return -1; // No free file descriptors
    }
    
    // Initialize the file descriptor
    file_descriptors[fd].first_cluster = ((uint32_t)entry.first_cluster_hi << 16) | 
                                         ((uint32_t)entry.first_cluster_low);
    file_descriptors[fd].current_cluster = file_descriptors[fd].first_cluster;
    file_descriptors[fd].current_sector_in_cluster = 0;
    file_descriptors[fd].size = entry.size;
    file_descriptors[fd].position = 0;
    file_descriptors[fd].is_open = true;
    
    return fd;
}

int FAT32::read(int fd, uint8_t* buf, uint32_t size) {
    // Validate inputs
    if (buf == nullptr) {
        libc::printf("Error: Null buffer provided\n");
        return -1;
    }
    
    // Validate file descriptor
    if (fd < 0 || fd >= FAT32_MAX_OPEN_FILES || !file_descriptors[fd].is_open) {
        libc::printf("Error: Invalid file descriptor\n");
        return -1;
    }
    
    FileDescriptor* file = &file_descriptors[fd];
    
    // Don't read past end of file
    if (file->position >= file->size) {
        return 0; // EOF
    }
    
    // Limit size to remaining bytes in file
    uint32_t remaining = file->size - file->position;
    if (size > remaining) {
        size = remaining;
    }
    
    // Handle zero size read
    if (size == 0) {
        return 0;
    }
    
    // Read data
    uint32_t bytes_read = 0;
    uint8_t sector_buffer[512];
    
    while (bytes_read < size && file->current_cluster != 0) {
        // Calculate current LBA
        uint32_t lba = cluster_to_lba(file->current_cluster) + file->current_sector_in_cluster;
        
        // Read the sector
        if (!read_sector(lba, sector_buffer)) {
            libc::printf("Error: Failed to read sector at LBA ");
            libc::print_hex(lba);
            libc::printf("\n");
            return -1; // Error reading sector
        }
        
        // Calculate how many bytes we can read from this sector
        uint32_t sector_offset = file->position % 512;
        uint32_t bytes_from_sector = 512 - sector_offset;
        uint32_t bytes_needed = size - bytes_read;
        
        if (bytes_from_sector > bytes_needed) {
            bytes_from_sector = bytes_needed;
        }
        
        // Copy data to output buffer
        libc::memcpy(buf + bytes_read, sector_buffer + sector_offset, bytes_from_sector);
        
        // Update position
        bytes_read += bytes_from_sector;
        file->position += bytes_from_sector;
        
        // Update sector tracking
        file->current_sector_in_cluster = (file->position / 512) % bpb.sector_per_cluster;
        
        // If we've reached the end of the current cluster, move to next cluster
        if (file->current_sector_in_cluster == 0 && file->position > 0 && bytes_read < size) {
            file->current_cluster = get_next_cluster(file->current_cluster);
            file->current_sector_in_cluster = 0;
            
            // Check for invalid cluster chain
            if (file->current_cluster == 0xFFFFFFFF) {
                libc::printf("Error: Invalid cluster chain detected\n");
                return -1;
            }
        }
    }
    
    return bytes_read;
}

void FAT32::close(int fd) {
    // Validate file descriptor
    if (fd < 0 || fd >= FAT32_MAX_OPEN_FILES) {
        libc::printf("Error: Invalid file descriptor\n");
        return;
    }
    
    if (!file_descriptors[fd].is_open) {
        libc::printf("Warning: File descriptor already closed\n");
        return;
    }
    
    // Reset file descriptor fields for safety
    file_descriptors[fd].first_cluster = 0;
    file_descriptors[fd].current_cluster = 0;
    file_descriptors[fd].current_sector_in_cluster = 0;
    file_descriptors[fd].size = 0;
    file_descriptors[fd].position = 0;
    
    // Mark as closed
    file_descriptors[fd].is_open = false;
}

void FAT32::list_root() {
    // Buffer to hold directory cluster data
    uint8_t buffer[512 * 32]; // Assuming max 32 sectors per cluster
    
    // Start with the root cluster
    uint32_t current_cluster = root_cluster;
    bool directory_empty = true;
    
    // Process clusters in the root directory chain
    while (current_cluster != 0) {
        // Read the current cluster
        if (!read_cluster(current_cluster, buffer)) {
            libc::printf("Error: Failed to read root directory cluster\n");
            return;
        }
        
        // Process directory entries in this cluster
        DirectoryEntryFat32* dir_entry = (DirectoryEntryFat32*)buffer;
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
            libc::printf("Error: Invalid cluster chain detected in root directory\n");
            break;
        }
        
        current_cluster = next_cluster;
    }
    
    // If directory is empty, print a message
    if (directory_empty) {
        libc::printf("Root directory is empty\n");
    }
}

} // namespace filesystem
} // namespace uqaabOS