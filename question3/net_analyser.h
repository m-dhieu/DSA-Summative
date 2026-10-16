#ifndef NET_ANALYSER_H
#define NET_ANALYSER_H
#include <stddef.h>
#define MAX_NAME 100

typedef struct {
    char name[MAX_NAME];
} Server;

typedef struct {
    Server *servers;
    int **matrix;
    int count;
} Graph;

void init_graph(Graph *g);
void free_graph(Graph *g);
int seed_initial_data(Graph *g);

int add_server(Graph *g, const char *name);
int remove_server(Graph *g, const char *name);

int create_dependency(Graph *g, const char *from, const char *to);
int delete_dependency(Graph *g, const char *from, const char *to);

void query_dependencies(const Graph *g, const char *name);
void display_matrix(const Graph *g);

/* input: 1 = success, 0 = invalid/overlong, -1 = EOF */
int read_line_secure(char *dest, size_t size, const char *prompt);

#endif
