#ifndef ROUTING_H
#define ROUTING_H
#include <stddef.h>
#define MAX_LOCATIONS 6
#define MAX_NAME_LEN 50
#define INF 999999

typedef struct {
    int path[MAX_LOCATIONS];
    int path_count;
    int total_distance;
    int start_idx;
    int dest_idx;
    int is_valid;
} RouteResult;

typedef enum {
    INPUT_SUCCESS,
    INPUT_EMPTY,
    INPUT_OVERLONG,
    INPUT_EOF,
    INPUT_ERROR
} InputStatus;

extern const char *LOCATION_NAMES[MAX_LOCATIONS];

extern const int ROAD_GRAPH[MAX_LOCATIONS][MAX_LOCATIONS];

InputStatus read_line(
    const char *prompt,
    char *buffer,
    size_t buffer_size
);

void display_locations(void);

int find_location(const char *name);

int find_shortest_route(
    const int graph[MAX_LOCATIONS][MAX_LOCATIONS],
    int start_idx,
    int dest_idx,
    RouteResult *result
);

void display_route(const RouteResult *result);

#endif

