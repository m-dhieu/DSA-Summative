#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "net_analyser.h"

static int normalize_name(const char *input, char output[MAX_NAME])
{
    const char *start;
    const char *end;
    size_t length;

    if (input == NULL) {return 0;}

    start = input;
    while (*start != '\0' && isspace((unsigned char)*start)) {start++;}

    end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) {end--;}

    length = (size_t)(end - start);
    if (length == 0) {return 0;}
    if (length >= MAX_NAME) {return -1;}

    memcpy(output, start, length);
    output[length] = '\0';

    return 1;
}

static int graph_is_valid(const Graph *g)
{
    int i;
    if (g == NULL || g->count < 0) {return 0;}
    if (g->count == 0) {
        return g->servers == NULL && g->matrix == NULL;
    }
    if (g->servers == NULL || g->matrix == NULL) {return 0;}
    for (i = 0; i < g->count; i++) {
        if (g->matrix[i] == NULL) {return 0;}
    }
    return 1;
}

static int find_server_idx(const Graph *g, const char *name)
{
    int i;
    if (!graph_is_valid(g) || name == NULL) {return -1;}
    for (i = 0; i < g->count; i++) {
        if (strcmp(g->servers[i].name, name) == 0) {return i;}
    }
    return -1;
}

void init_graph(Graph *g)
{
    if (g == NULL) {return;}
    g->servers = NULL;
    g->matrix = NULL;
    g->count = 0;
}

void free_graph(Graph *g)
{
    int i;
    if (g == NULL) {return;}
    if (g->matrix != NULL) {
        for (i = 0; i < g->count; i++) {free(g->matrix[i]);}
        free(g->matrix);
    }
    free(g->servers);
    g->servers = NULL;
    g->matrix = NULL;
    g->count = 0;
}

int add_server(Graph *g, const char *name)
{
    char normalized[MAX_NAME];
    int validation;
    int next_count;
    int i;
    int j;

    Server *new_servers;
    int **new_matrix;
    if (!graph_is_valid(g)) {
        printf("Error: Graph is not in a valid state.\n");
        return 0;
    }

    validation = normalize_name(name, normalized);
    if (validation == 0) {
        printf("Error: Server name can't be empty or whitespace-only.\n");
        return 0;
    }
    if (validation < 0) {
        printf("Error: Server name mustn't exceed %d characters.\n",
               MAX_NAME - 1);
        return 0;
    }

    if (find_server_idx(g, normalized) != -1) {
        printf("Error: Server '%s' already exists.\n", normalized);
        return 0;
    }

    if (g->count == INT_MAX) {
        printf("Error: Maximum server count reached.\n");
        return 0;
    }

    next_count = g->count + 1;
    if ((size_t)next_count > (size_t)-1 / sizeof(Server) ||
        (size_t)next_count > (size_t)-1 / sizeof(int *) ||
        (size_t)next_count > (size_t)-1 / sizeof(int)) {
        printf("Error: Requested graph size is too large.\n");
        return 0;
    }

    new_servers = malloc((size_t)next_count * sizeof(Server));
    if (new_servers == NULL) {
        printf("Error: Unable to allocate server storage.\n");
        return 0;
    }

    new_matrix = malloc((size_t)next_count * sizeof(int *));
    if (new_matrix == NULL) {
        printf("Error: Unable to allocate matrix storage.\n");
        free(new_servers);
        return 0;
    }

    for (i = 0; i < next_count; i++) {
        new_matrix[i] = malloc((size_t)next_count * sizeof(int));
        if (new_matrix[i] == NULL) {
            printf("Error: Unable to allocate adjacency matrix row.\n");
            for (j = 0; j < i; j++) {free(new_matrix[j]);}
            free(new_matrix);
            free(new_servers);
            return 0;
        }
        for (j = 0; j < next_count; j++) {new_matrix[i][j] = 0;}
    }

    for (i = 0; i < g->count; i++) {
        memcpy(new_servers[i].name,
               g->servers[i].name,
               sizeof(new_servers[i].name));
        for (j = 0; j < g->count; j++) {
            new_matrix[i][j] = g->matrix[i][j];
        }
    }

    memcpy(new_servers[g->count].name,
           normalized,
           strlen(normalized) + 1);

    if (g->matrix != NULL) {
        for (i = 0; i < g->count; i++) {free(g->matrix[i]);}
        free(g->matrix);
    }

    free(g->servers);

    g->servers = new_servers;
    g->matrix = new_matrix;
    g->count = next_count;

    printf("Server '%s' added successfully.\n", normalized);
    return 1;
}

