#include <stdio.h>
#include <string.h>
#include "logger.h"

static int read_command(char *command, size_t size)
{
    if (fgets(command, (int)size, stdin) == NULL)
        return 0;
    if (strchr(command, '\n') == NULL) {int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        printf("Invalid! Enter one command at a time.\n");
        return -1;
    }
    trim_newline(command);
    if (strlen(command) != 1) {
        printf("Invalid! Enter one command at a time.\n");
        return -1;
    }
    return 1;
}

int main(void)
{
    EventLogger logger;
    char command[16];
    init_logger(&logger);
    printf("\nEmbedded Device Event Logger\n\n");
    while (1) {
        printf("\nCommands:\n");
        printf("  a - add event\n");
        printf("  n - next/newer event\n");
        printf("  p - previous/older event\n");
        printf("  c - display current event\n");
        printf("  x - clear all events\n");
        printf("  q - save session and quit\n");
        printf("\nEnter Command: ");
        fflush(stdout);

        int result = read_command(command, sizeof(command));

        if (result == 0) {
            printf("\nInput ended. Attempting to save session...\n");
            if (!save_session(&logger))
                printf("Session could not be saved.\n");
            clear_all_events(&logger);
            break;
        }

        if (result < 0) continue;

        switch (command[0]) {
            case 'a':
            case 'A':
                insert_event(&logger);
                break;
            case 'n':
            case 'N':
                navigate_next(&logger);
                break;
            case 'p':
            case 'P':
                navigate_prev(&logger);
                break;
            case 'c':
            case 'C':
                display_event(logger.current);
                break;
            case 'x':
            case 'X':
                clear_all_events(&logger);
                printf("All events cleared.\n");
                break;
            case 'q':
            case 'Q':
                if (save_session(&logger)) {
                    clear_all_events(&logger);
                    printf("Exiting Event Logger...\n");
                    return 0;
                }
                printf("Save failed; "
                       "please retry or choose another command.\n");
                break;
            default:
                printf("Unknown command. Please choose a listed command.\n");
                break;
        }
    }
    return 0;
}

