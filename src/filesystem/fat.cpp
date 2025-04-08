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
   /*
    * Read Bios Parameter block i.e. first sector of partition also known as VolumeId
    * Use bpb to find fatStart, fatSize, dataStart, rootStart
    * read first rootCluster to find location of files/directory on filesystem
    * iterate over directory entries and find location of files using firstClusterLow, firstClusterHi
    * then Read all sector/cluster belonging to file:-
      i)  Get next cluster belonging to file from FAT table
      ii) Get next sector belonging to file from current file cluster
    */
  BiosParameterBlock32
      bpb; // Structure to hold the BIOS Parameter Block (BPB) data.

  hd->read28(
      partition_offset, (uint8_t *)&bpb,
      sizeof(BiosParameterBlock32)); // Read the BPB from the first sector of
                                     // the partition into the `bpb` structure.
  
  libc::printf("sector per cluster: ");
  libc::print_hex(bpb.sector_per_cluster);
  libc::printf("\n");

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
    if (dirent[i].name[0] == 0x00)  // unused directory entry
      break;

    if ((dirent[i].attributes & 0x0F) == 0x0F) // long file name directory entry
      continue;

    char *foo = "        \n"; // Temporary buffer to store the file name.
    for (int j = 0; j < 8; j++)
      foo[j] = dirent[i].name[j]; // Copy the file name from the directory entry
                                  // to the buffer.
    libc::printf(foo);

    // Skip directories, only process files.
    if ((dirent[i].attributes & 0x10) == 0x10) // directory
      continue;

    /* Read all sector/cluster belonging to file */
    uint32_t first_file_cluster = ((uint32_t)dirent[i].first_cluster_hi) << 16
                                | ((uint32_t)dirent[i].first_cluster_low);

    int32_t SIZE = dirent[i].size;
    int32_t next_file_cluster = first_file_cluster;
    uint8_t buffer[513];
    uint8_t fat_buffer[513];

    while(SIZE > 0)
    {
      uint32_t file_sector = data_start + bpb.sector_per_cluster * (next_file_cluster - 2) ;
      int sector_offset = 0;

      for(; SIZE > 0; SIZE -= 512)
      {
        /* Get next sector belonging to file from current file cluster
        1) read sector by sector data of file until SIZE < 0 or
         all sectors in currentCluster are read
        */
        hd->read28(file_sector + sector_offset, buffer, 512);
        buffer[SIZE > 512 ? 512 : SIZE] = '\0';
        libc::printf((char*)buffer);

        if(++sector_offset > bpb.sector_per_cluster)
          break;
      }

      /* Get next cluster belonging to file from FAT table
      1) get sector in FAT table holding current file cluster
      2) fig out current file cluster entry num in FAT sector out of 128 directory entries
      3) update next cluster with most significant 28 bits
      */
      uint32_t fat_sector_for_current_cluster = next_file_cluster / (512/sizeof(uint32_t));
      hd->read28(fat_start + fat_sector_for_current_cluster, fat_buffer, 512);

      uint32_t fat_offset_in_fat_sector_for_current_cluster = next_file_cluster % (512/sizeof(uint32_t));
      next_file_cluster = ((uint32_t*)&fat_buffer)[fat_offset_in_fat_sector_for_current_cluster] & 0x0FFFFFFF;
    }
  }
}

} // namespace filesystem
} // namespace uqaabOS