#include "registry.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

static int is_blank(const char *text)
{
    while (*text != '\0')
    {
        if (!isspace((unsigned char)*text)) return 0;
        text++;
    }
    return 1;
}

static Node *create_node(const char *command)
{
    Node *node = malloc(sizeof(*node));
    if (node == NULL) return NULL;
    (void)snprintf(node->command, sizeof(node->command), "%s", command);
    node->left = NULL;
    node->right = NULL;
    return node;
}

static Node *insert_node(Node *root, const char *command, int *status)
{
    int comparison;
    if (root == NULL)
    {
        Node *node = create_node(command);
        if (node == NULL)
        {
            *status = -1;
            return NULL;
        }
        *status = 1;
        return node;
    }
    comparison = strcmp(command, root->command);
    if (comparison == 0) {*status = 0;}
    else if (comparison < 0)
    {
        Node *child = insert_node(root->left, command, status);
        if (*status == 1) root->left = child;
    }
    else
    {
        Node *child = insert_node(root->right, command, status);
        if (*status == 1) root->right = child;
    }
    return root;
}

void init_registry(CommandRegistry *registry)
{
    if (registry != NULL)
    {
        registry->root = NULL;
        registry->count = 0;
    }
}

int insert_command(CommandRegistry *registry, const char *command)
{
    size_t length;
    int status = 0;
    Node *new_root;
    if (registry == NULL || command == NULL)
        return -2;
    length = strlen(command);
    if (length == 0 || length >= MAX_CMD_LEN || is_blank(command))
        return -2;
    if (registry->count >= MAX_COMMANDS)
    {
        if (search_command(registry, command)) return 0;
        return -3;
    }
    new_root = insert_node(registry->root, command, &status);
    if (status == 1)
    {
        registry->root = new_root;
        registry->count++;
    }
    return status;
}

int search_command(const CommandRegistry *registry, const char *command)
{
    const Node *current;
    if (registry == NULL || command == NULL) return 0;
    current = registry->root;
    while (current != NULL)
    {
        int comparison = strcmp(command, current->command);
        if (comparison == 0) return 1;
        current = (comparison < 0) ? current->left : current->right;
    }
    return 0;
}

static void print_in_order(const Node *node)
{
    if (node == NULL) return;
    print_in_order(node->left);
    printf("%s\n", node->command);
    print_in_order(node->right);
}

void print_commands(const CommandRegistry *registry)
{
    if (registry == NULL || registry->count == 0)
    {
        printf("The registry is empty.\n");
        return;
    }
    printf("Approved commands (%zu):\n", registry->count);
    print_in_order(registry->root);
}

static void free_nodes(Node *node)
{
    if (node == NULL) return;
    free_nodes(node->left);
    free_nodes(node->right);
    free(node);
}

void clear_registry(CommandRegistry *registry)
{
    if (registry == NULL) return;
    free_nodes(registry->root);
    registry->root = NULL;
    registry->count = 0;
}

void trim_whitespace(char *string)
{
    char *start;
    size_t length;
    if (string == NULL) return;
    start = string;
    while (*start != '\0' && isspace((unsigned char)*start))
        start++;
    if (start != string)
        memmove(string, start, strlen(start) + 1);
    length = strlen(string);
    while (length > 0 && isspace((unsigned char)string[length - 1]))
    {
        string[length - 1] = '\0';
        length--;
    }
}

int load_registry_from_file(CommandRegistry *registry, const char *filename)
{
    FILE *file;
    char line[MAX_CMD_LEN + 2];
    size_t line_number = 0;
    size_t loaded = 0;
    size_t skipped = 0;

    if (registry == NULL || filename == NULL) return -1;

    file = fopen(filename, "r");
    if (file == NULL)
    {
        perror(filename);
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        size_t length;
        int status;
        line_number++;
        if (strchr(line, '\n') == NULL && !feof(file))
        {
            int ch = fgetc(file);
            if (ch != '\n' && ch != EOF)
            {
                while (ch != '\n' && ch != EOF) ch = fgetc(file);
                fprintf(stderr,
                        "Warning: skipping overlong command at line %zu.\n",
                        line_number);
                skipped++;
                if (ferror(file))
                {
                    perror("Error reading command file");
                    (void)fclose(file);
                    return -1;
                }
                continue;
            }
        }

        trim_whitespace(line);

        if (line[0] == '\0') continue;

        length = strlen(line);
        if (length >= MAX_CMD_LEN)
        {
            fprintf(stderr,
                    "Warning: skipping command longer than %d characters at line %zu.\n",
                    MAX_CMD_LEN - 1, line_number);
            skipped++;
            continue;
        }

        status = insert_command(registry, line);

        if (status == 1) {loaded++;}
        else if (status == 0)
        {
            fprintf(stderr,
                    "Warning: skipping duplicate command at line %zu: %s\n",
                    line_number, line);
            skipped++;
        }
        else if (status == -3)
        {
            fprintf(stderr,
                    "Warning: registry limit (%d) reached; skipping line %zu.\n",
                    MAX_COMMANDS, line_number);
            skipped++;
        }
        else if (status == -2)
        {
            fprintf(stderr,
                    "Warning: skipping invalid command at line %zu.\n",
                    line_number);
            skipped++;
        }
        else
        {
            fprintf(stderr,
                    "Error: memory allocation failed while loading commands.\n");
            (void)fclose(file);
            return -1;
        }
    }

    if (ferror(file))
    {
        perror("Error reading command file");
        (void)fclose(file);
        return -1;
    }

    if (fclose(file) != 0)
    {
        perror("Error closing command file");
        return -1;
    }

    printf("Loaded %zu command(s) from %s; skipped %zu entr(y/ies).\n",
           loaded, filename, skipped);

    return 0;
}

