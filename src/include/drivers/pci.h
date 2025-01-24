#ifndef __PCI_H_
#define __PCI_H_

#include <stdint.h>

#include "../port.h"
#include "driver.h"
#include "../interrupts.h"

namespace uqaabOS
{
    namespace driver
    {

        enum BaseAdressRegisterType{
            memory_mapping = 0,
            input_output = 0
        };

        class BaseAdressRegister{
            public:
            bool prefetchable;
            uint8_t* address;
            uint32_t size;
            BaseAdressRegisterType type;
        };

       /*represents metadata about a single PCI device.*/
        class PeripheralComponentInterconnectDeviceDescriptor
        {

        public:

            /*Represents the base I/O or memory address used by the device for communication.*/
            uint32_t port_base;

            /*Indicates the interrupt line or vector used by the device*/
            uint32_t interrupt;

            /*PCI bus number (0–255).(can be represent in 8 bit)*/
            uint16_t bus;

            /*Device number on the bus (0–31).(cane be represent in 5 bit)*/
            uint16_t device;

            /*Function number of the device (0–7, for multifunction devices).(can be represent in 3 bit)*/
            uint16_t function;

            /*Identifies the manufacturer of the device (e.g., Intel, NVIDIA, AMD).*/
            uint16_t vendor_id;

            /*Enables the OS to match a driver with the device.*/
            uint16_t device_id;

            uint8_t class_id;
            uint8_t sub_class_id;

            /*Provides additional details about how the device operates or communicates.*/
            uint8_t interface_id;

            /*Identifies the specific revision or version of the hardware.*/
            uint8_t revision;


            PeripheralComponentInterconnectDeviceDescriptor();
            ~PeripheralComponentInterconnectDeviceDescriptor();
        };




        /*PCI controller is a hardware interface used to connect peripheral devices to the motherboard
         -> Enable communication with peripherals like network cards, GPUs, and storage devices.
        */
        class PCIController
        {

            /* data_port use I/O ports to communicate configuration data, such as device/vendor IDs or BARs*/
            include::Port32Bit data_port;

            /*I/O port for sending commands to the PCI controller.*/
            include::Port32Bit command_port;

        public:
            PCIController();
            ~PCIController();

            /*Reads a 32-bit value from a specific PCI device's configuration space.*/
            uint32_t read(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset);

            /*Writes a 32-bit value to a specific PCI device's configuration space.*/
            void write(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset, uint32_t value);

            /*Checks if a PCI device is multifunctional*/
            bool device_has_functions(uint16_t bus, uint16_t device);

            /*suitable drivers with detected PCI devices.*/
            void select_drivers(DriverManager *driver_manager , uqaabOS::interrupts::InterruptManager* interrupt_manager);

            /*retrieves a descriptor containing details about a PCI device.*/
            PeripheralComponentInterconnectDeviceDescriptor get_device_descriptor(uint16_t bus , uint16_t device , uint16_t function);

            Driver* get_driver(PeripheralComponentInterconnectDeviceDescriptor dev , uqaabOS::interrupts::InterruptManager* interrupt_manager);

            BaseAdressRegister get_base_adress_register(uint16_t bus , uint16_t device , uint16_t function , uint16_t bar);

        };
    }
}
#endif