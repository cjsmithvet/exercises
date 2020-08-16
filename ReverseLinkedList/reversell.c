#include <stdio.h>
#include <malloc.h>

#define NO_MORE_INPUT -1

struct node {
  struct node *next;
  int val;
};

// Create a new node and tack it onto the "next" of the given node
struct node *make_new_node(int val, struct node *put_it_here)
{
  struct node *new_node = malloc(sizeof(struct node));
  if (new_node != NULL) {
    if (put_it_here != NULL) put_it_here->next = new_node;
    new_node->val = val;
    new_node->next = NULL;
  }
  return new_node;
}

// User has to type -1 to get this to return NO_MORE_INPUT sadly
// NO_MORE_INPUT will mean quit making any more 
int get_number_from_user()
{
  int i;
  // char line[80];

  printf("Pick a number, any number: ");
  // scanf("%s", &line);
  scanf("%d", &i);
  // printf("You picked %d\n", i);
  return i;
}

// Read in a bunch of numbers and make a linked list in the order given
struct node *make_list()
{
  int i;
  struct node *current_node;
  struct node *head;

  head = current_node = NULL;
  while (1) {
    i = get_number_from_user();
    if (i == NO_MORE_INPUT) break;
    current_node = make_new_node(i, current_node);
    if (head == NULL) head = current_node;
  }    

  return head;
}

void print_list(struct node *head)
{
  struct node *current_node = head;
  while (current_node != NULL) {
    printf("%d ", current_node->val);
    current_node = current_node->next;
  }
  printf("\n");
}

struct node *reverse_list(struct node *current)
{
  struct node *next;
  struct node *new_head;

  if (!current) return NULL;
  if (current->next == NULL) {
    // reached the end of the list
    return current;
  }

  next = current->next;
  current->next = NULL; // turns out "==" doesn't work here - who knew?
  new_head = reverse_list(next);
  next->next = current;

  return new_head;
}

struct node *reverse_list_not_recursive(struct node *current)
{
  struct node *a;
  struct node *b;

  if (current == NULL) return current;
  a = current->next;
  if (a == NULL) return current;
  current->next = NULL;
  b = a->next;

  while(1) {
    a->next = current;
    current = a;
    a = b;
    if (a == NULL) return current;
    b = b->next;
  }
}

int main () {
  int i, user_val;
  struct node *list;

  list = make_list();
  if (!list) return -1;

  printf("Here's your list: ");
  print_list(list);

  printf("Here it is, reversed: ");
  list = reverse_list_not_recursive(list);
  print_list(list);

  return 0;
}
