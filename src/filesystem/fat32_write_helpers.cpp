#include "../include/filesystem/fat32.h"

namespace uqaabOS {
namespace filesystem {

bool FAT32::write_sector(uint32_t lba, uint8_t *buffer) {
  // Validate input
  if (buffer == nullptr) {
    libc::printf("Error: Null buffer provided to write_sector\n");
    return false;
  }
  
  // Validate LBA
  if (lba < partition_lba) {
    libc::printf("Error: Invalid LBA provided to write_sector: ");
    libc::print_hex(lba);
    libc::printf("\n");
    return false;
  }
  
  // Write a single sector to disk
  // Note: Since write28 is void, we can't check its return value
  // but we can add validation before calling it
  disk->write28(lba, buffer, 512);
  
  // Since we can't check the return value of write28, we'll assume it succeeds
  // In a real implementation, we would implement a way to verify the write
  return true;
}

bool FAT32::write_cluster(uint32_t cluster, uint8_t *buffer) {
  // Validate input
  if (buffer == nullptr) {
    libc::printf("Error: Null buffer provided to write_cluster\n");
    return false;
  }
  
  // Validate cluster number
  if (cluster < 2) {
    libc::printf("Error: Invalid cluster number in write_cluster: ");
    libc::print_hex(cluster);
    libc::printf("\n");
    return false;
  }
  
  // Convert cluster to LBA
  uint32_t lba = cluster_to_lba(cluster);
  
  // Validate LBA
  if (lba == 0) {
    libc::printf("Error: Invalid LBA calculated in write_cluster\n");
    return false;
  }

  // Write all sectors in this cluster
  for (int i = 0; i < bpb.sector_per_cluster; i++) {
    if (!write_sector(lba + i, buffer + (i * 512))) {
      libc::printf("Error: Failed to write sector in cluster: ");
      libc::print_hex(lba + i);
      libc::printf("\n");
      return false;
    }
  }

  return true;
}

bool FAT32::set_next_cluster(uint32_t cluster, uint32_t next_cluster) {
  // Validate cluster number
  if (cluster < 2) {
    libc::printf("Error: Invalid cluster number in set_next_cluster: ");
    libc::print_hex(cluster);
    libc::printf("\n");
    return false;
  }
  
  // Calculate which sector of the FAT contains this cluster's entry
  uint32_t fat_sector = cluster / (512 / sizeof(uint32_t));
  uint32_t fat_offset = cluster % (512 / sizeof(uint32_t));
  
  // Bounds check
  if (fat_sector >= fat_size) {
    libc::printf("Error: FAT sector out of bounds in set_next_cluster: ");
    libc::print_hex(fat_sector);
    libc::printf("\n");
    return false;
  }

  // Read the FAT sector
  uint8_t fat_buffer[512];
  if (!read_sector(fat_start + fat_sector, fat_buffer)) {
    libc::printf("Error: Failed to read FAT sector in set_next_cluster\n");
    return false;
  }

  // Update the next cluster value
  uint32_t *fat_entry = (uint32_t *)fat_buffer;
  fat_entry[fat_offset] =
      (fat_entry[fat_offset] & 0xF0000000) | (next_cluster & 0x0FFFFFFF);

  // Write the FAT sector back
  if (!write_sector(fat_start + fat_sector, fat_buffer)) {
    libc::printf("Error: Failed to write FAT sector in set_next_cluster\n");
    return false;
  }

  return true;
}

uint32_t FAT32::find_free_cluster() {
  // Search the FAT for a free cluster (marked with 0x00000000)
  uint8_t fat_buffer[512];

  // Calculate total number of clusters
  uint32_t total_clusters = fat_size * 512 / sizeof(uint32_t);

  for (uint32_t sector = 0; sector < fat_size; sector++) {
    // Read the FAT sector
    if (!read_sector(fat_start + sector, fat_buffer)) {
      libc::printf("Error: Failed to read FAT sector in find_free_cluster\n");
      return 0;
    }

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
  // Validate input
  if (cluster == nullptr) {
    libc::printf("Error: Null pointer provided to allocate_cluster\n");
    return false;
  }
  
  // Find a free cluster
  *cluster = find_free_cluster();
  if (*cluster == 0) {
    libc::printf("Error: No free clusters available\n");
    return false; // No free clusters
  }

  // Mark the cluster as end of chain (0x0FFFFFFF)
  if (!set_next_cluster(*cluster, 0x0FFFFFFF)) {
    libc::printf("Error: Failed to mark cluster as end of chain\n");
    return false;
  }

  return true;
}

bool FAT32::free_cluster_chain(uint32_t start_cluster) {
  uint32_t current_cluster = start_cluster;

  while (current_cluster != 0 && current_cluster != 0x0FFFFFFF) {
    // Validate cluster number
    if (current_cluster < 2) {
      libc::printf("Error: Invalid cluster number in free_cluster_chain: ");
      libc::print_hex(current_cluster);
      libc::printf("\n");
      return false;
    }
    
    // Get the next cluster before we overwrite the current one
    uint32_t next_cluster = get_next_cluster(current_cluster);
    
    // Check for invalid cluster chain
    if (next_cluster == 0xFFFFFFFF) {
      libc::printf("Error: Invalid cluster chain detected in free_cluster_chain\n");
      return false;
    }

    // Mark current cluster as free
    if (!set_next_cluster(current_cluster, 0x00000000)) {
      libc::printf("Error: Failed to mark cluster as free\n");
      return false;
    }

    // Move to next cluster
    current_cluster = next_cluster;
  }

  return true;
}

} // namespace filesystem
} // namespace uqaabOS