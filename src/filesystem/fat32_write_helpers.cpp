#include "../include/filesystem/fat32.h"

namespace uqaabOS {
namespace filesystem {

bool FAT32::write_sector(uint32_t lba, uint8_t *buffer) {
  // Write a single sector to disk
  disk->write28(lba, buffer, 512);
  return true;
}

bool FAT32::write_cluster(uint32_t cluster, uint8_t *buffer) {
  // Convert cluster to LBA
  uint32_t lba = cluster_to_lba(cluster);

  // Write all sectors in this cluster
  for (int i = 0; i < bpb.sector_per_cluster; i++) {
    write_sector(lba + i, buffer + (i * 512));
  }

  return true;
}

bool FAT32::set_next_cluster(uint32_t cluster, uint32_t next_cluster) {
  // Calculate which sector of the FAT contains this cluster's entry
  uint32_t fat_sector = cluster / (512 / sizeof(uint32_t));
  uint32_t fat_offset = cluster % (512 / sizeof(uint32_t));

  // Read the FAT sector
  uint8_t fat_buffer[512];
  read_sector(fat_start + fat_sector, fat_buffer);

  // Update the next cluster value
  uint32_t *fat_entry = (uint32_t *)fat_buffer;
  fat_entry[fat_offset] =
      (fat_entry[fat_offset] & 0xF0000000) | (next_cluster & 0x0FFFFFFF);

  // Write the FAT sector back
  write_sector(fat_start + fat_sector, fat_buffer);

  return true;
}

uint32_t FAT32::find_free_cluster() {
  // Search the FAT for a free cluster (marked with 0x00000000)
  uint8_t fat_buffer[512];

  // Calculate total number of clusters
  uint32_t total_clusters = fat_size * 512 / sizeof(uint32_t);

  for (uint32_t sector = 0; sector < fat_size; sector++) {
    // Read the FAT sector
    read_sector(fat_start + sector, fat_buffer);

    // Check each entry in this sector
    uint32_t *fat_entry = (uint32_t *)fat_buffer;
    for (int i = 0; i < 512 / sizeof(uint32_t); i++) {
      uint32_t cluster_num = sector * (512 / sizeof(uint32_t)) + i;

      // Skip reserved clusters (0 and 1)
      if (cluster_num < 2)
        continue;

      // Check if this cluster is free
      if (fat_entry[i] == 0x00000000) {
        return cluster_num;
      }
    }
  }

  // No free clusters found
  return 0;
}

bool FAT32::allocate_cluster(uint32_t *cluster) {
  // Find a free cluster
  *cluster = find_free_cluster();
  if (*cluster == 0) {
    return false; // No free clusters
  }

  // Mark the cluster as end of chain (0x0FFFFFFF)
  set_next_cluster(*cluster, 0x0FFFFFFF);

  return true;
}

bool FAT32::free_cluster_chain(uint32_t start_cluster) {
  uint32_t current_cluster = start_cluster;

  while (current_cluster != 0 && current_cluster != 0x0FFFFFFF) {
    // Get the next cluster before we overwrite the current one
    uint32_t next_cluster = get_next_cluster(current_cluster);

    // Mark current cluster as free
    set_next_cluster(current_cluster, 0x00000000);

    // Move to next cluster
    current_cluster = next_cluster;
  }

  return true;
}

} // namespace filesystem
} // namespace uqaabOS