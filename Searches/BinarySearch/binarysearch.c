#include <stdio.h>
#include <malloc.h>

#define NO_MORE_INPUT -1
#define NOT_FOUND -2

// I can't be arsed to do user input on this one, sorry.
// Fuss with these manually for testing.
#if 0
static int sorted_list[] = {1, 3, 5, 6, 8, 13, 15, 16, 19, 21, 22, 22, 22, 22, 28, 41, 78};
static int list_length = 17;
#endif

#if 0
static int sorted_list[] = {1, 3, 5};
static int list_length = 3;
#endif

static int sorted_list[] = {};
static int list_length = 0;

// User has to type -1 to get this to return NO_MORE_INPUT sadly
int get_number_from_user()
{
  int i;

  printf("Pick a number, any number (-1 to quit): ");
  scanf("%d", &i);
  // printf("You picked %d\n", i);
  return i;
}

void print_list()
{
  int i;
  for (i = 0; i < list_length; i++) {
    printf("%d, ", sorted_list[i]);
  }
  printf("\n");
}

int binary_find(int value, int *list, int len)
{
  int found_index;

  if (len < 1) return NOT_FOUND;
  if (len == 1) {
    if (list[0] == value) {
      return 0;
    }
    else {
      return NOT_FOUND;
    }
  }
  if (list[len/2] == value) {
    return (len/2);
  }
  else if (list[len/2] > value) {
    found_index = binary_find(value, list, len/2);
    if (found_index == NOT_FOUND) return NOT_FOUND;
  }
  else {
    found_index = binary_find(value, list + (len/2), len - (len/2));
    if (found_index == NOT_FOUND) return NOT_FOUND;
    // we are looking in the "second half"
    found_index += len/2;
  }
  return found_index; // turns out this line is important - who knew?
}

int binary_where_should_it_go(int value, int *list, int len)
{
  int found_index;

  if (len < 1) return NOT_FOUND;
  if (len == 1) {
    if (list[0] >= value) {
      return 0;
    }
    else {
      return NOT_FOUND;
    }
  }
  if (list[len/2] == value) {
    return (len/2);
  }
  else if (list[len/2] > value) {
    found_index = binary_where_should_it_go(value, list, len/2);
    if (found_index == NOT_FOUND) return len/2;
  }
  else {
    found_index = binary_where_should_it_go(value, list + (len/2), len - (len/2));
    if (found_index == NOT_FOUND) return NOT_FOUND;
    // we are looking in the "second half"
    found_index += len/2;
  }
  return found_index; // turns out this line is important - who knew?
}

int binary_find_iterative(int value, int *list, int len)
{
  int max = len - 1;
  int min = 0;

  while (1) {
    // handle the case where we ran out of places to look
    printf ("max %d, min %d\n", max, min);
    if (max < min) {
      return NOT_FOUND;
    }
    if (max == min) {
      if (list[max] == value) {
	return max;
      }
      else {
	return NOT_FOUND;
      }
    }
    // we didn't run out of places to look,
    // so look in the middle of what we've got
    if (list[max / 2] == value) {
      return (max / 2);
    }
    else if (list[max / 2] > value) {
      max /= 2;
    }
    else {
      min += (((max - min) / 2) ? ((max - min) / 2) : 1);
    }
  }
}

int binary_where_should_it_go_iterative(int value, int *list, int len)
{
  // never did get to this
  return NOT_FOUND;
}

int main () {
  int i, index;

  printf("We're finding a number in this list: ");
  print_list();
  // printf("If it's not there we'll tell you where it should be inserted.\n"); 

  while (1) {
    i = get_number_from_user();
    if (i == NO_MORE_INPUT) break;
    index = binary_find_iterative(i, sorted_list, list_length);
    if (index == NOT_FOUND) {
      printf("Couldn't find it!  ");
      index = binary_where_should_it_go(i, sorted_list, list_length);
      if (index == NOT_FOUND) {
	printf("It should be inserted at the end.\n");
      }
      else {
	printf("It should be inserted before index %d.\n", index);
      }
   }
    else {
      printf("Your number is at index %d\n", index);
    }
  }

  return 0;
}
