#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pool_alloc.h"

#define PASS(test) (printf("test \"%s\" passed! yay!\n", (test)))
#define FAIL(test) (printf("test \"%s\" failed! sadness!\n", (test)))

#define MANY 2048
void *g_tons_of_pointers_1[MANY];
void *g_tons_of_pointers_2[MANY];

// A simple initialization for alloc/free tests
#define NUM_BLOCK_SIZES 2
size_t g_block_sizes[NUM_BLOCK_SIZES] = {sizeof(int), 1024*sizeof(int)};
int g_num_blocks_available;

// print any successes or failures to stdout
bool test_init()
{
  bool all_passed = true;
  bool result;
  char *test;

  size_t block_too_big[1] = {(size_t)65536};
  size_t block_too_small[1] = {sizeof(uint8_t)};
  size_t too_many_blocks[32] = {sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long),
				sizeof(int), sizeof(long)};
  size_t one_block_size[1] = {sizeof(int)};
  size_t same_block_sizes[2] = {2*sizeof(int), 2*sizeof(int)};
  size_t two_block_sizes[2] = {2048*sizeof(int), 1024*sizeof(int)};

  printf("Testing initialization\n");

  // 1. no block sizes
  test = "no block sizes";
  result = pool_init(NULL, 3);
  if (result == false) {
    PASS(test);
  }
  else {
    FAIL(test);
    all_passed = false;
  }

  // 2. block too big to fit
  test = "block too big to fit";
  result = pool_init(block_too_big, sizeof(block_too_big) / sizeof(size_t));
  if (result == false) {
    PASS(test);
  }
  else {
    FAIL(test);
    all_passed = false;
  }

  // 3. block too small
  test = "block too small";
  result = pool_init(block_too_small, sizeof(block_too_small) / sizeof(size_t));
  if (result == false) {
    PASS(test);
  }
  else {
    FAIL(test);
    all_passed = false;
  }

  // 4. too many sizes of blocks
  test = "too many sizes requested";
  result = pool_init(too_many_blocks, sizeof(too_many_blocks) / sizeof(size_t));
  if (result == false) {
    PASS(test);
  }
  else {
    FAIL(test);
    all_passed = false;
  }

  // 5. one block size
  test = "only one block size";
  result = pool_init(one_block_size, sizeof(one_block_size) / sizeof(size_t));
  if (result == true) {
    PASS(test);
  }
  else {
    FAIL(test);
    all_passed = false;
  }

  // 6. block sizes the same (shouldn't matter, we didn't optimize for that)
  test = "duplicate block sizes";
  result = pool_init(same_block_sizes, sizeof(same_block_sizes) / sizeof(size_t));
  if (result == true) {
    PASS(test);
  }
  else {
    FAIL(test);
    all_passed = false;
  }

  // 7. two different block sizes
  test = "two different block sizes";
  result = pool_init(two_block_sizes, sizeof(two_block_sizes) / sizeof(size_t));
  if (result == true) {
    PASS(test);
  }
  else {
    FAIL(test);
    all_passed = false;
  }

  return all_passed;
}

bool generic_init()
{
  return pool_init(g_block_sizes, NUM_BLOCK_SIZES);
}

// Allocate until failure.
// Can print the addresses and eyeball them to make sure they make sense.
// "Sanity check" on the number of allocations happens to magically know
// the heap size.
bool count_allocations()
{
  int i;
  int large_size = 4096;
  size_t biggish[1] = {(size_t)large_size};
  bool retval = true;

  retval = pool_init(biggish, 1);
  if (retval == false) {
    FAIL("count_allocations initialization");
    return false;
  }

  for (i = 0; ; i++) {
    // Just allocate everything until it fails
    void *jnk = pool_malloc(sizeof(int));
    // printf("Allocation %d at location %d\n", i, (int)jnk);
    if (jnk == NULL) {
      break;
    }
  }

  // Number of blocks allocated is i.
  // Given that the size we asked for is a significant fraction of the
  // size of the heap, we shouldn't be using a ton of memory for flags,
  // so this should be almost all the blocks that would fit without overhead.
  if (i != ((65536 / large_size) - 1)) {
    FAIL("count_allocations blocks available");
    return false;
  }
  else
  {
    PASS("count_allocations blocks available");
  }

  return true;
}

// Allocate and free one block a bunch of times.
// Not all failures will be noted in the return value,
// since free fails silently, but we can at least check
// that subsequent allocations get the block that was just freed.
bool test_repeated_alloc_free()
{
  int i;
  void *first_ptr;
  void *current_ptr;

  first_ptr = pool_malloc(sizeof(int));
  if (first_ptr == NULL) {
    // This test is meant to be called with a bunch of memory available!
    return false;
  }
  pool_free(first_ptr);

  for (i = 0; i < 100; i++) {
    current_ptr = pool_malloc(sizeof(int));
    // printf("Allocation %d at location %d\n", i, (int)current_ptr);
    if (current_ptr != first_ptr) {
      return false;
    }
    pool_free(current_ptr);
  }

  return true;
}

