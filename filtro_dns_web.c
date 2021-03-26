#include "dict.h"
#include <sys/stat.h> // mkdir
int mkdir(const char *pathname, mode_t mode); // mkdir header

#define MAX_LINE_SZ 2001
#define BLANKS_PER_INDENT 4

typedef struct {
	char data[15];
	char hora[5];
	char min[5];
	char sec[5];
	char ttl[6];
	char proto[6];
	//int ip_src_arr[4];
	//int ip_dst_arr[4];
	char ip_src[32];
	char ip_dst[32];
	char port_src[8];
	char port_dst[8];
	char host[100];
	char query[100];
} DataHandler;


int get_number(char *dst, char *src, int len, int end) {
    for (; len; dst ++, src++, len --) {
        if (!(*src >= '0' && *src <= '9')) return 0;
        *dst = *src;
    }
    if (end) *dst = '\0';
    return 1;
}

int get_number_var(char *dst, char *src, int max_len) {
    int len;
    for (len = 0; len < max_len; dst ++, src++, len++) {
        if (!(*src >= '0' && *src <= '9')) break;
        *dst = *src;
    }
    if (!len) return 0;
    *dst = '\0';
    return 1;
}

int header_line(DataHandler *dados, char *start) {
	char *_;
	char *p;

    p =  strtok(start, " ");
	if (!p) return 0;
    if (strlen(p) != 10) return 0;

    if (!get_number(dados->data+0, p+0, 4, 0)) return 0;
    if (!get_number(dados->data+4, p+5, 2, 0)) return 0;
    if (!get_number(dados->data+6, p+8, 2, 1)) return 0;


	p = strtok(NULL, " ");
	if (!p) return 0;
    if (strlen(p) < 6) return 0;

    if (!get_number(dados->hora, p+0, 2, 1)) return 0;
    if (!get_number(dados->min , p+3, 2, 1)) return 0;


	char *headerType = strtok(NULL, " ");
	if ( !headerType || strcmp(headerType, "IP") != 0) return 0;

	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");

	p = strtok(NULL, " "); // ttl
	if (!p) return 0;
	//if (strlen(p) >= 6) fatal("Erro 4 %s\n", p);

    if (!get_number_var(dados->ttl, p, 5)) return 0;


	_ = strtok(NULL, " ");

	p = strtok(NULL, " "); // ip_id
	if (!p) return 0;
	if (strlen(p) >= 11) return 0;

    //if (!get_number_var(dados->val_ip_id, p, 5)) return 0;

	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");
	_ = strtok(NULL, " ");

	p = strtok(NULL, " "); // protocolo
	if (!p) return 0;
	p++; // remove "("
	if (strlen(p) >= 6) return 0;
	p[strlen(p)-2] = '\0'; // remove "),"
	strcpy(dados->proto, p); // armazena protocolo

	return 1;
}


int get_ips_port_dns_query(char *start, DataHandler *data) {
	char *p = strtok(start, " ");

	if (!(p[0] >= '0' && p[0] <= '9')) return 0;
	
	char *aux;
	int count_ip;
	int count;

	if (strlen(p) >= 32) return 0;

	for(aux = p, count_ip = 0, count = 0; count < 4; aux++, count_ip++) {
		if(*aux == '.') {
			count++;
		}
	}
	strcpy(data->ip_src, p);
	data->ip_src[count_ip-1] = '\0';
	strcpy(data->port_src, aux);
	
	p = strtok(NULL, " "); // >
	p = strtok(NULL, " ");
	if (strlen(p) >= 32) return 0;

	for(aux = p, count_ip = 0, count = 0; count < 4; aux++, count_ip++) {
		if(*aux == '.') {
			count++;
		}
	}
	strcpy(data->ip_dst, p);
	data->ip_dst[count_ip-1] = '\0';
	strcpy(data->port_dst, aux);

	data->port_dst[strlen(data->port_dst)-1] = '\0'; // remove ":"

	if(strcmp(data->port_dst, "53") == 0) { // DNS
		p = strtok(NULL, " ");
		p = strtok(NULL, " ");
		p = strtok(NULL, " ");
		p = strtok(NULL, " ");
		p = strtok(NULL, " ");
		if(p[strlen(p)-1] == '?') { // line without [1au]
			p = strtok(NULL, " ");
		}
		else {
			p = strtok(NULL, " ");
			if(p[strlen(p)-1] == '?') p = strtok(NULL, " ");
			else {
				strcpy(data->query, "0"); // lixo
				return 0;
			}
		}
		if(p && strlen(p) < 100) strcpy(data->query, p);
		else strcpy(data->query, "0"); // lixo
	}

	return 1;
}

int http_host_line(char *start, DataHandler *data ){
	start[4] = '\0';
	char *aux = start;
	if (strcmp(aux, "Host") == 0) {
		char *host = start + 6;

    strcpy(data->host, host);
	}
}

void list_flush(List *pl, FILE *global_fout) {
	Info *pi, *pa;
	pi = pl->first;
	if (!pi) return;
	for (pa = NULL; pi; pa = pi, pi = pi->dict_next) { 
		if(pi->counter > 0) fprintf(global_fout,"%s;%d\n", pi->key, pi->counter);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 1) {
		printf("Missing argument: filename\n");
		exit(1);
	}

	char line[MAX_LINE_SZ+1];
	char prev_key[MAX_KEY+1] = "";
	DictInfo *d = dict_create();

	DataHandler *dados = (DataHandler*)malloc(sizeof(DataHandler));

	mkdir("filtered_data", 0777); // cria diretorio
	
	char filename[60];
	sprintf(filename, "filtered_data/%s", argv[1]);

	FILE *pfin = fopen(argv[1], "r");
	
	while (fgets(line, MAX_LINE_SZ, pfin)) {
		char *start = line;

		int inner = 0;
		while (*start == ' ' || *start == '\t') {
			if (*start == '\t') { inner += 8; }
			else { inner++; }
			start++;
		}

		if (!start[0] || start[0]=='\n') { continue; }
		inner /= BLANKS_PER_INDENT;

		// primeiro nao branco
		if (inner == 0) {
			if (!header_line(dados, start)) continue;

		}

		if (inner == 1) {
			if (get_ips_port_dns_query(start, dados)) {
				if (strcmp(dados->proto , "17") == 0 ) {
				sprintf( prev_key,"%s;%s;%s", dados->ip_src, dados->ttl,dados->query);
				dict_insert(d, prev_key, NULL);
				}
			}
			else {
				prev_key[0] = '\0';
			}
		}
		// procurando User-Agent no corpo de requisicoes HTTP
		else {
			http_host_line(start, dados);
      if (strcmp(dados->proto , "6") == 0 ) {
        sprintf(prev_key,"%s;%s;%s", dados->ip_src, dados->ttl,dados->host);
        void *pvalue;
        Info *pi =dict_locate(d, prev_key, &pvalue);
        if(pi) pi->counter++;
      }
		}
	}

	fclose(pfin);

	FILE *pfout = fopen(filename, "wt");

	List *pl = d->content;
	for (int i = 0; i < DICT_SIZE; i++, pl++) {
		list_flush(pl, pfout);
	}
	fclose(pfout);
	
	free(dados);
	return 0;
}
