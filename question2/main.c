#include "registry.h"
#include <stdio.h>
#include <stdlib.h>
#define INPUT_SIZE 256

static void verify_command(const CommandRegistry *registry)
{
    char command[INPUT_SIZE];
    int input_status;
    printf("Enter command: ");
    fflush(stdout);
    input_status = read_line_secure(command, sizeof(command));

    if (input_status == 0)
    {
        printf("\nEnd of input.\n");
        return;
    }

    if (input_status == -1)
    {
        printf("Invalid input: command is too long or could not be read.\n");
        return;
    }

    trim_whitespace(command);

    if (command[0] == '\0')
    {
        printf("Please enter a non-empty command.\n");
        return;
    }

    if (search_command(registry, command))
    {
        printf("COMMAND APPROVED\n");
        return;
    }

    printf("UNKNOWN COMMAND\n");

    if (log_audit(command) != 0)
        fprintf(stderr, "Warning: unknown command was not audited.\n");

    suggest_command(registry, command);
}

static int save_and_exit(const CommandRegistry *registry)
{
    if (save_registry_session(registry) != 0)
    {
        fprintf(stderr, "Session was not saved. Retry entry...\n");
        return -1;
    }
    printf("Registry saved. Goodbye.\n");
    return 0;
}

static void display_menu(void)
{
    printf("\nNetwork Device Command Registry\n");
    printf("v - Verify a command\n");
    printf("p - Print approved commands\n");
    printf("s - Show command count\n");
    printf("x - Clear the registry\n");
    printf("q - Save session and quit\n");
    printf("Select an option: ");
    fflush(stdout);
}

int main(void)
{
    CommandRegistry registry;
    char choice[INPUT_SIZE];
    int input_status;
    int running = 1;

    init_registry(&registry);

    if (load_registry_from_file(&registry, REGISTRY_FILE) != 0)
    {
        fprintf(stderr,
                "Fatal: unable to load command registry from %s.\n",
                REGISTRY_FILE);
        clear_registry(&registry);
        return EXIT_FAILURE;
    }

    while (running)
    {
        display_menu();
        input_status = read_line_secure(choice, sizeof(choice));

        if (input_status == 0)
        {
            printf("\nEnd of input. Saving session...\n");
            if (save_and_exit(&registry) != 0)
            {
                clear_registry(&registry);
                return EXIT_FAILURE;
            }
            running = 0;
            continue;
        }

        if (input_status == -1)
        {
            printf("Invalid input. Please enter menu option.\n");
            continue;
        }

        trim_whitespace(choice);

        if (choice[0] == '\0' || choice[1] != '\0')
        {
            printf("Invalid option. Enter v, p, s, x, or q.\n");
            continue;
        }

        switch (choice[0])
        {
        case 'v':
            verify_command(&registry);
            break;
        case 'p':
            print_commands(&registry);
            break;
        case 's':
            printf("Registered commands: %zu/%d\n",
                   registry.count, MAX_COMMANDS);
            break;
        case 'x':
            clear_registry(&registry);
            printf("Registry cleared. The change will be saved when you quit with q.\n");
            break;
        case 'q':
            if (save_and_exit(&registry) == 0)
                running = 0;
            break;
        default:
            printf("Invalid option. Enter v, p, s, x, or q.\n");
            break;
        }
    }

    clear_registry(&registry);
    return EXIT_SUCCESS;
}

