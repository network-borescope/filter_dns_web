#include "dict.h"

static List *freelist = NULL;

static uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length) {
    size_t i = 0;
    uint32_t hash = 0;
    while (i != length) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    // convertendo para hashkey do dict
    //long long int a = (long long int)hash;
    //int hashKey = a % DICT_SIZE;
    //return hashKey;
    return hash;
}

void dict_info_free(Info *pi) {
	if(freelist == NULL){
		freelist = createList();
	}
  	append(freelist, pi);
}

static Info *dict_info_alloc(void) {
	Info *pi = getFirstInfo(freelist);
	if (pi) return pi;
  	return malloc(sizeof(Info));
}

void dict_list_flush(List *pl, FILE *global_fout) {
	Info *pi, *pa;
	pi = pl->first;
	if (!pi) return;
	for (pa = NULL; pi; pa = pi, pi = pi->dict_next) {
		fprintf(global_fout,"%s;%d\n",pi->key, pi->counter);
	}
	pa->dict_next = freelist->first;
	freelist->first = pl->first;
	pl->first = NULL;
}


DictInfo *dict_create() {
    DictInfo *d = (DictInfo*)calloc(1, sizeof(DictInfo));
    return d;
}

void dict_insert(DictInfo *d, char *prev_key, List_value *pvalue, int time) {
    if (prev_key[0] == '\0') return;
    uint32_t keyHash = jenkins_one_at_a_time_hash(prev_key, strlen(prev_key));
    void *pvalue0;
    Info *pInfoLocate = dict_locate(d, prev_key, &pvalue0);

    if (!pInfoLocate) {
        pInfoLocate = dict_info_alloc();
        pInfoLocate->counter = 0;
        pInfoLocate->hash = keyHash;
        pInfoLocate->pvalue = pvalue;
        pInfoLocate->time_counter = time;
        strcpy(pInfoLocate->key, prev_key);

        int index = (int)(keyHash % DICT_SIZE);
        append(&d->content[index], pInfoLocate);
    }
    //pInfoLocate->counter++;
}



Info *dict_locate(DictInfo *d, char *pkey, void **ref_pvalue) {
    uint32_t keyHash = jenkins_one_at_a_time_hash(pkey, strlen(pkey));
    Info *info_locate = search(&(d->content[(int)(keyHash % DICT_SIZE)]), pkey, keyHash);
    if (info_locate && ref_pvalue) *ref_pvalue = info_locate->pvalue;
    return info_locate;
}

void dict_print(DictInfo *d) {
    for(int i = 0; i < DICT_SIZE; i++) {
        printf("Dict hash %d: ", i);
        printList(d->content+i); //v[i] == *(v+1)
        printf("\n");
    }
}

/*
void freeDict(DictInfo *d){
    for(int i = 0; i < DICT_SIZE; i++){
        freeList(&d->content[i]);
    }
    free(d);
}
*/
