#include <stdio.h>
#include <string.h>
#include "net_analyser.h"

int main(void)
{
    Graph g;
    char command_buffer[MAX_NAME];
    init_graph(&g);
    printf("\nServer Dependency Network Analyzer\n\n");

    if (!seed_initial_data(&g)) {
        printf("Fatal error: Unable to initialize network graph.\n");
        free_graph(&g);
        return 1;
    }

    while (1) {
        int status;

        printf("\nCommands:\n");
        printf("  a - Add server\n");
        printf("  r - Remove server\n");
        printf("  c - Create dependency\n");
        printf("  d - Delete dependency\n");
        printf("  q - Query dependencies\n");
        printf("  m - Display adjacency matrix\n");
        printf("  x - Exit\n");

        status = read_line_secure(
            command_buffer,
            sizeof(command_buffer),
            "Enter command: "
        );

        if (status < 0) {
            printf("\nEnd of input detected. Exiting safely.\n");
            break;
        }

        if (status == 0) {continue;}

        if (strlen(command_buffer) != 1) {
            printf("Error: Enter exactly one command character.\n");
            continue;
        }

        switch (command_buffer[0]) {
            case 'a': {
                char name[MAX_NAME];
                status = read_line_secure(
                    name, sizeof(name), "Enter server name to add: "
                );
                if (status < 0) {goto shutdown;}
                if (status == 1) {add_server(&g, name);}
                break;
            }

            case 'r': {
                char name[MAX_NAME];
                status = read_line_secure(
                    name, sizeof(name), "Enter server name to remove: "
                );
                if (status < 0) {goto shutdown;}
                if (status == 1) {remove_server(&g, name);}
                break;
            }

            case 'c': {
                char source[MAX_NAME];
                char destination[MAX_NAME];
                status = read_line_secure(
                    source, sizeof(source), "Enter source server: "
                );
                if (status < 0) {goto shutdown;}
                if (status == 0) {break;}
                status = read_line_secure(
                    destination, sizeof(destination),
                    "Enter dependent server: "
                );
                if (status < 0) {goto shutdown;}
                if (status == 1) {
                    create_dependency(&g, source, destination);
                }
                break;
            }

            case 'd': {
                char source[MAX_NAME];
                char destination[MAX_NAME];
                status = read_line_secure(
                    source, sizeof(source), "Enter source server: "
                );
                if (status < 0) {goto shutdown;}
                if (status == 0) {break;}
                status = read_line_secure(
                    destination, sizeof(destination),
                    "Enter dependent server: "
                );
                if (status < 0) {goto shutdown;}
                if (status == 1) {
                    delete_dependency(&g, source, destination);
                }
                break;
            }

            case 'q': {
                char name[MAX_NAME];
                status = read_line_secure(
                    name, sizeof(name), "Enter server name to query: "
                );
                if (status < 0) {goto shutdown;}
                if (status == 1) {query_dependencies(&g, name);}
                break;
            }

            case 'm':
                display_matrix(&g);
                break;

            case 'x':
                goto shutdown;

            default:
                printf("Error: Unknown command. Please try again.\n");
                break;
        }
    }

shutdown:
    free_graph(&g);
    printf("\nNetwork graph memory releasing...\nProgram terminated.\n");

    return 0;
}

