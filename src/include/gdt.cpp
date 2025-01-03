#include "gdt.h"

// Without extern "C", the linker would fail to find the load_gdt
extern "C" void load_gdt(uqaabOS::include::GDTPointer *gdt);

void uqaabOS::include::GDTDescriptor::setGDTSegment(uint32_t base,
                                                    uint32_t limit,
                                                    uint8_t access) {
  // The target pointer is used for representation of the SegmentDescriptor
  // object.
  uint8_t *target = (uint8_t *)this;

  // encode the base
  target[2] = base & 0xFF;
  target[3] = (base >> 8) & 0xFF;
  target[4] = (base >> 16) & 0xFF;
  target[7] = (base >> 24) & 0xFF;

  // access / type
  target[5] = access;

  if (limit <= 65536) {
    // 16 bit adress space
    //  No need for granularity, therefore unset the granularity bit
    target[6] = 0x40;
  } else {

    // 32 bit adress space

    if ((limit & 0xFFF) != 0xFFF) {
      limit = (limit >> 12) - 1;
    } else {
      limit = (limit >> 12);
    }

    // need for granularity, therefore set the granularity bit
    target[6] = 0xC0;
  }

  // limit
  target[0] = limit & 0xFF;
  target[1] = (limit >> 8) & 0xFF;
  target[6] |= (limit >> 16) & 0xF;
}

// GDTDescriptor destructor
uqaabOS::include::GDTDescriptor::~GDTDescriptor() {}

// combine all the base into 32bits
uint32_t uqaabOS::include::GDTDescriptor::Base() {

  // The target pointer is used to representation of the SegmentDescriptor
  // object.
  uint8_t *target = (uint8_t *)this;

  // target[2] , target[3] -> lower bytes
  // target[5] -> middle byte
  // target[7] -> higher byte

  uint32_t baseResult = target[7];
  baseResult = (baseResult << 8) + target[4];
  baseResult = (baseResult << 8) + target[3];
  baseResult = (baseResult << 8) + target[2];

  return baseResult;
}

// combine all the limit in 20bits
uint32_t uqaabOS::include::GDTDescriptor::limit() {

  // The target pointer is used to representation of the SegmentDescriptor
  // object.
  uint8_t *target = (uint8_t *)this;

  // target[0] , target[1] -> lower bytes;
  // target[6] -> upper 4 bits

  uint32_t limitResult = target[6] & 0xF;
  limitResult = (limitResult << 8) + target[1];
  limitResult = (limitResult << 8) + target[0];

  if (((target[6] >> 4) & 0xC) == 0xC) {
    // granularity bit is set thats why we do
    limitResult = (limitResult << 12) | 0xFFF;
  }

  return limitResult;
}

// GDTPointer destructor
uqaabOS::include::GDTPointer::~GDTPointer() {}

// initialize necessary descriptor
void uqaabOS::include::init_gdt() {
  uqaabOS::include::GDTDescriptor gdt_entries[4];
  uqaabOS::include::GDTPointer firstGdt;

  // initialize all the descriptors
  gdt_entries[NULL_DESCRIPTOR].setGDTSegment(0, 0, 0);
  gdt_entries[UNUSED_DESCRIPTOR].setGDTSegment(0, 0, 0);
  gdt_entries[CODE_DESCRIPTOR].setGDTSegment(0, 64 * 1024 * 1024, 0x9A);
  gdt_entries[DATA_DESCRIPTOR].setGDTSegment(0, 64 * 1024 * 1024, 0x92);

  firstGdt.limit = (sizeof(GDTDescriptor) * 4) - 1;
  firstGdt.base = (uint32_t *)&gdt_entries;

  // load_gdt start implemenattion is in load_gdt.s file
  load_gdt(&firstGdt);
}
