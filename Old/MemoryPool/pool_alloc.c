#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pool_alloc.h"

static uint8_t g_pool_heap[65536];

// Ideally, run some statistics on usage to tune this appropriately.
#define MAX_BLOCK_SIZE_COUNT 8

// If we make the blocks too small, we wind up spending tons of
// memory on bookkeeping
#define MIN_BLOCK_SIZE (sizeof(int))

// 65536 up there should probably be a #define;
// at the very least we'll need to use it a few times
#define TOTAL_HEAP_SIZE 65536

// are we allowed to use math.h? well, this is easy anyway
#define CEILING_DIVIDE(dividend, divisor) (((dividend) + ((divisor)-1)) / (divisor))

// For each block size requested by the user, we must keep track of:
// - The size requested
// - How many chunks of this size we're going to allow the user to ask for
// - Within this pool, which of the possible blocks are taken (a bit vector)
// - Where we're putting the memory pool for this block size
// The bit vector is tucked into low memory in each pool.
typedef uint32_t bitflags;
#define BITFLAGS_NBITS 32

typedef struct {
  size_t block_size;
  unsigned int num_blocks;
  bitflags *blocks_allocated_flags;
  uint8_t *block_pool_base;
} pool_data;

// Is it cheating to have these as file-level static variables?
// It's a minor tweak to stuff them into low memory in the heap.
pool_data g_pool_data[MAX_BLOCK_SIZE_COUNT];
int g_num_pools;

bool pool_init(size_t* block_sizes, size_t block_size_count)
{
  int i, j;
  int bytes_per_pool;
  uint8_t *current_heap_location = &(g_pool_heap[0]);
  size_t sorted_block_sizes[block_size_count];

  if (block_size_count > MAX_BLOCK_SIZE_COUNT) {
    return false;
  }

  if (block_sizes == NULL) {
    return false;
  }

  // separate this loop from the sort purely for readability
  for (i = 0; i < block_size_count; i++) {
    if ((block_sizes[i]) < MIN_BLOCK_SIZE) {
      // Too much bookkeeping!  Also, alignment.
      return false;
    }
  }

  // It's going to be helpful to have these chunks in size order
  // for things like "failing upward" in allocation requests
  // (qsort would probably be my friend here, but this will do).
  // Arbitrarily selecting largest-to-smallest ordering.
  for (i = 0; i < (block_size_count - 1); i++) {
    int biggest = i;
    size_t tmp;
    for (j = i+1; j < block_size_count; j++) {
      if (block_sizes[j] > block_sizes[biggest]) {
	biggest = j;
      }
    }
    if (biggest != i) {
      tmp = block_sizes[i];
      block_sizes[i] = block_sizes[biggest];
      block_sizes[biggest] = tmp;
    }
  }

  // Possible improvement: If two requested sizes are the same, merge them.

  // For now, allocate equally to all block sizes.
  // Ideally, run statistics to see the usage pattern of the application
  // and tune this allocation based on those.
  bytes_per_pool = TOTAL_HEAP_SIZE / block_size_count;

  for (i = 0; i < block_size_count; i++) {
    int j, num_blocks, bookkeeping_bytes, allocatable_bytes;

    num_blocks = bytes_per_pool / (int)(block_sizes[i]); // max that will fit
    if (num_blocks < 1) {
      // Possible improvement: make a bigger pool in this case,
      // so that we can offer at least one block at this requested size,
      // then divide the remaining heap into smaller portions.
      // (In some cases, pools for very small items may then be zero size.)
      return false;
    }

    // As long as "bitflags" is a multiple of 32 bits in size,
    // the allocatable memory in the pool will also be 32 bit aligned.
    bookkeeping_bytes = 4 * (CEILING_DIVIDE(num_blocks, BITFLAGS_NBITS));
    allocatable_bytes = bytes_per_pool - bookkeeping_bytes;
    g_pool_data[i].block_size = block_sizes[i];
    g_pool_data[i].num_blocks = allocatable_bytes / (int)(block_sizes[i]);
    g_pool_data[i].blocks_allocated_flags = (bitflags *)current_heap_location;
    g_pool_data[i].block_pool_base = current_heap_location + bookkeeping_bytes;

    // Bookkeeping took away some space, so it's possible we have fewer blocks
    // than we initially thought.
    if (g_pool_data[i].num_blocks < 1) {
      // Possible improvement as noted above
      return false;
    }

    // all blocks in this pool are free
    for (j = 0; j < bookkeeping_bytes / 4; j++) {
      g_pool_data[i].blocks_allocated_flags[j] = 0;
    }

    current_heap_location += bytes_per_pool;
  }

  // good practice: initialize even what we think we'll never use
  for (; i < MAX_BLOCK_SIZE_COUNT; i++) {
    g_pool_data[i].block_size = 0;
    g_pool_data[i].num_blocks = 0;
    g_pool_data[i].blocks_allocated_flags = NULL;
    g_pool_data[i].block_pool_base = NULL;
  }

  // This line will need to change if "possible improvement" noted above.
  g_num_pools = block_size_count;

  return true;
}

// REQUIRES that one and only one bit be set in 'flags'
// This has no error checking!
int which_bit_is_set(bitflags flags)
{
  int i;
  for (i = 0; i < BITFLAGS_NBITS; i++) {
    if (flags & 1) {
      return i;
    }
    flags = flags >> 1;
  }
  // assert here
}

