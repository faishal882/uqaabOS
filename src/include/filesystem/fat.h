#ifndef __FILESYSTEM__FAT_h
#define __FILESYSTEM__FAT_h

#include "../drivers/storage/ata.h"
#include "../libc/stdio.h"

namespace uqaabOS {
namespace filesystem {
/**
 * BIOS Parameter Block (BPB) structure for FAT32 filesystems.
 * 
 * This structure contains both the common BIOS parameter fields for FAT12/16/32
 * and the extended fields specific to FAT32. It is used to describe the layout
 * and configuration of a FAT32 filesystem.
 */
struct BiosParameterBlock32 {
  uint8_t jump[3];                  // Jump instruction to the boot code
  uint8_t soft_name[8];             // OEM name or system identifier
  uint16_t bytes_per_sector;        // Number of bytes per sector
  uint8_t sector_per_cluster;       // Number of sectors per cluster
  uint16_t reserved_sectors;        // Number of reserved sectors
  uint8_t fat_copies;               // Number of File Allocation Tables (FATs)
  uint16_t root_dir_entries;        // Number of entries in the root directory (FAT12/16 only)
  uint16_t total_sectors;           // Total number of sectors in the filesystem
  uint8_t media_type;               // Media descriptor type
  uint16_t fat_sector_count;        // Number of sectors per FAT (FAT12/16 only)
  uint16_t sector_per_track;        // Number of sectors per track
  uint16_t head_count;              // Number of heads
  uint32_t hidden_sectors;          // Number of hidden sectors
  uint32_t total_sector_count;      // Total number of sectors (32-bit value)

  // extended biosParameter for fat32 only
  uint32_t table_size;              // Size of each FAT in sectors
  uint16_t ext_flags;               // Extended flags for FAT32
  uint16_t fat_version;             // Version of the FAT32 filesystem
  uint32_t root_cluster;            // Cluster number of the root directory
  uint16_t fat_info;                // Sector number of the FSInfo structure
  uint16_t backup_sector;           // Sector number of the backup boot sector
  uint8_t reserved0[12];            // Reserved space for future use
  uint8_t drive_number;             // Physical drive number
  uint8_t reserved;                 // Reserved byte
  uint8_t boot_signature;           // Extended boot signature (0x29 if present)
  uint32_t volume_id;               // Volume serial number
  uint8_t volume_label[11];         // Volume label
  uint8_t fatType_label[8];         // FAT type label (e.g., "FAT32")

} __attribute__((packed));

/**
 * DirectoryEntryFat32
 * Represents a directory entry in a FAT32 filesystem.
 *
 * This structure is used to describe a single file or directory entry in a
 * FAT32 filesystem. It contains metadata about the file or directory, such as
 * its name, attributes, timestamps, and size.
 */
struct DirectoryEntryFat32 {
  uint8_t name[8];              // name of file or directory  (8 characters)
  uint8_t ext[3];               // the file extension (3 characters)
  uint8_t attributes;           // file attributes (e.g., read-only, hidden, system)
  uint8_t reserved;             // reserved byte for future use
  uint8_t c_time_tenth;         // creation time in tenths of a second
  uint16_t c_time;              // creation time
  uint16_t c_date;              // creation date
  uint16_t a_time;              // last access date
  uint16_t first_cluster_hi;    // high 16 bits of the first cluster number
  uint16_t w_time;              // last write time
  uint16_t w_date;              // last write date
  uint16_t first_cluster_low;   // low 16 bits of the first cluster number
  uint32_t size;                // size of the file in bytes

} __attribute__((packed));

// function
void read_bios_parameter_block(driver::ATA *hd, uint32_t parition_offset);
} 
} 

#endif