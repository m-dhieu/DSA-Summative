#include "routing.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define INPUT_BUFFER_SIZE 100

static int read_menu_choice(int *choice) {
    char input[INPUT_BUFFER_SIZE];
    
    if (choice == NULL) {return -1;}

    for (;;) {
        char *end;
        long value;
        InputStatus status;
        status = read_line("\nEnter your choice (1-4): ",
                           input, sizeof(input));
        if (status == INPUT_EOF) {return 0;}
        if (status == INPUT_ERROR) {return -1;}
        if (status == INPUT_EMPTY || status == INPUT_OVERLONG) {
            printf("Invalid input. Please enter a number from 1 to 4.\n");
            continue;
        }
        errno = 0;
        value = strtol(input, &end, 10);
        while (isspace((unsigned char)*end)) {end++;}
        if (errno == ERANGE ||
            end == input ||
            *end != '\0' ||
            value < 1 ||
            value > 4) {
            printf("Invalid choice. Please enter a number from 1 to 4.\n");
            continue;
        }
        *choice = (int)value;
        return 1;
    }
}

static int read_location_index(const char *prompt, int *index) {
    char input[INPUT_BUFFER_SIZE];
    InputStatus status;
    int found;

    status = read_line(prompt, input, sizeof(input));

    if (status == INPUT_EOF) {return 0;}

    if (status == INPUT_ERROR) {return -1;}

    if (status == INPUT_EMPTY || status == INPUT_OVERLONG) {
        printf("Invalid or empty input. Please try again from the menu.\n");
        return 2;
    }

    found = find_location(input);
    if (found == -1) {
        printf("Unknown location: %s\n", input);
        display_locations();
        return 2;
    }

    *index = found;
    return 1;
}

static void display_menu(void) {
    printf("\nEMERGENCY MEDICAL SUPPLY ROUTING SYSTEM\n\n");
    printf("1. Display locations\n");
    printf("2. Find shortest route\n");
    printf("3. Display route distance\n");
    printf("4. Exit\n");
}

int main(void) {
    RouteResult last_route = {0};
    int running = 1;

    printf("Emergency Medical Supply Routing System\n");
    printf("Road distances are measured in kilometres.\n");

    while (running) {
        int choice;
        int menu_status;

        display_menu();

        menu_status = read_menu_choice(&choice);

        if (menu_status == 0) {
            printf("\nEnd of input. Exiting safely.\n");
            break;
        }

        if (menu_status < 0) {
            fprintf(stderr, "\nInput-stream error. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1:
                display_locations();
                break;
            case 2: {
                int start_idx;
                int dest_idx;
                int status;
                RouteResult candidate = {0};
                display_locations();
                status = read_location_index(
                    "Enter exact starting location name: ",
                    &start_idx
                );
                if (status == 0) {
                    printf("\nEnd of input. Exiting safely...\n");
                    running = 0;
                    break;
                }
                if (status < 0) {
                    fprintf(stderr, "\nInput-stream error. Exiting...\n");
                    running = 0;
                    break;
                }
                if (status == 2) {break;}
                status = read_location_index(
                    "Enter exact destination location name: ",
                    &dest_idx
                );
                if (status == 0) {
                    printf("\nEnd of input. Exiting safely.\n");
                    running = 0;
                    break;
                }
                if (status < 0) {
                    fprintf(stderr, "\nInput-stream error. Exiting.\n");
                    running = 0;
                    break;
                }
                if (status == 2) {break;}
                if (find_shortest_route(
                        ROAD_GRAPH,
                        start_idx,
                        dest_idx,
                        &candidate)) {
                    last_route = candidate;
                    display_route(&last_route);
                } else {
                    printf("Unable to calculate a valid route.\n");
                    printf("Destination may be unreachable, "
                           "or the graph may be invalid.\n");
                }
                break;
            }

            case 3:
                if (last_route.is_valid) {
                    printf("\nLast calculated route: %s -> %s\n",
                           LOCATION_NAMES[last_route.start_idx],
                           LOCATION_NAMES[last_route.dest_idx]);
                    printf("Total route distance: %d km\n",
                           last_route.total_distance);
                } else {
                    printf("\nNo successful route has been calculated yet.\n");
                    printf("Select option 2 first.\n");
                }
                break;

            case 4:
                printf("\nExiting the routing system...\nGoodbye:)\n");
                running = 0;
                break;

            default:
                printf("Invalid menu choice.\n");
                break;
        }
    }

    return 0;
}

