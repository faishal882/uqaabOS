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

            data_port.write(value);
        }

        bool PCIController::device_has_functions(uint16_t bus, uint16_t device)
        {
            /*Checks if bit 7 is set, indicating that the device supports multiple functions.*/
            return read(bus, device, 0, 0x0E) & (1 << 7);
        }

        void PCIController::select_drivers(DriverManager *driver_manager, uqaabOS::interrupts::InterruptManager *interrpt_manager)
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
                            continue;
                        }

                        /*bar_num variable refers to the base address register number 0-5 in case of header type 0 or 0-1 in case of header type 1*/
                        for (int bar_num = 0; bar_num < 6; bar_num++)
                        {

                            BaseAdressRegister bar = get_base_adress_register(bus, device, function, bar_num);

                            if (bar.address && bar.type == input_output)
                                dev.port_base = (uint32_t)bar.address;

                            Driver *driver = get_driver(dev, interrpt_manager);
                            // libc::printf();

                            if (driver != 0)
                            {
                                driver_manager->add_driver(driver);
                            }
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

        BaseAdressRegister PCIController::get_base_adress_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar)
        {

            BaseAdressRegister result;

            uint32_t header_type = read(bus, device, function, 0x0E) & 0x7F;

            int max_bars = 6 - (4 * header_type);

            if (bar >= max_bars)
                return result;

            uint32_t bar_value = read(bus, device, function, 0x10 + 4 * bar);
            result.type = (bar_value & 0x1) ? input_output : memory_mapping;

            uint32_t temp;

            if (result.type == memory_mapping)
            {

                switch ((bar_value >> 1) & 0x3)
                {

                case 0:
                case 1:
                case 2:
                    break;
                }
            }
            else
            {

                result.address = (uint8_t *)(bar_value & ~0x3);
                result.prefetchable = false;
            }

            return result;
        }

        Driver *PCIController::get_driver(PeripheralComponentInterconnectDeviceDescriptor dev, uqaabOS::interrupts::InterruptManager *interrupt_manager)
        {

            switch (dev.vendor_id)
            {
            case 0x1022: // AMD
                switch (dev.device_id)
                {
                case 0x2000:
                    libc::printf("AMD RYZEN");
                    break;
                }

                break;
            case 0x8086: // Intel
                switch (dev.device_id)
                {
                case 0x1237:
                    libc::printf("QEMU INTEL");
                    break;
                }
                break;
            }

            switch (dev.class_id)
            {
            case 0x03: // graphics
                switch (dev.sub_class_id)
                {
                case 0x00: // VGA
                    libc::printf("VGA ");
                    break;
                }
                break;
            }

            return 0;
        }

    }
}