static int write_in_order(FILE *file, const Node *node)
{
    if (node == NULL) return 0;
    if (write_in_order(file, node->left) != 0) return -1;
    if (fprintf(file, "%s\n", node->command) < 0) return -1;
    if (write_in_order(file, node->right) != 0) return -1;
    return 0;
}

int save_registry_session(const CommandRegistry *registry)
{
    FILE *file;
    int failed = 0;
    if (registry == NULL) return -1;
    file = fopen(TEMP_FILE, "w");
    if (file == NULL)
    {
        perror(TEMP_FILE);
        return -1;
    }
    if (write_in_order(file, registry->root) != 0) failed = 1;
    if (fclose(file) != 0) failed = 1;
    if (failed)
    {
        fprintf(stderr, "Error: failed to write registry session.\n");
        (void)remove(TEMP_FILE);
        return -1;
    }
    if (rename(TEMP_FILE, REGISTRY_FILE) != 0)
    {
        perror("Unable to replace commands.txt");
        (void)remove(TEMP_FILE);
        return -1;
    }
    return 0;
}

size_t get_levenshtein_distance(const char *first, const char *second)
{
    size_t first_length;
    size_t second_length;
    size_t *previous;
    size_t *current;
    size_t i;
    size_t j;
    size_t result;
    if (first == NULL || second == NULL) return SIZE_MAX;
    
    first_length = strlen(first);
    second_length = strlen(second);
    if (second_length >= SIZE_MAX / sizeof(size_t)) return SIZE_MAX;
    
    previous = malloc((second_length + 1) * sizeof(*previous));
    current = malloc((second_length + 1) * sizeof(*current));
    if (previous == NULL || current == NULL)
    {
        free(previous);
        free(current);
        return SIZE_MAX;
    }
    
    for (j = 0; j <= second_length; j++) previous[j] = j;
    for (i = 1; i <= first_length; i++)
    {
        current[0] = i;
        for (j = 1; j <= second_length; j++)
        {
            size_t deletion = previous[j] + 1;
            size_t insertion = current[j - 1] + 1;
            size_t substitution = previous[j - 1] +
                ((first[i - 1] == second[j - 1]) ? 0U : 1U);
            size_t minimum = deletion < insertion ? deletion : insertion;
            current[j] = minimum < substitution ? minimum : substitution;
        }
        {
            size_t *temporary = previous;
            previous = current;
            current = temporary;
        }
    }
    result = previous[second_length];
    
    free(previous);
    free(current);

    return result;
}

static void find_suggestion(const Node *node, const char *command,
                            const char **best_command, size_t *best_distance)
{
    size_t distance;
    if (node == NULL) return;
    distance = get_levenshtein_distance(command, node->command);
    if (distance < *best_distance)
    {
        *best_distance = distance;
        *best_command = node->command;
    }
    find_suggestion(node->left, command, best_command, best_distance);
    find_suggestion(node->right, command, best_command, best_distance);
}

void suggest_command(const CommandRegistry *registry, const char *command)
{
    const char *best_command = NULL;
    size_t best_distance = SIZE_MAX;

    if (registry == NULL || command == NULL || registry->root == NULL)
        return;
    find_suggestion(registry->root, command, &best_command, &best_distance);
    if (best_command != NULL && best_distance <= 2)
        printf("Did you mean: %s?\n", best_command);
}

int log_audit(const char *command)
{
    FILE *file;
    time_t now;
    struct tm *local_time;
    char timestamp[20];
    if (command == NULL) return -1;

    now = time(NULL);
    if (now == (time_t)-1) return -1;

    local_time = localtime(&now);
    if (local_time == NULL ||
        strftime(timestamp, sizeof(timestamp),
                 "%Y-%m-%d %H:%M:%S", local_time) == 0)
        return -1;

    file = fopen(AUDIT_FILE, "a");
    if (file == NULL) return -1;
    if (fprintf(file, "%s | UNKNOWN | %s\n", timestamp, command) < 0)
    {
        (void)fclose(file);
        return -1;
    }
    if (fclose(file) != 0) return -1;
    return 0;
}

int read_line_secure(char *buffer, size_t size)
{
    char *newline;
    if (buffer == NULL || size < 2 || size > (size_t)INT32_MAX) return -1;
    if (fgets(buffer, (int)size, stdin) == NULL)
        return ferror(stdin) ? -1 : 0;

    newline = strchr(buffer, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
        return 1;
    }

    if (!feof(stdin))
    {
        int ch = fgetc(stdin);
        if (ch != '\n' && ch != EOF)
        {
            while (ch != '\n' && ch != EOF) ch = fgetc(stdin);
            return -1;
        }
        if (ferror(stdin)) return -1;
    }

    return 1;
}

