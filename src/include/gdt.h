#ifndef __GDT_H
#define __GDT_H

#include <stdint.h>

namespace uqaabOS {
namespace include {

// segment of global descriptor table
class GDTDescriptor {
  /*  **** GDT Descriptor ****
     -> limit: low_limit(0-15bytes) + granularity(48-51bytes)(low 4bits of
               granualirity),
     -> base: low_base(16-31bytes) + mid_base(32-39bytes) +
              high_base(56-63bytes),
     -> access: access(40-47bytes),
     -> flags: granularity(52-55bytes)(high 4bits of granualirity),
     */
private:
  uint16_t low_limit;
  uint16_t low_base;
  uint8_t mid_base;
  uint8_t access;
  uint8_t granularity; // high 4 bits (flags) low 4 bits (limit)
  uint8_t high_base;

public:
  GDTDescriptor();
  GDTDescriptor(uint32_t base, uint32_t limit, uint8_t access);
  ~GDTDescriptor();

  uint32_t segment_base();
  uint32_t segment_limit();
} __attribute__((packed));

// global descriptor table(GDT)
class GDT {
private:
  // four GDTSegmentDescriptor necessary for basic kernel
  GDTDescriptor null_segment;
  GDTDescriptor unused_segment;
  GDTDescriptor code_segment;
  GDTDescriptor data_segment;

public:
  GDT();

  // Delete copy constructor and copy assignment operator
  GDT(const GDT &) = delete;
  GDT &operator=(const GDT &) = delete;
  ~GDT();

  uint16_t code_segment_selector();
  uint16_t data_segment_selector();
} __attribute__((packed));
} // namespace include
} // namespace uqaabOS
#endif
