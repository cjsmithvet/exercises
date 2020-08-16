#include <stdio.h>
#include <malloc.h>

#define NO_MORE_INPUT -1
#define MAXVALS 80
int array[MAXVALS];
int array_length;

// User has to type -1 to get this to return NO_MORE_INPUT sadly
// NO_MORE_INPUT will mean quit making any more 
int get_number_from_user()
{
  int i;

  printf("Pick a number, any number: ");
  scanf("%d", &i);
  // printf("You picked %d\n", i);
  return i;
}

// Read in a bunch of numbers and put them into our array in the order given
void fill_array()
{
  int i, val;

  // put something innocuous in for the stuff we may not fill
  for (i = 0; i < MAXVALS; i++) {
    array[i] = 0;
  }

  // ask the user for values for the rest of it
  for (i = 0; i < MAXVALS; i++) {
    val = get_number_from_user();
    if (val == NO_MORE_INPUT) break;
    array[i] = val;
  }    
  array_length = i;
}

void print_array()
{
  int i;
  for (i = 0; i < array_length; i++) {
    printf("%d ", array[i]);
  }
  printf("\n");
}

void insertion_sort()
{
  int i, j;
  int current_value;

  if (array_length < 2) {
    return;
  }

  for (i = 1; i < array_length; i++) {
    current_value = array[i];
    for (j = i - 1; (j >= 0) && (array[j] > current_value); j--) {
      array[j + 1] = array[j];
    }
    array[j + 1] = current_value;
  }
}

int main () {
  int i, user_val;

  fill_array();
  printf("Here's your array: ");
  print_array();

  if (array_length < 2) {
    printf("That's not worth sorting.\n");
  }
  else {
    insertion_sort();
    printf("Here, I sorted it: ");
    print_array();
  }

  return 0;
}
