#include <stdio.h>
#include <malloc.h>
#include "binarytree.h"

#define MAXVALS 80

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
void fill_tree()
{
  int i, val;

  // ask the user for values for the rest of it
  for (i = 0; i < MAXVALS; i++) {
    val = get_number_from_user();
    if (val == NO_MORE_INPUT) break;
    add_to_tree(val);
  }    
}

int main ()
{
  fill_tree();
  printf("Here's your tree: ");
  print_tree();

  return 0;
}