int free_in_reverse_order(int n_iterations)
{
  int retval = n_iterations;
  void *jnk = pool_malloc(sizeof(int));
  if (jnk == NULL) {
    // all right, we ran it out of room, stop pestering it for more
    return retval; // here's where we stopped
  }

  // hey, this test doesn't have to be all that efficient, let's recurse!
  retval = free_in_reverse_order(n_iterations + 1);
  pool_free(jnk);
  return retval;
}

bool test_alloc_free()
{
  char *test;
  int iterations1, iterations2, i, j;
  bool pass = true;
  bool all_pass = true;

  // 1. allocate everything, free it all in reverse order,
  // and make sure it's all still available for the next go
  test = "allocate, free in reverse, allocate again";
  iterations1 = free_in_reverse_order(0);
  iterations2 = free_in_reverse_order(0);
  if (iterations1 != iterations2) {
    FAIL(test);
    all_pass = false;
  }
  else {
    PASS(test);
  }

  // 2. allocate a lot of items and
  // free them in the order allocated;
  // request them all again and make sure the results match
  test = "free in the order allocated, check they are all available";
  for (i = 0; i < MANY; i++) {
    g_tons_of_pointers_1[i] = pool_malloc(sizeof(uint32_t));
    if (g_tons_of_pointers_1[i] == NULL) {
      break;
    }
  }
  for (j = 0; j < i; j++) {
    pool_free(g_tons_of_pointers_1[j]);
  }
  for (i = 0; i < MANY; i++) {
    g_tons_of_pointers_2[i] = pool_malloc(sizeof(uint32_t));
    if (g_tons_of_pointers_2[i] == NULL) {
      break;
    }
  }
  for (j = 0; j < i; j++) {
    if (g_tons_of_pointers_1[j] != g_tons_of_pointers_2[j]) {
      pass = false;
    }
    pool_free(g_tons_of_pointers_2[j]);
  }
  if (pass == true) {
    PASS(test);
  }
  else {
    FAIL(test);
    all_pass = false;
  }

  // 3. allocate a lot of items and
  // free them in reverse order;
  // request them all again and make sure the results match
  test = "free in reverse order allocated, check all available";
  for (i = 0; i < MANY; i++) {
    g_tons_of_pointers_1[i] = pool_malloc(sizeof(uint32_t));
    if (g_tons_of_pointers_1[i] == NULL) {
      break;
    }
  }
  for (j = i - 1; j >= 0; j--) {
    pool_free(g_tons_of_pointers_1[j]);
  }
  for (i = 0; i < MANY; i++) {
    g_tons_of_pointers_2[i] = pool_malloc(sizeof(uint32_t));
    if (g_tons_of_pointers_2[i] == NULL) {
      break;
    }
  }
  for (j = 0; j < i; j++) {
    if (g_tons_of_pointers_1[j] != g_tons_of_pointers_2[j]) {
      pass = false;
    }
    pool_free(g_tons_of_pointers_2[j]);
  }
  if (pass == true) {
    PASS(test);
  }
  else {
    FAIL(test);
    all_pass = false;
  }

  return all_pass;
}

// This can't really fail, since free fails silently, but it could crash
void test_free_null()
{
  pool_free(NULL);
}

int main()
{
  bool result;
  bool all_passed = true;

  // Test a few initialization corner cases.
  result = test_init();
  if (result == false) {
    printf("Can't continue with testing; heap not initialized.\n");
    return -1;
  }

  // "Generic init" leaves the heap all free and available for use,
  // for the next test.
  result = generic_init();
  if (result == false) {
    all_passed = false;
    printf("Can't continue with testing; heap not initialized.\n");
    return -1;
  }

  // Sanity check the number of blocks it's giving us
  result = count_allocations();
  if (result == false) {
    all_passed = false;
    // OK to continue testing at this point
  }

  printf("Testing alloc / free\n");

  // We don't know that free works yet, so just start over at this point
  result = generic_init();
  if (result == false) {
    all_passed = false;
    printf("Can't continue with testing; heap not initialized.\n");
    return -1;
  }
  test_free_null();

  result = generic_init();
  if (result == false) {
    all_passed = false;
    printf("Can't continue with testing; heap not initialized.\n");
    return -1;
  }
  result = test_repeated_alloc_free();
  if (result == false) {
    FAIL("repeated_alloc_free");
    all_passed = false;
  }
  else {
    PASS("repeated_alloc_free");
  }

  result = generic_init();
  if (result == false) {
    all_passed = false;
    printf("Can't continue with testing; heap not initialized.\n");
    return -1;
  }
  result = test_alloc_free();
  if (result == false) {
    FAIL("alloc_free");
    all_passed = false;
  }
  else {
    PASS("alloc_free");
  }

  return 0;
}
