#ifndef REGISTRY_H
#define REGISTRY_H
#include <stddef.h>
#define MAX_COMMANDS 50
#define MAX_CMD_LEN 100
#define REGISTRY_FILE "commands.txt"
#define AUDIT_FILE "audit.log"
#define TEMP_FILE "commands.tmp"

typedef struct Node
{
    char command[MAX_CMD_LEN];
    struct Node *left;
    struct Node *right;
} Node;

typedef struct
{
    Node *root;
    size_t count;
} CommandRegistry;

void init_registry(CommandRegistry *registry);
int insert_command(CommandRegistry *registry, const char *command);
int search_command(const CommandRegistry *registry, const char *command);

void print_commands(const CommandRegistry *registry);
void clear_registry(CommandRegistry *registry);

int load_registry_from_file(CommandRegistry *registry, const char *filename);
int save_registry_session(const CommandRegistry *registry);

size_t get_levenshtein_distance(const char *first, const char *second);
void suggest_command(const CommandRegistry *registry, const char *command);

int log_audit(const char *command);

int read_line_secure(char *buffer, size_t size);
void trim_whitespace(char *string);

#endif

