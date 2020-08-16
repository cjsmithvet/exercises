#include <stdio.h>
#include <malloc.h>

#define NO_MORE_INPUT -1

struct slnode {
  struct slnode *next;
  int val;
};

struct dlnode {
  struct dlnode *next;
  struct dlnode *prev;
  int val;
};

// Create a new node and tack it onto the "next" of the given node
struct slnode *make_new_node(int val, struct slnode *put_it_here)
{
  struct slnode *new_node = malloc(sizeof(struct slnode));
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
struct slnode *make_list()
{
  int i;
  struct slnode *current_node;
  struct slnode *head;

  head = current_node = NULL;
  while (1) {
    i = get_number_from_user();
    if (i == NO_MORE_INPUT) break;
    current_node = make_new_node(i, current_node);
    if (head == NULL) head = current_node;
  }    

  return head;
}

void print_slist(struct slnode *head)
{
  struct slnode *current_node = head;
  while (current_node != NULL) {
    printf("%d ", current_node->val);
    current_node = current_node->next;
  }
  printf("\n");
}

void print_dlist(struct dlnode *head)
{
  struct dlnode *current_node = head;
  while (current_node != NULL) {
    printf("%d ", current_node->val);
    current_node = current_node->next;
  }
  printf("\n");
}

void print_dlist_backwards(struct dlnode *head)
{
  struct dlnode *current_node = head;
  while (current_node->next != NULL) {
    current_node = current_node->next;
  }
  // current_node now points to the last valid dlnode
  while (current_node != NULL) {
    printf("%d ", current_node->val);
    current_node = current_node->prev;
  }
  printf("\n");
}

struct dlnode *make_doubly_linked(struct slnode *slist)
{
  struct dlnode *dlist;
  struct dlnode *tmp;
  struct dlnode *dnewest;

  if (slist == NULL) {
    return NULL;
  }
  dlist = malloc(sizeof(struct dlnode));
  if (dlist == NULL) {
    printf("Out of memory!\n");
    exit(1);
  }
  dlist->next = NULL; // for now - if there are more, we'll get this in the loop
  dlist->val = slist->val;
  dlist->prev = NULL;
  tmp = dlist;
  while (slist->next != NULL) {
    slist = slist->next;
    dnewest = malloc(sizeof(struct dlnode));
    if (dnewest == NULL) {
      printf("Out of memory!\n");
      exit(1);
    }
    tmp->next = dnewest;
    dnewest->prev = tmp;
    dnewest->val = slist->val;
    dnewest->next = NULL;
    tmp = dnewest;
  }
  return dlist;
}

int main () {
  int i, user_val;
  struct slnode *slist;
  struct dlnode *dlist;

  slist = make_list();
  if (!slist) return -1;

  printf("Here's your list: ");
  print_slist(slist);

  dlist = make_doubly_linked(slist);
  printf("I made it doubly linked now.  Here it is again: ");
  print_dlist(dlist);
  printf("And here it is backwards, for fun: ");
  print_dlist_backwards(dlist);

  return 0;
}
