#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_KEY 256

typedef struct s_Info {
	struct s_Info *dict_next;
	char key[MAX_KEY];
	int counter;
	void *pvalue;
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

Info *getFirstInfo(List *l);
Info *search(List *l, char *key, uint32_t keyHash);

void printList(List *l);
