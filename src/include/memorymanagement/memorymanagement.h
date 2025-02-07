#ifndef __MEMORYMANAGEMENT_H
#define __MEMORYMANAGEMENT_H

#include <cstddef>
#include <stdint.h>

namespace uqaabOS {
namespace memorymanagement {

// @brief Typedef for storing size values of memory chunks.
typedef unsigned long size_t;

/**
 * MemoryChunk: Represents a chunk of memory in the memory manager.
 * MemoryChunk::next: Pointer to the next memory chunk.
 * MemoryChunk::prev: Pointer to the previous memory chunk.
 * MemoryChunk::allocated: Boolean flag indicating whether the chunk is allocated.
 * MemoryChunk::size: Size of the memory chunk.
 */
struct MemoryChunk {
  MemoryChunk *next;
  MemoryChunk *prev;
  bool allocated;
  size_t size;
};

/**
 * MemoryManager: Manages memory allocation and deallocation.
 * The MemoryManager class provides methods for allocating and freeing memory.
 * It maintains a linked list of MemoryChunk structures to keep track of memory usage.
 */
class MemoryManager {

protected:
  // Pointer to the first memory chunk.
  MemoryChunk *first;

public:
  // Static pointer to the currently active memory manager.
  static MemoryManager *active_memory_manager;

  /**
   * Constructs a MemoryManager object.
   * first: The starting address of the memory to manage.
   * size:  The size of the memory to manage.
   */
  MemoryManager(size_t first, size_t size);

  ~MemoryManager();

  // Allocates a block of memory, of size 'size'.
  void *malloc(size_t size);

  // Frees a previously allocated block of memory.
  void free(void *ptr);
};

} // namespace memorymanagement
} // namespace uqaabOS

void *operator new(uqaabOS::memorymanagement::size_t, void *ptr);
void *operator new[](uqaabOS::memorymanagement::size_t, void *ptr);
void operator delete(void *ptr);
void operator delete[](void *ptr);

#endif