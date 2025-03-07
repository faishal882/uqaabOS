#ifndef __FILESYSTEM__MSDOSPART_H 
#define __FILESYSTEM__MSDOSPART_H

#include "../drivers/storage/ata.h" 
#include "../libc/stdio.h"

namespace uqaabOS 
{
    namespace filesystem 
    {
        // Structure representing a partition table entry
        struct PartitionTableEntry
        {
          uint8_t bootable; // Bootable flag (0x80 if bootable, 0x00 if not)
          uint8_t start_head; // Starting head of the partition
          uint8_t start_sector: 6; // Starting sector of the partition (6 bits)
          uint16_t start_cylinder: 10; // Starting cylinder of the partition (10 bits)

          uint8_t partition_id; // Partition type identifier

          uint8_t end_head; // Ending head of the partition
          uint8_t end_sector: 6; // Ending sector of the partition (6 bits)
          uint16_t end_cylinder: 10; // Ending cylinder of the partition (10 bits)

          uint32_t start_lba; // Starting LBA (Logical Block Address) of the partition
          uint32_t length; // Length of the partition in sectors
        } __attribute__((packed)); 

        // Structure representing the Master Boot Record (MBR)
        struct MasterBootRecord
        {
          uint8_t bootloader[440];  // Bootloader code (440 bytes)
          uint32_t signature;       // Disk signature
          uint16_t unused;          // Unused bytes

          PartitionTableEntry primary_partitions[4]; // Array of primary partition table entries

          uint16_t magicnumber; // Magic number (0xAA55) indicating a valid MBR
        } __attribute__((packed)); 

        // Class representing the MSDOS partition table
        class MSDOSPartitionTable
        {
            public:
                static void read_partitions(driver::ATA *hd); // Static method to read partitions from an ATA hard drive
        };
    }
}

#endif 