int remove_server(Graph *g, const char *name)
{
    char normalized[MAX_NAME];
    int validation;
    int idx;
    int old_count;
    int i;
    int j;

    if (!graph_is_valid(g)) {
        printf("Error: Graph is not in valid state.\n");
        return 0;
    }

    if (g->count == 0) {
        printf("Error: Graph is empty. Can't remove server.\n");
        return 0;
    }

    validation = normalize_name(name, normalized);
    if (validation != 1) {
        printf("Error: Invalid server name.\n");
        return 0;
    }

    idx = find_server_idx(g, normalized);
    if (idx == -1) {
        printf("Error: Server '%s' doesn't exist.\n", normalized);
        return 0;
    }

    old_count = g->count;
    for (i = 0; i < old_count; i++) {
        for (j = idx; j < old_count - 1; j++) {
            g->matrix[i][j] = g->matrix[i][j + 1];
        }
    }

    free(g->matrix[idx]);

    for (i = idx; i < old_count - 1; i++) {
        g->matrix[i] = g->matrix[i + 1];
    }

    for (i = idx; i < old_count - 1; i++) {
        g->servers[i] = g->servers[i + 1];
    }

    g->count--;

    if (g->count == 0) {
        free(g->servers);
        free(g->matrix);
        g->servers = NULL;
        g->matrix = NULL;
    }

    printf("Server '%s' and all connected relationships removed.\n",
           normalized);
    return 1;
}

int create_dependency(Graph *g, const char *from, const char *to)
{
    char source[MAX_NAME];
    char destination[MAX_NAME];
    int u;
    int v;
    if (!graph_is_valid(g)) {
        printf("Error: Graph is not in valid state.\n");
        return 0;
    }
    if (normalize_name(from, source) != 1 ||
        normalize_name(to, destination) != 1) {
        printf("Error: Both dependency endpoints must be valid names.\n");
        return 0;
    }
    u = find_server_idx(g, source);
    v = find_server_idx(g, destination);
    if (u == -1 || v == -1) {
        printf("Error: One or both servers don't exist.\n");
        return 0;
    }
    if (g->matrix[u][v] == 1) {
        printf("Error: Dependency '%s -> %s' already exists.\n",
               source, destination);
        return 0;
    }
    g->matrix[u][v] = 1;
    printf("Dependency created: %s -> %s\n", source, destination);
    return 1;
}

int delete_dependency(Graph *g, const char *from, const char *to)
{
    char source[MAX_NAME];
    char destination[MAX_NAME];
    int u;
    int v;
    if (!graph_is_valid(g)) {
        printf("Error: Graph is not in valid state.\n");
        return 0;
    }
    if (normalize_name(from, source) != 1 ||
        normalize_name(to, destination) != 1) {
        printf("Error: Both dependency endpoints must be valid names.\n");
        return 0;
    }
    u = find_server_idx(g, source);
    v = find_server_idx(g, destination);
    if (u == -1 || v == -1) {
        printf("Error: One or both servers don't exist.\n");
        return 0;
    }
    if (g->matrix[u][v] == 0) {
        printf("Error: Dependency '%s -> %s' doesn't exist.\n",
               source, destination);
        return 0;
    }
    g->matrix[u][v] = 0;
    printf("Dependency deleted: %s -> %s\n", source, destination);
    return 1;
}

