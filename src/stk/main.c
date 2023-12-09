/* Example program for stack ADT */
/* Written by Wyatt Carss, January 2011 */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "stack.h"

int main()
{
  int r = 0;
  char str[64];
  Stack s = NULL;
  strcpy(str, "test data");

  printf("Stack test.\n"); _st(r)

  r = new_stack(&s);
  printf("Stack init.\n"); _st(r)

  r = push(s, "one");
  printf("Pushed one.\n"); _st(r)

  r = push(s, "two");
  printf("Pushed two.\n"); _st(r)

  r = push(s, "three");
  printf("Pushed three.\n"); _st(r)

  r = peek(s, str);
  printf("Top of the stack is %s\n", str); _st(r)

  /* pop until empty */
  while (pop(s, str) != 2) {
    printf("Popped %s.\n", str); _st(r)
  }

  r = kill_stack(&s);
  printf("stack killed.\n"); _st(r)

  return 0;
}

