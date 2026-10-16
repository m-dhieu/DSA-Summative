#ifndef LOGGER_H
#define LOGGER_H
#include <stddef.h>
#define MAX_EVENTS 25
#define BUFFER_SIZE 64
#define SAVE_FILE "events.log"
#define TEMP_FILE "events.tmp"

typedef struct EventNode {
    char event_id[BUFFER_SIZE];
    char event_type[BUFFER_SIZE];
    char device_id[BUFFER_SIZE];
    char timestamp[BUFFER_SIZE];
    char severity[BUFFER_SIZE];
    struct EventNode *prev;
    struct EventNode *next;
} EventNode;

typedef struct {
    EventNode *head;
    EventNode *tail;
    EventNode *current;
    int count;
} EventLogger;

void init_logger(EventLogger *logger);
void clear_all_events(EventLogger *logger);
void insert_event(EventLogger *logger);
void display_event(const EventNode *node);
void navigate_next(EventLogger *logger);
void navigate_prev(EventLogger *logger);
int save_session(const EventLogger *logger);
int read_line_secure(char *dest, size_t size, const char *prompt);
void trim_newline(char *str);

#endif