void query_dependencies(const Graph *g, const char *name)
{
    char normalized[MAX_NAME];
    int idx;
    int i;
    int found;

    if (!graph_is_valid(g)) {
        printf("Error: Graph is not in valid state.\n");
        return;
    }

    if (g->count == 0) {
        printf("Error: Cannot query an empty graph.\n");
        return;
    }

    if (normalize_name(name, normalized) != 1) {
        printf("Error: Invalid server name.\n");
        return;
    }

    idx = find_server_idx(g, normalized);
    if (idx == -1) {
        printf("Error: Server '%s' doesn't exist.\n", normalized);
        return;
    }

    printf("\nServer: %s\n", g->servers[idx].name);

    printf("Direct dependencies:\n");
    found = 0;

    for (i = 0; i < g->count; i++) {
        if (g->matrix[idx][i] == 1) {
            printf("  - %s\n", g->servers[i].name);
            found = 1;
        }
    }

    if (!found) {
        printf("  (None)\n");
    }

    printf("Services depending on %s:\n", g->servers[idx].name);
    found = 0;

    for (i = 0; i < g->count; i++) {
        if (g->matrix[i][idx] == 1) {
            printf("  - %s\n", g->servers[i].name);
            found = 1;
        }
    }

    if (!found) {
        printf("  (None)\n");
    }
}

void display_matrix(const Graph *g)
{
    int i;
    int j;
    if (!graph_is_valid(g)) {
        printf("Error: Graph is not in valid state.\n");
        return;
    }
    if (g->count == 0) {
        printf("Adjacency matrix is empty.\n");
        return;
    }
    printf("\nAdjacency Matrix (row -> column):\n");
    printf("%-30s", "Server");
    for (i = 0; i < g->count; i++) {printf("%-5d", i);}
    printf("\n");
    for (i = 0; i < g->count; i++) {
        printf("[%d] %-26s", i, g->servers[i].name);
        for (j = 0; j < g->count; j++) {
            printf("%-5d", g->matrix[i][j]);
        }
        printf("\n");
    }
    printf("\nA value of 1 indicates a directed dependency.\n");
}

int seed_initial_data(Graph *g)
{
    if (!graph_is_valid(g) || g->count != 0) {
        printf("Error: Initial graph must be empty and valid.\n");
        return 0;
    }

    if (!add_server(g, "Web Server") ||
        !add_server(g, "API Server") ||
        !add_server(g, "Authentication Server") ||
        !add_server(g, "Database Server") ||
        !add_server(g, "Cache Server") ||
        !add_server(g, "Backup Server")) {return 0;}

    if (!create_dependency(g, "Web Server", "API Server") ||
        !create_dependency(g, "Web Server", "Authentication Server") ||
        !create_dependency(g, "API Server", "Database Server") ||
        !create_dependency(g, "API Server", "Cache Server") ||
        !create_dependency(g, "Authentication Server", "Database Server") ||
        !create_dependency(g, "Cache Server", "Database Server") ||
        !create_dependency(g, "Database Server", "Backup Server")) {return 0;}

    return 1;
}

int read_line_secure(char *dest, size_t size, const char *prompt)
{
    size_t length;
    size_t start;
    int ch;

    if (dest == NULL || size < 2 || size > (size_t)INT_MAX) {
        printf("Error: Invalid input buffer.\n");
        return 0;
    }

    if (prompt != NULL) {printf("%s", prompt);}
    if (fgets(dest, (int)size, stdin) == NULL) {return -1;}

    length = strlen(dest);
    if (length == size - 1 && strchr(dest, '\n') == NULL) {
        ch = getchar();
        if (ch != '\n' && ch != EOF) {
            do {
                ch = getchar();
            } while (ch != '\n' && ch != EOF);
            dest[0] = '\0';
            printf("Error: Input exceeds maximum allowed length.\n");
            return 0;
        }
    }

    dest[strcspn(dest, "\r\n")] = '\0';
    start = 0;
    while (dest[start] != '\0' &&
           isspace((unsigned char)dest[start])) {start++;}
    if (start > 0) {
        memmove(dest, dest + start, strlen(dest + start) + 1);
    }

    length = strlen(dest);
    while (length > 0 &&
           isspace((unsigned char)dest[length - 1])) {
        dest[length - 1] = '\0';
        length--;
    }

    return 1;
}

