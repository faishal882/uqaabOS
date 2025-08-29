#ifndef __DRIVERS__ATA_H
#define __DRIVERS__ATA_H

#include "../../interrupts.h"
#include "../../libc/stdio.h"
#include "../../port.h"
#include <stdint.h>

namespace uqaabOS {
namespace driver {

/*
 * Description: This header file defines the ATA class for interfacing with ATA
 * storage devices. It provides methods to identify the ATA device, read and
 * write sectors using 28-bit LBA addressing, and flush the write cache. The
 * class encapsulates low-level I/O port operations required to communicate with
 * the hardware.
 */

// ATA class for interfacing with ATA storage devices.
class ATA : public uqaabOS::interrupts::InterruptHandler {
protected:
  bool master; // Boolean flag indicating if the device is the master drive.
  uint16_t bytes_per_sector; // Number of bytes per sector (default 512).
  include::Port16Bit data_port; // 16-bit data port for data transfers(0x1F0/0x170).
  include::Port8Bit error_port; // 8-bit error port for error reporting(0x1F1/0x171).
  include::Port8Bit
      sector_count_port; // 8-bit port to set the number of sectors(0x1F2/0x172).
  include::Port8Bit
      lba_low_port; // 8-bit port for the low byte of the LBA address(0x1F3/0x173).
  include::Port8Bit
      lba_mid_port; // 8-bit port for the mid byte of the LBA address(0x1F4/0x174).
  include::Port8Bit
      lba_high_port; // 8-bit port for the high byte of the LBA address(0x1F5/0x175).
  include::Port8Bit device_port;  // 8-bit device register port(0x1F6/0x176).
  include::Port8Bit command_port; // 8-bit command register port(0x1F7/0x177).
  include::Port8Bit control_port; // 8-bit control register port(0x3F6/0x376).
public:
  // Constructor: Initializes ATA object with master/slave flag and base I/O
  // port.
  ATA(uqaabOS::interrupts::InterruptManager* interrupt_manager, bool master, uint16_t port_base);

  ~ATA();

  // Handle interrupt for ATA device
  virtual uint32_t handle_interrupt(uint32_t esp);

  // Sends the IDENTIFY command to the ATA device and prints its information.
  void identify();

  // Reads data from one sector of the device using 28-bit LBA addressing.
  void read28(uint32_t sector_num, uint8_t* data, uint32_t count);

  // Writes data to one sector of the device using 28-bit LBA addressing.
  void write28(uint32_t sector_num, uint8_t *data, uint32_t count);
  
  // Flushes the ATA device's write cache.
  void flush();
};

} // namespace driver
} // namespace uqaabOS

#endif // __DRIVERS__ATA_H
