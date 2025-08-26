#include "../include/filesystem/fat32.h"
#include "../include/libc/string.h"

namespace uqaabOS {
namespace filesystem {

FAT32::FAT32(driver::ATA* disk, uint32_t partition_lba) {
    this->disk = disk;
    this->partition_lba = partition_lba;
    
    // Initialize file descriptors
    for (int i = 0; i < FAT32_MAX_OPEN_FILES; i++) {
        file_descriptors[i].is_open = false;
    }
}

bool FAT32::initialize() {
    // Read the BIOS Parameter Block from the first sector of the partition
    disk->read28(partition_lba, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));
    
    // Validate BPB signature
    if (bpb.boot_signature != 0x29) {
        libc::printf("Invalid FAT32 boot signature\n");
        return false;
    }
    
    // Check if this is actually FAT32 by looking at the FAT type label
    if (libc::strncmp((const char*)bpb.fatType_label, "FAT32   ", 8) != 0) {
        libc::printf("Not a FAT32 filesystem\n");
        return false;
    }
    
    // Calculate filesystem layout
    fat_start = partition_lba + bpb.reserved_sectors;
    fat_size = bpb.table_size;
    data_start = fat_start + (fat_size * bpb.fat_copies);
    root_cluster = bpb.root_cluster;
    
    return true;
}

uint32_t FAT32::cluster_to_lba(uint32_t cluster) {
    // Convert cluster number to LBA
    // Cluster numbers start at 2, and cluster 2 is the first cluster of the data area
    return data_start + (cluster - 2) * bpb.sector_per_cluster;
}

bool FAT32::read_sector(uint32_t lba, uint8_t* buffer) {
    // Read a single sector from disk
    disk->read28(lba, buffer, 512);
    return true;
}

uint32_t FAT32::get_next_cluster(uint32_t cluster) {
    // Calculate which sector of the FAT contains this cluster's entry
    uint32_t fat_sector = cluster / (512 / sizeof(uint32_t));
    uint32_t fat_offset = cluster % (512 / sizeof(uint32_t));
    
    // Read the FAT sector
    uint8_t fat_buffer[512];
    read_sector(fat_start + fat_sector, fat_buffer);
    
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
    // Convert cluster to LBA
    uint32_t lba = cluster_to_lba(cluster);
    
    // Read all sectors in this cluster
    for (int i = 0; i < bpb.sector_per_cluster; i++) {
        read_sector(lba + i, buffer + (i * 512));
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
    // Buffer to hold directory cluster data
    uint8_t buffer[512 * 32]; // Assuming max 32 sectors per cluster
    
    // Start with the root cluster
    uint32_t current_cluster = root_cluster;
    
    // Process clusters in the root directory chain
    while (current_cluster != 0) {
        // Read the current cluster
        read_cluster(current_cluster, buffer);
        
        // Process directory entries in this cluster
        DirectoryEntryFat32* dir_entry = (DirectoryEntryFat32*)buffer;
        for (int i = 0; i < (512 * bpb.sector_per_cluster) / sizeof(DirectoryEntryFat32); i++) {
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
            
            // Compare with requested name (case insensitive comparison)
            if (strcasecmp(entry_name, name)) {
                *entry = dir_entry[i];
                return true;
            }
        }
        
        // Get next cluster in the chain
        current_cluster = get_next_cluster(current_cluster);
    }
    
    return false;
}

int FAT32::open(const char* path) {
    // For simplicity, we only support root directory files
    // and the path should be just the filename
    
    DirectoryEntryFat32 entry;
    if (!find_file_in_root(path, &entry)) {
        return -1; // File not found
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
    // Validate file descriptor
    if (fd < 0 || fd >= FAT32_MAX_OPEN_FILES || !file_descriptors[fd].is_open) {
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
    
    // Read data
    uint32_t bytes_read = 0;
    uint8_t sector_buffer[512];
    
    while (bytes_read < size && file->current_cluster != 0) {
        // Calculate current LBA
        uint32_t lba = cluster_to_lba(file->current_cluster) + file->current_sector_in_cluster;
        
        // Read the sector
        read_sector(lba, sector_buffer);
        
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
        if (file->current_sector_in_cluster == 0 && file->position > 0) {
            file->current_cluster = get_next_cluster(file->current_cluster);
            file->current_sector_in_cluster = 0;
        }
    }
    
    return bytes_read;
}

void FAT32::close(int fd) {
    // Validate file descriptor
    if (fd < 0 || fd >= FAT32_MAX_OPEN_FILES || !file_descriptors[fd].is_open) {
        return;
    }
    
    // Mark as closed
    file_descriptors[fd].is_open = false;
}

void FAT32::list_root() {
    // Buffer to hold directory cluster data
    uint8_t buffer[512 * 32]; // Assuming max 32 sectors per cluster
    
    // Start with the root cluster
    uint32_t current_cluster = root_cluster;
    
    // Process clusters in the root directory chain
    while (current_cluster != 0) {
        // Read the current cluster
        read_cluster(current_cluster, buffer);
        
        // Process directory entries in this cluster
        DirectoryEntryFat32* dir_entry = (DirectoryEntryFat32*)buffer;
        for (int i = 0; i < (512 * bpb.sector_per_cluster) / sizeof(DirectoryEntryFat32); i++) {
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
            
            // Print file name and size
            libc::printf(entry_name);
            libc::printf(" ");
            libc::print_hex(dir_entry[i].size);
            libc::printf("\n");
        }
        
        // Get next cluster in the chain
        current_cluster = get_next_cluster(current_cluster);
    }
}

} // namespace filesystem
} // namespace uqaabOS