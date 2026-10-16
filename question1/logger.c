#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "logger.h"

static int is_string_whitespace_only(const char *str);
static void sanitize_field_delimiters(char *str);
static int strings_equal_ignore_case(const char *a, const char *b);
static int event_id_exists(const EventLogger *logger, const char *id);
static int validate_timestamp_format(const char *timestamp);
static int validate_severity_level(char *severity);
static void remove_oldest_event(EventLogger *logger);

void init_logger(EventLogger *logger)
{
    if (logger == NULL) return;
    logger->head = NULL;
    logger->tail = NULL;
    logger->current = NULL;
    logger->count = 0;
}

void trim_newline(char *str)
{
    if (str != NULL) str[strcspn(str, "\r\n")] = '\0';
}

static int is_string_whitespace_only(const char *str)
{
    while (*str != '\0') {
        if (!isspace((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
}

static void sanitize_field_delimiters(char *str)
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '|') str[i] = ' ';
    }
}

int read_line_secure(char *dest, size_t size, const char *prompt)
{
    if (dest == NULL || size < 2) return 0;
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(dest, (int)size, stdin) == NULL) return -1;
    if (strchr(dest, '\n') == NULL && !feof(stdin)) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        dest[0] = '\0';
        printf("Input so long. Maximum length is %zu characters.\n",
               size - 2);
        return 0;
    }
    trim_newline(dest);
    if (dest[0] == '\0' || is_string_whitespace_only(dest)) {
        printf("Input can't be empty or whitespace only.\n");
        return 0;
    }
    sanitize_field_delimiters(dest);
    return 1;
}

static int strings_equal_ignore_case(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) !=
            tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static int event_id_exists(const EventLogger *logger, const char *id)
{
    const EventNode *node = logger->head;
    while (node != NULL) {
        if (strings_equal_ignore_case(node->event_id, id))
            return 1;
        node = node->next;
    }
    return 0;
}

static int validate_timestamp_format(const char *timestamp)
{
    if (strlen(timestamp) != 16) return 0;
    for (int i = 0; i < 16; i++) {
        if (i == 4 || i == 7) {
            if (timestamp[i] != '-') return 0;
        } else if (i == 10) {
            if (timestamp[i] != ' ') return 0;
        } else if (i == 13) {
            if (timestamp[i] != ':') return 0;
        } else if (!isdigit((unsigned char)timestamp[i])) {
            return 0;
        }
    }
    return 1;
}

static int validate_severity_level(char *severity)
{
    for (size_t i = 0; severity[i] != '\0'; i++)
        severity[i] = (char)tolower((unsigned char)severity[i]);
    if (strcmp(severity, "low") == 0) {
        strcpy(severity, "Low");
        return 1;
    }
    if (strcmp(severity, "medium") == 0) {
        strcpy(severity, "Medium");
        return 1;
    }
    if (strcmp(severity, "high") == 0) {
        strcpy(severity, "High");
        return 1;
    }
    return 0;
}

static void remove_oldest_event(EventLogger *logger)
{
    if (logger->head == NULL) return;
    EventNode *old_head = logger->head;
    if (logger->current == old_head)
        logger->current = old_head->next;
    logger->head = old_head->next;
    if (logger->head != NULL) {
        logger->head->prev = NULL;
    } else {
        logger->tail = NULL;
        logger->current = NULL;
    }
    free(old_head);
    logger->count--;
}

void clear_all_events(EventLogger *logger)
{
    if (logger == NULL) return;
    EventNode *node = logger->head;
    while (node != NULL) {
        EventNode *next_node = node->next;
        free(node);
        node = next_node;
    }
    logger->head = NULL;
    logger->tail = NULL;
    logger->current = NULL;
    logger->count = 0;
}

