
#include <cstdint>
#include <stdint.h>

#include "../include/port.h"

namespace uqaabOS{
    namespace include{

        Port::Port(uint16_t portNumber){
            this -> portNumber = portNumber;
        }
        Port::~Port(){}


        // 8bit Port
        Port8Bit::Port8Bit(uint16_t portNumber):Port(portNumber){}

        Port8Bit::~Port8Bit(){}

        void Port8Bit::write(uint8_t data){
            write8(portNumber , data);
        }

        uint8_t Port8Bit::read(){
            return read8(portNumber);
        }

        //8bits slow port
        Port8BitSlow::Port8BitSlow(uint16_t portNumber):Port8Bit(portNumber){
        }

        Port8BitSlow::~Port8BitSlow(){}

        void Port8BitSlow::write(uint8_t data){
            write8Slow(portNumber , data);
        }
        
        //16bits Port
        Port16Bit::Port16Bit(uint16_t portNumber):Port(portNumber){}

        Port16Bit::~Port16Bit(){}

        void Port16Bit::write(uint16_t data){
            write16(portNumber , data);
        }

        uint16_t Port16Bit::read(){
            return read16(portNumber);
        }

        //32bits port 
        Port32Bit::Port32Bit(uint16_t portNumber):Port(portNumber){}

        Port32Bit::~Port32Bit(){}

        void Port32Bit::write(uint32_t data){
            write32(portNumber , data);
        }

        uint32_t Port32Bit::read(){
            return read32(portNumber);
        }

    }
}