/* Stack ADT */
/* Written by Wyatt Carss in January 2011 */

#include <stdlib.h>
#include <string.h>
#include "stack.h"


/* Stack Constructor
 * Input: An existing, NULL stack pointer.
 * Output: 0 if success
 *	   1 if failure
 * Errors: malloc could fail.
 */
int new_stack(Stack *s)
{
  *s = malloc(sizeof(stack));

  if (*s == NULL) {
    return 1; /* failure */
  }

  (*s)->head = NULL;
  (*s)->tail = NULL;
  (*s)->size = 0;

  return 0; /* success */
}


/* Stack Destructor
 * Input: An existing, instantiated stack pointer.
 * Output: 0 if success
 * 	   1 if failure
 * Errors: if s is NULL, stack cannot be destroyed
 */
int kill_stack(Stack *s)
{
  char str[64] = {0};

  if (NULL == s) {
    return 1;
  }

  while (pop(*s, str) != 2);

  (*s)->head = NULL;
  (*s)->tail = NULL;
  (*s)->size = 0;

  free(*s);
  *s = NULL;

  return 0;
}



/* Pushes a string str onto stack s
 * Input: An existing stack to push onto
 * 	  A preallocated and properly formed string.
 * Output: 0 if success
 * 	   1 if failure
 * Errors: if s is NULL or if new element creation fails, push cannot complete
 */
int push(Stack s, char *str)
{
  int result = 0;

  if (s == NULL) { /* fail early */
    return 1;
  }

  if (s->tail == NULL) {
    /* if there is no tail, this element will be it.
       populate it with str, and
       set its prev and next to NULL
    */
    result = new_element(&(s->tail), str, NULL, NULL);
  } else {
    /* insert new element at tail->next,
       populate it with str, and
       set prev to tail, next to NULL
    */
    result = new_element(&(s->tail->next), str, s->tail, NULL);
    s->tail = s->tail->next;
  }

  if (result == 1) { /* if create failed */
    return 1;
  }

  /* increment # of elements */
  (s->size)++;

  return 0;
}



/* Pops a string off of the stack
 * Input: An existing stack s to be popped from.
 * 	  An existing string, to be filled with output.
 * Output: 0 if success
 * 	   1 if error
 * 	   2 if stack empty
 * Errors: if s is NULL, stack does not exist (cannot be popped from)
 * 	   if kill fails, element isn't properly removed
 *	   if tail is NULL, stack is empty - no more pops possible
 */
int pop(Stack s, char *str)
{
  int result = 0;

  if (s == NULL) {
    /* stack is null */
    return 1;
  }

  if (s->tail == NULL) {
    /* stack empty -- will happen during kill_stack. */
    return 2;
  }

  /* save tail into e */
  result = kill_element(str, &(s->tail));

  if (result == 1 || str == NULL) {
    /* if kill failed */
    return 1;
  }

  /* decrease # of elements */
  (s->size)--;

  return 0;
}



/* Captures top of the stack nondestructively
 * Input: Existing stack s
 * 	  Existing string to save result into
 * Output: 0 if success
 * 	   1 if error
 * 	   2 if stack empty
 * Errors: If stack is null, or str is null, cannot peek
 */
int peek(Stack s, char *str)
{
  if (s == NULL) {
    return 1;
  }

  if (s->tail == NULL) {
    return 2; /* stack is empty -- will happen during kill_stack */
  }

  if (str == NULL) {
    return 1; /* no space for str */
  }

  strcpy(str, s->tail->str);

  return 0;
}



/* Quickly determines if a stack is empty or not
 * Input: An existing stack
 * Output: 0 if stack is nonempty
 * 	   1 if stack does not exist
 * 	   1 if stack is empty
 * Errors: If the stack doesn't exist, cannot read
 */
int is_empty(Stack s)
{
  if (s == NULL) {
    return 1;
  }

  if (s->tail == NULL) {
    return 1;
  }

  return 0;

}


/* Returns stack size variable
 * Input: An existing stack
 * Output: int stack size; ranging from 1 to n
 * Errors: If stack is null, will return -1
*/
int stack_size(Stack s)
{
  if (s == NULL) {
    return -1;
  }

  return s->size;
}



/* Element Constructor
 * Input: NULL Element pointer,
 *	  string to populate element with,
 *	  prev and next ptrs
 * Output: 0 if success
 * 	   1 if failure
 * Errors: Malloc could fail.
 */
int new_element(Element *e, char *str, Element prev, Element next)
{
  *e = malloc(sizeof(element));

  if (*e == NULL) {
    return 1;
  }

  strcpy((*e)->str, str);
  (*e)->prev = prev;
  (*e)->next = next;

  return 0;
}



/* Element Destructor
 * Input: An existing element (the stack's tail)
 * 	  A string to store contents in
 * Output: 0 if success
 * 	   1 if failure
 * Errors: If tail or str are NULL, cannot proceed
 */
int kill_element(char *str, Element *tail)
{
  if (tail == NULL || str == NULL) {
    return 1; /* failure */
  }

  /* Copy away the element's contents */
  strcpy(str, (*tail)->str);

  if ((*tail)->prev != NULL) {
    /* If the tail is not the last element,
    	new tail is element beneath it
     	free the old tail
    	set new tail's next to NULL
    */
    *tail = (*tail)->prev;
    free((*tail)->next);
    (*tail)->next = NULL;
  } else {
    /* If the tail is the last element,
    	just free it and NULL it.
    */
    free(*tail);
    *tail = NULL;
  }

  return 0;
}

