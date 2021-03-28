#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_KEY 256

typedef struct list_val{
	char value[500];
	struct list_val *next;
} List_value;

typedef struct s_Info {
	struct s_Info *dict_next;
	char key[MAX_KEY];
	int counter;
	int time_counter;
	List_value *pvalue;
	uint32_t hash;
} Info;

#define List_SIG 0x010A9842

typedef struct {
	int sig;
	Info *first;
} List;



List *createList();
void freeList(List *l);

void append(List *l, Info *s);
void append_value(Info *pi, List_value *lvalue);

Info *getFirstInfo(List *l);
Info *search(List *l, char *key, uint32_t keyHash);

void printList(List *l);
