#include "../include/filesystem/fat.h"

namespace uqaabOS {
namespace filesystem {

// Function to read the BIOS Parameter Block (BPB) and process the FAT32
// filesystem.
void read_bios_parameter_block(driver::ATA *hd,
                                           uint32_t partition_offset) {
  /*
   * This function reads the BIOS Parameter Block (BPB) from the first sector of
   * a FAT32 partition. It calculates the locations of FAT, root directory, and
   * data regions. It then iterates over the root directory entries to locate
   * files and directories. Finally, it reads and prints the content of files
   * found in the root directory.
   */

  BiosParameterBlock32
      bpb; // Structure to hold the BIOS Parameter Block (BPB) data.

  hd->read28(
      partition_offset, (uint8_t *)&bpb,
      sizeof(BiosParameterBlock32)); // Read the BPB from the first sector of
                                     // the partition into the `bpb` structure.
                                     
  uint32_t fat_start =
      partition_offset + bpb.reserved_sectors; // Calculate the starting sector of the FAT table.
  uint32_t fat_size = bpb.table_size; // Get the size of a single FAT table in sectors.
  uint32_t data_start =
      fat_start + (fat_size * bpb.fat_copies); // Calculate the starting sector of the data region.
  uint32_t root_start =
      data_start + (bpb.sector_per_cluster * (bpb.root_cluster - 2)); // Calculate the starting sector of the root directory.

  DirectoryEntryFat32 dirent[16]; // Array to hold directory entries (16 entries at a time).

  hd->read28(
      root_start, (uint8_t *)&dirent[0],
      16 * sizeof(DirectoryEntryFat32)); // Read the first 16 directory entries
                                         // from the root directory.

  for (int i = 0; i < 16; i++) {
    if (dirent[i].name[0] == 0x00)
      break;

    if ((dirent[i].attributes & 0x0F) == 0x0F)
      continue;

    char *foo = "        \n"; // Temporary buffer to store the file name.
    for (int j = 0; j < 8; j++)
      foo[j] = dirent[i].name[j]; // Copy the file name from the directory entry
                                  // to the buffer.
    libc::printf(foo);

    // Skip directories, only process files.
    if ((dirent[i].attributes & 0x10) == 0x10) // directory
      continue;

    uint32_t file_cluster =
        ((uint32_t)dirent[i].first_cluster_hi) << 16 |
        ((uint32_t)dirent[i]
             .first_cluster_low); // Calculate the starting cluster of the file.
    uint32_t file_sector =
        data_start +
        bpb.sector_per_cluster *
            (file_cluster - 2); // Calculate the starting sector of the file.

    uint8_t buffer[512]; // Buffer to hold the file content (one sector).

    hd->read28(file_sector, buffer,
               512); // Read the first sector of the file into the buffer.

    buffer[dirent[i].size] = '\0';

    libc::printf((char *)buffer); // Print the file content.
  }
}

} // namespace filesystem
} // namespace uqaabOS