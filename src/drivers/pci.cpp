#include "../include/drivers/pci.h"
#include "../include/libc/stdio.h"

namespace uqaabOS
{
    namespace driver
    {

        PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor() {};

        PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor() {};

        /*   Port 0xCF8: Command port where you write the address of the register you want to access.
          -> Port 0xCFC: Data port where you read/write the data from/to the specified register.
        */
        PCIController::PCIController()
            : data_port(0xCFC),
              command_port(0xCF8)
        {
        }

        PCIController::~PCIController() {}

        uint32_t PCIController::read(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset)
        {

            uint32_t id = 0x1 << 31;

            id |= ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (register_offset & 0xFC);

            command_port.write(id);

            uint32_t result = data_port.read();

            /*Adjusting for alignment: might not align with the beginning of the 4-byte chunk*/
            return result >> (8 * (register_offset % 4));
        }

        void PCIController::write(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset, uint32_t value)
        {

            uint32_t id = 0x1 << 31;

            id |= ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (register_offset & 0xFC);

            command_port.write(id);

            data_port.write(id);
        }

        bool PCIController::device_has_functions(uint16_t bus, uint16_t device)
        {
            /*Checks if bit 7 is set, indicating that the device supports multiple functions.*/
            return read(bus, device, 0, 0x0E) & (1 << 7);
        }

        void PCIController::select_drivers(DriverManager *driver_manager)
        {

            for (int bus = 0; bus < 8; bus++)
            {

                for (int device = 0; device < 32; device++)
                {

                    int num_functions = device_has_functions(bus, device) ? 8 : 1;

                    for (int function = 0; function < num_functions; function++)
                    {

                        PeripheralComponentInterconnectDeviceDescriptor dev = get_device_descriptor(bus, device, function);

                        if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
                        {
                            /*(invalid or unused) vendor_id*/
                            break;
                        }

                        libc::printf("PCI BUS ");
                        libc::print_hex(bus & 0xFF);

                        libc::printf(", DEVICE ");
                        libc::print_hex(device & 0xFF);

                        libc::printf(", FUNCTION ");
                        libc::print_hex(function & 0xFF);

                        libc::printf(" =  VENDOR ");
                        libc::print_hex((dev.vendor_id & 0xFF00) >> 8);
                        libc::print_hex(dev.vendor_id & 0xFF);

                        libc::printf(" DEVICE ID ");
                        libc::print_hex((dev.device_id & 0xFF00) >> 8);
                        libc::print_hex(dev.device_id & 0xFF);

                        libc::printf("\n");
                    }
                }
            }
        }

        PeripheralComponentInterconnectDeviceDescriptor PCIController::get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function)
        {

            PeripheralComponentInterconnectDeviceDescriptor result;
            result.device = device;
            result.bus = bus;
            result.function = function;

            result.vendor_id = read(bus, device, function, 0x00);
            result.device_id = read(bus, device, function, 0x02);

            result.class_id = read(bus, device, function, 0x0b);
            result.sub_class_id = read(bus, device, function, 0x0a);
            result.interface_id = read(bus, device, function, 0x09);

            result.revision = read(bus, device, function, 0x08);
            result.interrupt = read(bus, device, function, 0x03c);

            return result;
        }

    }
}