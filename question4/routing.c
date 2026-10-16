#include "routing.h"
#include <stdio.h>
#include <string.h>

const char *LOCATION_NAMES[MAX_LOCATIONS] = {
    "Central Warehouse",
    "Distribution Center",
    "District Hospital",
    "Health Center A",
    "Health Center B",
    "Health Center C"
};

const int ROAD_GRAPH[MAX_LOCATIONS][MAX_LOCATIONS] = {
    {0,   8,   14,  INF, INF, INF},
    {8,   0,   INF, 5,   11,  INF},
    {14,  INF, 0,   7,   9,   6},
    {INF, 5,   7,   0,   INF, 4},
    {INF, 11,  9,   INF, 0,   3},
    {INF, INF, 6,   4,   3,   0}
};

InputStatus read_line(const char *prompt, char *buffer, size_t buffer_size) {
    size_t length;
    int ch;

    if (buffer == NULL || buffer_size < 2) {return INPUT_ERROR;}
    if (prompt != NULL) {
        printf("%s", prompt);
        fflush(stdout);
    }

    if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
        return ferror(stdin) ? INPUT_ERROR : INPUT_EOF;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[--length] = '\0';
        if (length > 0 && buffer[length - 1] == '\r') {
            buffer[--length] = '\0';
        }
        return length == 0 ? INPUT_EMPTY : INPUT_SUCCESS;
    }

    ch = getchar();
    if (ch == '\n') {
        if (length > 0 && buffer[length - 1] == '\r') {
            buffer[--length] = '\0';
        }
        return length == 0 ? INPUT_EMPTY : INPUT_SUCCESS;
    }

    if (ch == EOF) {
        if (ferror(stdin)) {
            return INPUT_ERROR;
        }
        return length == 0 ? INPUT_EMPTY : INPUT_SUCCESS;
    }

    while (ch != '\n' && ch != EOF) {ch = getchar();}
    if (ferror(stdin)) {return INPUT_ERROR;}

    return INPUT_OVERLONG;
}

void display_locations(void) {
    int i;
    printf("\nAvailable locations:\n");
    for (i = 0; i < MAX_LOCATIONS; i++) {
        printf("%d. %s\n", i + 1, LOCATION_NAMES[i]);
    }
    printf("\n");
}

int find_location(const char *name) {
    int i;
    if (name == NULL) {return -1;}
    for (i = 0; i < MAX_LOCATIONS; i++) {
        if (strcmp(name, LOCATION_NAMES[i]) == 0) {return i;}
    }
    return -1;
}

int find_shortest_route(
    const int graph[MAX_LOCATIONS][MAX_LOCATIONS],
    int start_idx,
    int dest_idx,
    RouteResult *result
) {
    int distance[MAX_LOCATIONS];
    int previous[MAX_LOCATIONS];
    int visited[MAX_LOCATIONS] = {0};
    int reverse_path[MAX_LOCATIONS];
    int path_count = 0;
    int i;
    int j;

    if (result == NULL) {return 0;}

    result->is_valid = 0;
    result->path_count = 0;
    result->total_distance = 0;

    if (graph == NULL ||
        start_idx < 0 || start_idx >= MAX_LOCATIONS ||
        dest_idx < 0 || dest_idx >= MAX_LOCATIONS) {return 0;}

    for (i = 0; i < MAX_LOCATIONS; i++) {
        for (j = 0; j < MAX_LOCATIONS; j++) {
            if (graph[i][j] < 0 || graph[i][j] > INF) {return 0;}
            if (i == j && graph[i][j] != 0) {return 0;}
            if (graph[i][j] != graph[j][i]) {return 0;}
        }
    }

    for (i = 0; i < MAX_LOCATIONS; i++) {
        distance[i] = INF;
        previous[i] = -1;
    }

    distance[start_idx] = 0;

    for (i = 0; i < MAX_LOCATIONS; i++) {
        int u = -1;
        int smallest_distance = INF;
        for (j = 0; j < MAX_LOCATIONS; j++) {
            if (!visited[j] && distance[j] < smallest_distance) {
                smallest_distance = distance[j];
                u = j;
            }
        }
        if (u == -1) {break;}
        if (u == dest_idx) {break;}
	visited[u] = 1;
        for (j = 0; j < MAX_LOCATIONS; j++) {
            int candidate_distance;
            if (visited[j] || graph[u][j] == INF) {continue;}
            if (distance[u] > INF - graph[u][j]) {continue;}
            candidate_distance = distance[u] + graph[u][j];
            if (candidate_distance < distance[j]) {
                distance[j] = candidate_distance;
                previous[j] = u;
            }
        }
    }

    if (distance[dest_idx] == INF) {return 0;}
    {
        int current = dest_idx;
        while (current != -1 && path_count < MAX_LOCATIONS) {
            reverse_path[path_count++] = current;
            if (current == start_idx) {break;}
            current = previous[current];
        }
    }

    if (path_count == 0 ||
        reverse_path[path_count - 1] != start_idx) {return 0;}

    for (i = 0; i < path_count; i++) {
        result->path[i] = reverse_path[path_count - 1 - i];
    }

    result->path_count = path_count;
    result->total_distance = distance[dest_idx];
    result->start_idx = start_idx;
    result->dest_idx = dest_idx;
    result->is_valid = 1;

    return 1;
}

void display_route(const RouteResult *result) {
    int i;

    if (result == NULL || !result->is_valid ||
        result->path_count <= 0) {
        printf("No valid route is available.\n");
        return;
    }

    printf("\nShortest route:\n");

    for (i = 0; i < result->path_count; i++) {
        printf("%s", LOCATION_NAMES[result->path[i]]);
        if (i < result->path_count - 1) {printf(" -> ");}
    }

    printf("\nTotal distance: %d km\n", result->total_distance);
}

