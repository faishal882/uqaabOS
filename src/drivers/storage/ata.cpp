#include "../../include/drivers/storage/ata.h"

namespace uqaabOS {
namespace driver {

// ATA class constructor: Initializes the I/O ports based on the base port and
// whether the device is master.
ATA::ATA(bool master, uint16_t port_base)
    : data_port(port_base),        // Initialize data port at base address.
      error_port(port_base + 0x1), // Initialize error port at base + 1.
      sector_count_port(port_base +
                        0x2), // Initialize sector count port at base + 2.
      lba_low_port(port_base + 0x3),  // Initialize LBA low port at base + 3.
      lba_mid_port(port_base + 0x4),  // Initialize LBA mid port at base + 4.
      lba_high_port(port_base + 0x5), // Initialize LBA high port at base + 5.
      device_port(port_base +
                  0x6), // Initialize device register port at base + 6.
      command_port(port_base + 0x7), // Initialize command port at base + 7.
      control_port(port_base +
                   0x206) // Initialize control port at base + 0x206.
{
  this->master = master; // Set the master flag.
}

// Destructor for ATA class (currently no dynamic resources to free).
ATA::~ATA() {}

// identify(): Sends the IDENTIFY command to the device and prints its returned
// identification data.
void ATA::identify() {
  device_port.write(
      master ? 0xA0 : 0xB0); // Select master (0xA0) or slave (0xB0) device.
  control_port.write(0);     // Clear the control port.

  device_port.write(0xA0); // Set device register to master (if applicable).
  uint8_t status = command_port.read(); // Read the device status.
  if (status == 0xFF) // If status is 0xFF, the device is not present.
    return;

  device_port.write(master ? 0xA0
                           : 0xB0); // Re-select device based on master flag.
  sector_count_port.write(0);       // Write 0 to sector count.
  lba_low_port.write(0);            // Write 0 to LBA low.
  lba_mid_port.write(0);            // Write 0 to LBA mid.
  lba_high_port.write(0);           // Write 0 to LBA high.
  command_port.write(0xEC);         // Send the IDENTIFY command (0xEC).

  status = command_port.read(); // Read the status after issuing the command.
  if (status == 0x00)           // If status is 0x00, no device is present.
    return;

  // Wait until the device is not busy (BSY cleared) and no error bit is set.
  while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
    status = command_port.read();

  if (status & 0x01) {     // If an error is indicated...
    libc::printf("ERROR"); // ...print an error message.
    return;
  }

  uint16_t identify_data[256];
  for (int i = 0; i < 256; i++) {
    identify_data[i] = data_port.read();
  }

  // Extract the model number (words 27–46)
  char model[41] = {0}; // 40 characters + null terminator
  for (int i = 0; i < 20; i++) {
    model[i * 2] = identify_data[27 + i] & 0xFF;     // Low byte
    model[i * 2 + 1] = (identify_data[27 + i] >> 8); // High byte
  }
  libc::printf("Model Number: %s\n", model);

  // Extract the serial number (words 10–19)
  char serial[21] = {0}; // 20 characters + null terminator
  for (int i = 0; i < 10; i++) {
    serial[i * 2] = identify_data[10 + i] & 0xFF;
    serial[i * 2 + 1] = (identify_data[10 + i] >> 8);
  }
  libc::printf("Serial Number: %s\n", serial);

}

// read28(): Reads data from a given sector using 28-bit LBA addressing.
void ATA::read28(uint32_t sector_num, int count) {
  if (sector_num > 0x0FFFFFFF) // Ensure the sector number fits in 28 bits.
    return;

  // Set the device register with the top 4 bits of LBA and master/slave
  // selection.
  device_port.write((master ? 0xE0 : 0xF0) | ((sector_num & 0x0F000000) >> 24));
  error_port.write(0);        // Clear the error register.
  sector_count_port.write(1); // Set sector count to 1 (reading one sector).
  lba_low_port.write(sector_num & 0x000000FF);        // Write the LBA low byte.
  lba_mid_port.write((sector_num & 0x0000FF00) >> 8); // Write the LBA mid byte.
  lba_high_port.write((sector_num & 0x00FF0000) >>
                     16);   // Write the LBA high byte.
  command_port.write(0x20); // Send the READ command (0x20).

  uint8_t status =
      command_port.read(); // Read the status after issuing the command.
  // Wait until the device is ready (not busy) and no error has occurred.
  while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
    status = command_port.read();

  if (status & 0x01) {     // If an error is indicated...
    libc::printf("ERROR"); // ...print an error message.
    return;
  }

  libc::printf(
      "reading ATA Drive: "); // Inform that data reading is in progress.

  // Read the data in 16-bit chunks and print each chunk.
  for (int i = 0; i < count; i += 2) {
    uint16_t wdata = data_port.read(); // Read a 16-bit word from the data port.

    char *text = "  \0";    // Temporary buffer for output.
    text[0] = wdata & 0xFF; // Get the lower byte.
    if (i + 1 < count)
      text[1] = (wdata >> 8) & 0xFF; // Get the upper byte if within the count.
    else
      text[1] = '\0';   // Otherwise, null-terminate the string.
    libc::printf(text); // Print the two-character string.
  }

  // Read and discard any remaining words to complete a full sector (512 bytes).
  for (int i = count + (count % 2); i < 512; i += 2)
    data_port.read();
}

// write28(): Writes data to a given sector using 28-bit LBA addressing.
void ATA::write28(uint32_t sector_num, uint8_t *data, uint32_t count) {
  if (sector_num >
      0x0FFFFFFF) // Validate that the sector number fits in 28 bits.
    return;
  if (count >
      512) // Ensure that no more than one sector (512 bytes) is written.
    return;

  // Set the device register with the upper LBA bits and select master/slave.
  device_port.write((master ? 0xE0 : 0xF0) | ((sector_num & 0x0F000000) >> 24));
  error_port.write(0);                         // Clear the error register.
  sector_count_port.write(1);                  // Set sector count to 1.
  lba_low_port.write(sector_num & 0x000000FF); // Write the LBA low byte.
  lba_mid_port.write((sector_num & 0x0000FF00) >> 8); // Write the LBA mid byte.
  lba_high_port.write((sector_num & 0x00FF0000) >>
                     16);   // Write the LBA high byte.
  command_port.write(0x30); // Send the WRITE command (0x30).

  libc::printf(
      "Writing to ATA Drive: "); // Inform that data writing is in progress.
  // Write the data in 16-bit chunks.
  for (int i = 0; i < count; i += 2) {
    uint16_t wdata = data[i]; // Get the first byte.
    if (i + 1 < count)
      wdata |= ((uint16_t)data[i + 1])
               << 8;        // Combine with the second byte if available.
    data_port.write(wdata); // Write the 16-bit word to the data port.

    char *text = "  \0";           // Temporary buffer for output.
    text[0] = (wdata >> 8) & 0xFF; // Extract the upper byte.
    text[1] = wdata & 0xFF;        // Extract the lower byte.
    libc::printf(text);            // Print the two-character string.
  }

  // Write zero padding if less than 512 bytes of data were provided.
  for (int i = count + (count % 2); i < 512; i += 2)
    data_port.write(0x0000); // Write zero to complete the sector.
}

// flush(): Flushes the ATA device's write cache.
void ATA::flush() {
  device_port.write(master ? 0xE0
                           : 0xF0); // Select the device (master or slave).
  command_port.write(0xE7);         // Send the FLUSH command (0xE7).
  uint8_t status =
      command_port.read(); // Read the status after the flush command.
  if (status == 0x00)      // If status indicates no activity, return.
    return;

  // Wait until the device is ready (not busy) and check for errors.
  while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
    status = command_port.read();

  if (status & 0x01) {     // If an error occurred...
    libc::printf("ERROR"); // ...print an error message.
    return;
  }
}

} // namespace driver
} // namespace uqaabOS
