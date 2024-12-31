#ifndef __GDT_H
#define __GDT_H

#include <stdint.h>

namespace uqaabOS {
namespace include {

class GDTPointer {
private:
  uint16_t limit;
  uint32_t base;

public:
  GDTPointer();
  ~GDTPointer();

  uint16_t getLimit();
  uint32_t getBase();
} __attribute__((packed));

class GDTDescriptor {
private:
  uint16_t low_limit;
  uint16_t low_base;
  uint8_t mid_base;
  uint8_t access;
  uint8_t granularity;
  uint8_t high_base;

public:
  GDTDescriptor();
  ~GDTDescriptor();

  void setGDTSegment(uint32_t base, uint32_t limit, uint8_t access,
                     uint8_t gran);
} __attribute__((packed));
} // namespace include
} // namespace uqaabOS

#endif
