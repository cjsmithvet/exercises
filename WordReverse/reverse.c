#include <stdio.h>
#define MAX_INPUT 80

int reverse_chars(char *input, int len)
{
  int i;

  if (len >= MAX_INPUT) {
    // printf("reverse_chars returning %d\n", 1);
    return 1;
  }

  for (i = 0; i < len/2; i++) {
    char tmp = input[i];
    input[i] = input[(len - i) - 1];
    input[(len - i) - 1] = tmp;
  }

  // printf("%s\n", input);
  // printf("reverse_chars returning %d\n", 0);
  return 0;
}

int is_whitespace(char c)
{
  if ((c == ' ') || (c == '\t') || (c == '\n')) return 1;
  return 0;
}

int reverse_words(char *input)
{
  int i, err;
  char *current_word;
  int current_word_len;
  int len = strlen(input);

  if (len >= MAX_INPUT) {
    return 1;
  }

  err = reverse_chars(input, len);
  if (err) return err;

  for (i = 0; i < len; i++) {
    current_word = input + i;
    while (!is_whitespace(input[i]) && (input[i] != '\0')) {
      i++;
      current_word_len++;
    }
    err = reverse_chars(current_word, current_word_len);
    current_word_len = 0;
  }

  return err;
}

// presumes "line" has enough memory for MAX_INPUT
// will initialize "line" to all NULLs if no input
// returns length of line obtained
int getaline (char *line)
{
  int i;
  char c;

  // clear "line"
  for (i = 0; i < MAX_INPUT; i++)
    line[i] = '\0';

  i = 0;
  while ((c = getchar()) != '\n')
    line[i++] = c;

  return i;
}

int main()
{
  char input[MAX_INPUT];
  int len, err;

  printf("Gimme some words!\n");
  // scanf("%s\n", input);
  len = getaline(input);

  err = reverse_words(input);
  if (err != 0) {
    printf("Unknown error %d!\n", err);
    return err;
  }

  printf("I munged them for you!\n");
  printf("%s\n", input);

  return 0;
}
