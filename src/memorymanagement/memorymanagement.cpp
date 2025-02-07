#include "../include/memorymanagement/memorymanagement.h" 

namespace uqaabOS {
namespace memorymanagement {
/*
 * Memory Management Approach:
 * This implementation uses a linked list of memory chunks (MemoryChunk structs) to manage free and allocated memory blocks.
 * Each chunk contains metadata (size, allocated flag, prev/next pointers) followed by the actual memory space.
 * - First-fit allocation strategy: Scans from the first chunk to find the first sufficiently large free block.
 * - Splitting: When allocating, if remaining space is enough for another chunk, split into allocated chunk and new free chunk.
 * - Coalescing: When freeing memory, merge with adjacent free chunks to prevent fragmentation.
 * - Operators new/delete are overridden to use the active MemoryManager instance.
*/


// Static pointer to track the active memory manager instance
MemoryManager *MemoryManager::active_memory_manager = 0;

/* MemoryManager Constructor
 * Initializes memory manager with initial memory block
 * @param start: Starting address of the memory pool
 * @param size: Total size of the memory pool */
MemoryManager::MemoryManager(size_t start, size_t size) {
  active_memory_manager = this;  // Set this instance as active

  // Check if initial size is too small for even one MemoryChunk
  if (size < sizeof(MemoryChunk)) {
    first = 0;  // No chunks can be created
  } else {
    // Initialize first chunk at start address
    first = (MemoryChunk *)start;

    // Set initial chunk properties
    first->allocated = false;  // Mark as free
    first->prev = 0;           // No previous chunk
    first->next = 0;           // No next chunk
    // Calculate usable space (subtract chunk metadata size)
    first->size = size - sizeof(MemoryChunk);
  }
}

/* MemoryManager Destructor
 * Clears active manager if it's the current instance */
MemoryManager::~MemoryManager() {
  if (active_memory_manager == this)
    active_memory_manager = 0;
}

/* Memory Allocation Function
 * @param size: Requested memory size
 * @return: Pointer to allocated memory or 0 if failed */
void *MemoryManager::malloc(size_t size) {
  MemoryChunk *result = 0;

  // First-fit search: Iterate through chunks until suitable free chunk found
  for (MemoryChunk *chunk = first; chunk != 0 && result == 0;
       chunk = chunk->next) {
    // Check if chunk is free and has sufficient size
    if (chunk->size > size && !chunk->allocated)
      result = chunk;
  }

  if (result == 0)  // No suitable chunk found
    return 0;

  /* Split chunk if remaining space is enough for a new chunk (metadata + at least 1 byte)
   * This prevents creating chunks with zero usable space */
  if (result->size >= size + sizeof(MemoryChunk) + 1) {
    // Calculate position for new chunk after allocated space
    MemoryChunk *temp =
        (MemoryChunk *)((size_t)result + sizeof(MemoryChunk) + size);

    // Initialize new chunk properties
    temp->allocated = false;
    temp->size = result->size - size - sizeof(MemoryChunk);  // Update size
    temp->prev = result;     // Previous chunk is the original chunk
    temp->next = result->next;  // Next chunk becomes original's next
    if (temp->next != 0)
      temp->next->prev = temp;  // Update next chunk's previous pointer

    // Resize original chunk and update links
    result->size = size;      // Set to allocated size
    result->next = temp;      // Insert new chunk after original
  }

  result->allocated = true;  // Mark chunk as allocated
  // Return pointer to memory area after chunk metadata
  return (void *)(((size_t)result) + sizeof(MemoryChunk));
}

/* Memory Deallocation Function
 * @param ptr: Pointer to memory to be freed */
void MemoryManager::free(void *ptr) {
  // Get chunk metadata from memory pointer (subtract metadata size)
  MemoryChunk *chunk = (MemoryChunk *)((size_t)ptr - sizeof(MemoryChunk));
  chunk->allocated = false;  // Mark as free

  // Coalesce with previous chunk if it's free
  if (chunk->prev != 0 && !chunk->prev->allocated) {
    // Merge current chunk into previous
    chunk->prev->next = chunk->next;
    chunk->prev->size += chunk->size + sizeof(MemoryChunk);  // Combine sizes
    if (chunk->next != 0)
      chunk->next->prev = chunk->prev;  // Update next chunk's previous pointer

    chunk = chunk->prev;  // Move pointer to merged chunk
  }

  // Coalesce with next chunk if it's free
  if (chunk->next != 0 && !chunk->next->allocated) {
    chunk->size += chunk->next->size + sizeof(MemoryChunk);  // Absorb next chunk
    chunk->next = chunk->next->next;  // Skip over next chunk
    if (chunk->next != 0)
      chunk->next->prev = chunk;  // Update new next chunk's previous pointer
  }
}

} // namespace memorymanagement
} // namespace uqaabOS

/* Global Operator Overloads for Dynamic Memory Management */

// Single-object new operator
void *operator new(uqaabOS::memorymanagement::size_t size) {
  if (uqaabOS::memorymanagement::MemoryManager::active_memory_manager == 0)
    return 0;  // Safety check if no memory manager exists
  return uqaabOS::memorymanagement::MemoryManager::active_memory_manager->malloc(
      size);
}

// Array new operator
void *operator new[](uqaabOS::memorymanagement::size_t size) {
  if (uqaabOS::memorymanagement::MemoryManager::active_memory_manager == 0)
    return 0;
  return uqaabOS::memorymanagement::MemoryManager::active_memory_manager->malloc(
      size);
}

// Placement new operators (directly return given pointer)
void *operator new(uqaabOS::memorymanagement::size_t size, void *ptr) {
  return ptr;  // Construct object at specified location
}

void *operator new[](uqaabOS::memorymanagement::size_t size, void *ptr) {
  return ptr;
}

// Single-object delete operator
void operator delete(void *ptr) {
  if (uqaabOS::memorymanagement::MemoryManager::active_memory_manager != 0)
    uqaabOS::memorymanagement::MemoryManager::active_memory_manager->free(ptr);
}

// Array delete operator
void operator delete[](void *ptr) {
  if (uqaabOS::memorymanagement::MemoryManager::active_memory_manager != 0)
    uqaabOS::memorymanagement::MemoryManager::active_memory_manager->free(ptr);
}