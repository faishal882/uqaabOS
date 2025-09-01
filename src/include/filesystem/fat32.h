#ifndef __FILESYSTEM__FAT32_H
#define __FILESYSTEM__FAT32_H

#include "../drivers/storage/ata.h"
#include "../libc/stdio.h"
#include "fat.h"

namespace uqaabOS {
namespace filesystem {

// Maximum number of open files
#define FAT32_MAX_OPEN_FILES 16

// File descriptor structure
struct FileDescriptor {
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t current_sector_in_cluster;
    uint32_t size;
    uint32_t position;
    bool is_open;
};

class FAT32 {
private:
    driver::ATA* disk;
    uint32_t partition_lba;
    
    // BPB information
    BiosParameterBlock32 bpb;
    
    // Filesystem layout information
    uint32_t fat_start;
    uint32_t fat_size;
    uint32_t data_start;
    uint32_t root_cluster;
    
    // Open file descriptors
    FileDescriptor file_descriptors[FAT32_MAX_OPEN_FILES];
    
    // Private helper methods
    uint32_t get_next_cluster(uint32_t cluster);
    bool read_cluster(uint32_t cluster, uint8_t* buffer);
    bool find_file_in_root(const char* name, DirectoryEntryFat32* entry);
    uint32_t cluster_to_lba(uint32_t cluster);
    bool read_sector(uint32_t lba, uint8_t* buffer);
    bool strcasecmp(const char* str1, const char* str2); // Case-insensitive string comparison
    
    // New helper methods for write operations
    bool write_sector(uint32_t lba, uint8_t* buffer);
    bool write_cluster(uint32_t cluster, uint8_t* buffer);
    bool set_next_cluster(uint32_t cluster, uint32_t next_cluster);
    uint32_t find_free_cluster();
    bool allocate_cluster(uint32_t* cluster);
    bool free_cluster_chain(uint32_t start_cluster);
    
    // Helper for path traversal
    bool find_file_in_directory(uint32_t dir_cluster, const char* name, DirectoryEntryFat32* entry, uint32_t* entry_cluster, uint32_t* entry_offset);
    bool parse_path(const char* path, char* parent_dir, char* filename);
    uint32_t find_directory_cluster(const char* path);
    void list_directory(uint32_t dir_cluster);
    
public:
    // Constructor
    FAT32(driver::ATA* disk, uint32_t partition_lba);
    
    // Initialize the FAT32 filesystem
    bool initialize();
    
    // Open a file and return a file descriptor
    int open(const char* path);
    
    // Read data from a file
    int read(int fd, uint8_t* buf, uint32_t size);
    
    // Close a file
    void close(int fd);
    
    // List files in the root directory
    void list_root();
    
    // Extended read-only functions
    void ls(const char* path); // List files in a directory
    
    // Write functions
    bool mkdir(const char* path); // Create a new directory
    bool touch(const char* path); // Create a new empty file
    bool rm(const char* path); // Remove a file
    bool rmdir(const char* path); // Remove a directory (recursive)
    int write(int fd, uint8_t* buf, uint32_t size); // Write data to a file
};

} // namespace filesystem
} // namespace uqaabOS

#endif // __FILESYSTEM__FAT32_H