void insert_event(EventLogger *logger)
{
    if (logger == NULL) return;
    EventNode *new_node = calloc(1, sizeof(*new_node));
    if (new_node == NULL) {
        printf("Unable to allocate memory for new event.\n");
        return;
    }

    int result;
    while (1) {
        result = read_line_secure(
            new_node->event_id,
            sizeof(new_node->event_id),
            "Enter Event ID: ");
        if (result < 0) goto input_ended;
        if (result == 0) continue;
        if (event_id_exists(logger, new_node->event_id)) {
            printf("Event ID already exists. Enter unique ID.\n");
            continue;
        }
        break;
    }

    while (1) {
        result = read_line_secure(
            new_node->event_type,
            sizeof(new_node->event_type),
            "Enter Event Type: ");
        if (result < 0) goto input_ended;
        if (result > 0) break;
        printf("Please enter valid event type.\n");
    }

    while (1) {
        result = read_line_secure(
            new_node->device_id,
            sizeof(new_node->device_id),
            "Enter Device/Sensor ID: ");
        if (result < 0) goto input_ended;
        if (result > 0) break;
        printf("Please enter valid device/sensor ID.\n");
    }

    while (1) {
        result = read_line_secure(
            new_node->timestamp,
            sizeof(new_node->timestamp),
            "Enter Timestamp (YYYY-MM-DD HH:MM): ");
        if (result < 0) goto input_ended;
        if (result == 0) continue;
        if (validate_timestamp_format(new_node->timestamp))
            break;
        printf("Invalid timestamp format. "
               "Use YYYY-MM-DD HH:MM, e.g. 2026-10-16 12:33.\n");
    }

    while (1) {
        result = read_line_secure(
            new_node->severity,
            sizeof(new_node->severity),
            "Enter Severity (Low/Medium/High): ");
        if (result < 0) goto input_ended;
        if (result == 0) continue;
        if (validate_severity_level(new_node->severity))
            break;
        printf("Invalid severity. Enter Low, Medium, or High.\n");
    }

    if (logger->count >= MAX_EVENTS) {
        printf("Capacity reached. Removing oldest event...\n");
        remove_oldest_event(logger);
    }

    if (logger->head == NULL) {
        logger->head = new_node;
        logger->tail = new_node;
    } else {
        new_node->prev = logger->tail;
        logger->tail->next = new_node;
        logger->tail = new_node;
    }

    /* make newly entered event the current event */
    logger->current = new_node;

    logger->count++;
    printf("Event %s successfully logged. (Entries: %d/%d)\n",
           new_node->event_id, logger->count, MAX_EVENTS);
    return;
input_ended:
    printf("\nInput ended. Incomplete event was discarded.\n");
    free(new_node);
}

void display_event(const EventNode *node)
{
    if (node == NULL) {
        printf("No event selected / List is empty.\n");
        return;
    }
    printf("\nCurrent Event Details\n");
    printf("Event ID     : %s\n", node->event_id);
    printf("Type         : %s\n", node->event_type);
    printf("Device ID    : %s\n", node->device_id);
    printf("Timestamp    : %s\n", node->timestamp);
    printf("Severity     : %s\n", node->severity);
    printf("\n");
}

void navigate_next(EventLogger *logger)
{
    if (logger == NULL || logger->current == NULL) {
        printf("Log is empty. Nothing to navigate.\n");
    } else if (logger->current->next == NULL) {
        printf("Already at newest event.\n");
    } else {
        logger->current = logger->current->next;
        display_event(logger->current);
    }
}

void navigate_prev(EventLogger *logger)
{
    if (logger == NULL || logger->current == NULL) {
        printf("Log is empty. Nothing to navigate.\n");
    } else if (logger->current->prev == NULL) {
        printf("Already at oldest event.\n");
    } else {
        logger->current = logger->current->prev;
        display_event(logger->current);
    }
}

int save_session(const EventLogger *logger)
{
    FILE *file = fopen(TEMP_FILE, "w");
    if (file == NULL) {
        perror("Unable to open temporary save file");
        return 0;
    }

    const EventNode *node = logger->head;
    while (node != NULL) {
        if (fprintf(file, "%s|%s|%s|%s|%s\n",
                    node->event_id,
                    node->event_type,
                    node->device_id,
                    node->timestamp,
                    node->severity) < 0) {
            perror("Failed to write event data");
            fclose(file);
            remove(TEMP_FILE);
            return 0;
        }
        node = node->next;
    }

    if (fclose(file) == EOF) {
        perror("Failed to close save file");
        remove(TEMP_FILE);
        return 0;
    }

    if (rename(TEMP_FILE, SAVE_FILE) != 0) {
        perror("Failed to replace saved event");
        remove(TEMP_FILE);
        return 0;
    }

    printf("Session saved to %s.\n", SAVE_FILE);
    return 1;
}

