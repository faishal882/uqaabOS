#include "../include/filesystem/msdospart.h"

/*
This code implements, The function which reads
the Master Boot Record (MBR) from an ATA hard drive and prints its contents in
both raw and formatted forms. It also checks the validity of the MBR by
verifying its magic number and prints information about the primary partitions.

The code includes:
1. Reading the MBR from the hard drive.
2. Printing the raw MBR data.
3. Checking the MBR's magic number for validity.
4. Printing formatted information about each primary partition.
*/

namespace uqaabOS {
namespace filesystem {

void MSDOSPartitionTable::read_partitions(driver::ATA *hd) {
  MasterBootRecord mbr;

  libc::printf("MBR: ");

  // Read the MBR from the hard drive into the mbr variable
  hd->read28(0, (uint8_t *)&mbr, sizeof(MasterBootRecord));

  // // RAW printing of MBR
  // for (int i = 0x1BE; i <= 0x01FF; i++) {
  //   // Print each byte of the MBR in hexadecimal format
  //   libc::print_hex(((uint8_t *)&mbr)[i]);
  //   libc::printf(" ");
  // }
  // libc::printf("\n");

  // Check if the MBR's magic number is valid
  if (mbr.magicnumber != 0xAA55) {
    // Print an error message if the MBR is invalid
    libc::printf("illegal MBR");
    return;
  }

  for (int i = 0; i < 4; i++) {
    // Print partition number
    libc::printf(" Partition ");
    libc::print_hex(i & 0xFF);

    // Check if the partition is bootable
    if (mbr.primary_partitions[i].bootable == 0x80) {
      libc::printf(" bootable. Type ");
    } else {
      libc::printf(" not bootable. Type ");
    }

    // Print the partition type
    libc::print_hex(mbr.primary_partitions[i].partition_id);
  }
}

} // namespace filesystem
} // namespace uqaabOS
