#include "../include/gdt.h"

namespace uqaabOS
{
  namespace include
  {

    GDT::GDT()
        : null_segment(0, 0, 0), unused_segment(0, 0, 0),
          code_segment(0, 64 * 1024 * 1024, 0x9A),
          data_segment(0, 64 * 1024 * 1024, 0x92)
    {

      // load gdt

      struct
      {
        uint16_t limit;
        uint32_t base;
      } __attribute__((packed)) gdt_ptr;

      gdt_ptr.limit = sizeof(GDT) - 1;
      gdt_ptr.base = (uint32_t)this;

      // Load the GDT using inline assembly
      asm volatile(
          "lgdt %0"
          :              // no output operands
          : "m"(gdt_ptr) // memory operand
          : "memory");
    }

    GDT::~GDT()
    {
      // do nothing, if gdt is to be destroyed
    }

    uint16_t GDT::data_segment_selector()
    {
      return (uint8_t *)&data_segment - (uint8_t *)this;
    }

    uint16_t GDT::code_segment_selector()
    {
      return (uint8_t *)&code_segment - (uint8_t *)this;
    }

    // GDTSegmentDescriptor constructor
    GDTDescriptor::GDTDescriptor(uint32_t base, uint32_t limit, uint8_t access)
    {
      uint8_t *target = (uint8_t *)this;

      // 32-bit address space (64 MiB)
      // Squeeze the 32-bit limit into 20-bit limit (only the 20 least significant
      // bits are stored)
      if ((limit & 0xFFF) != 0xFFF)
        limit = (limit >> 12) - 1;
      else
        limit = limit >> 12;

      // encode flags
      target[6] = 0xC0; // 32-bit limit with GDT flags

      // encode limit
      target[0] = limit & 0xFF;
      target[1] = (limit >> 8) & 0xFF;
      target[6] |= (limit >> 16) & 0xF;

      // encode base
      target[2] = base & 0xFF;
      target[3] = (base >> 8) & 0xFF;
      target[4] = (base >> 16) & 0xFF;
      target[7] = (base >> 24) & 0xFF;

      // encode access
      target[5] = access;
    }

    // GDTSegmentDescriptor destructor
    GDTDescriptor::~GDTDescriptor()
    {
      // do nothing,if segment is to be destroyed
    }

    // Function to extract base from GDTSegmentDescriptor
    uint32_t GDTDescriptor::segment_base()
    {
      uint8_t *target = (uint8_t *)this;

      uint32_t result = target[7];
      result = (result << 8) + target[4];
      result = (result << 8) + target[3];
      result = (result << 8) + target[2];

      return result;
    }

    // Function to extract limit from GDTSegmentDescriptor
    uint32_t GDTDescriptor::segment_limit()
    {
      uint8_t *target = (uint8_t *)this;

      uint32_t result = target[6] & 0xF;
      result = (result << 8) + target[1];
      result = (result << 8) + target[0];

      if ((target[6] & 0xC0) == 0xC0)
        result = (result << 12) | 0xFFF;

      return result;
    }
  } // namespace include
} // namespace uqaabOS
