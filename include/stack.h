/* Stack.h - Header and type definitions for Stack ADT */
/* Written by Wyatt Carss in January 2011 */

#define _st(x) printf("Status is %d.\n", x);
#define STACK_STRING_LENGTH 64

struct _elmnt{
	char str[STACK_STRING_LENGTH];
	struct _elmnt *next;
	struct _elmnt *prev;
};

typedef struct _elmnt element;
typedef element * Element;

struct _stk{
	Element head;
	Element tail;
	int size;
};

typedef struct _stk stack;
typedef stack * Stack;

int new_stack(Stack *s);
int kill_stack(Stack *s);
int push(Stack s, char *str);
int pop(Stack s, char *str);
int peek(Stack s, char *str);
int is_empty(Stack s);
int stack_size(Stack s);

int new_element(Element *e, char *str, Element prev, Element next);
int kill_element(char *str, Element *tail);