void* malloc_within_pool(int pool_num)
{
  int i;
  int num_ints_for_flags; // how many integers this pool uses for flags
  int flagset; // index of the integer containing the flag we care about
  int highest_flag; // bit position of highest valid flag in highest int
  int flag_bit_position; // bit position of the flag we care about
  bitflags offset_flag;
  uint8_t *block_address;

  if ((pool_num < 0) || (pool_num >= g_num_pools)) {
    return NULL;
  }

  // Where are my flags for this pool?
  num_ints_for_flags = CEILING_DIVIDE(g_pool_data[pool_num].num_blocks, BITFLAGS_NBITS);
  highest_flag = (g_pool_data[pool_num].num_blocks - 1) % BITFLAGS_NBITS;

  // Does this pool have anything available?
  offset_flag = 0;
  for (flagset = 0; flagset < num_ints_for_flags; flagset++) {
    bitflags x = g_pool_data[pool_num].blocks_allocated_flags[flagset];
    offset_flag = (~x) & (x + 1);  // this computes the rightmost zero bit in x
    if (offset_flag != 0) {
      break;
    }
  }
  if (offset_flag == 0) {
    // didn't find any, sorry
    return NULL;
  }
  flag_bit_position = which_bit_is_set(offset_flag);
  if ((flagset == num_ints_for_flags - 1) // we're in the most significant flags
      && flag_bit_position > highest_flag) {
    // this flag that we found is off the end of our usable set of blocks
    // so we still didn't find any memory to return, sorry
    return NULL;
  }

  // Boy is this ever not thread safe
  // 1. Claim the block
  g_pool_data[pool_num].blocks_allocated_flags[flagset] |= offset_flag;
  // 2. Find the address of the block we just claimed
  block_address = g_pool_data[pool_num].block_pool_base;
  block_address += g_pool_data[pool_num].block_size * flagset * BITFLAGS_NBITS;
  block_address += g_pool_data[pool_num].block_size * flag_bit_position;
  // 3. Give that block to the caller
  return (void *)(block_address);
}

void* pool_malloc(size_t n)
{
  int i;
  void *retval;

  // Search pools from smallest to largest, and
  // return the first allocation that succeeds
  for (i = g_num_pools - 1; i >= 0; i--) {
    if (g_pool_data[i].block_size < n) {
      continue;
    }
    retval = malloc_within_pool(i);
    if (retval != NULL) {
      break;
    }
  }

  // Possible improvement: if n is larger than the block size of the
  // largest memory we have available, check for adjacent blocks within a
  // smaller-block-size pool and allocate them together.
  // Freeing will then become more complex.

  return retval;
}

void print_pool_flagset(int pool_num)
{
  int flagset;
  bitflags bf;
  int num_ints_for_flags = CEILING_DIVIDE(g_pool_data[pool_num].num_blocks, BITFLAGS_NBITS);

  for(flagset = 0; flagset < num_ints_for_flags; flagset++) {
    bitflags bf = g_pool_data[pool_num].blocks_allocated_flags[flagset];
    printf("pool %d   flags %3d  =  0x%x\n", pool_num, flagset, bf);
  }
}

void pool_free(void* ptr)
{
  int pool_num;
  int offset_within_pool;
  uint8_t *min_allocatable_addr; // lowest address in pool's allocatable space
  uint8_t *max_allocatable_addr; // highest address in pool's allocatable space
  uint8_t *ptr_to_free = ptr; // sorry, I just got sick of casting
  int flagset;
  int flag_bit_position;
  bitflags flag_mask;

  // 1. Figure out what pool this pointer is in
  // and if we can't figure that out, don't try to free anything at all
  for (pool_num = 0; pool_num < g_num_pools; pool_num++) {
    min_allocatable_addr = g_pool_data[pool_num].block_pool_base;
    max_allocatable_addr = min_allocatable_addr +
      (g_pool_data[pool_num].block_size * g_pool_data[pool_num].num_blocks) - 1;
    if ((ptr_to_free >= min_allocatable_addr) &&
	(ptr_to_free <= max_allocatable_addr)) {
      // we've found the pool this pointer belongs to
      break;
    }
  }

  // Did we find it or not?
  if (pool_num >= g_num_pools) {
    // oops, don't touch anything
    // assert? We should have SOME kind of error reporting here!
    return;
  }

  // Possible improvement: check that the pointer is exactly the base
  // of the block.
  // As it is, this code will free the block containing the given address.

  // 2. Find the flag we need to clear to indicate this pointer is freed
  offset_within_pool = ptr_to_free - g_pool_data[pool_num].block_pool_base;
  flagset = (offset_within_pool / g_pool_data[pool_num].block_size) / BITFLAGS_NBITS;
  flag_bit_position = (offset_within_pool / g_pool_data[pool_num].block_size) % BITFLAGS_NBITS;

  // 3. Clear that flag
  // (first, set a single bit in the position we're interested in,
  // then invert the whole thing to create a mask,
  // then AND the mask in to clear the bit we're interested in.)
  // NOTE: if this memory was already free, there is no error thrown.
  flag_mask = 1 << flag_bit_position;
  flag_mask = ~ flag_mask;
  g_pool_data[pool_num].blocks_allocated_flags[flagset] &= flag_mask;

  // print_pool_flagset(pool_num); // debugging

  return;
}
