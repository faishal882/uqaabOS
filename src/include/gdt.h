#ifndef __GDT_H
#define __GDT_H

#include <stdint.h>

namespace uqaabOS {
namespace include {

#define NULL_DESCRIPTOR 0
#define UNUSED_DESCRIPTOR 1
#define CODE_DESCRIPTOR 2
#define DATA_DESCRIPTOR 3

//points to the location of gdt , limit:
class GDTPointer {
private:
  uint32_t limit;
  uint32_t base;

public:
  GDTPointer();
  ~GDTPointer();

  uint32_t getLimit();
  uint32_t getBase();
} __attribute__((packed));

//segment of global descriptor table
class GDTDescriptor {
private:
  uint16_t low_limit;
  uint16_t low_base;
  uint8_t mid_base;
  uint8_t access;
  uint8_t granularity; // high 4 bits (flags) low 4 bits (limit 4 last bits)(limit is 20 bit wide)
  uint8_t high_base;

public:
  GDTDescriptor();
  ~GDTDescriptor();
  
  uint32_t Base();
  uint32_t limit();
  void setGDTSegment(uint32_t base, uint32_t limit, uint8_t access);
               
} __attribute__((packed));

//necessary or basic descriptor which is present in GDT
extern GDTDescriptor gdt_entries[4];
extern GDTPointer firstGdt;

extern void init_gdt();

} // namespace include
} // namespace uqaabOS

#endif
