#include <stdio.h>
#include <stdlib.h>
#include "huffman_utility.h"

int main(void)
{
    char choice_buffer[32];

    for (;;) {
        printf("\nEMBEDDED LOG COMPRESSION UTILITY\n\n");
        printf("1. Compress log\n");
        printf("2. Decompress log\n");
        printf("3. Verify restored log\n");
        printf("4. Exit\n");
        printf("Select an option (1-4): ");

        InputStatus status =
            read_line(choice_buffer, (int)sizeof(choice_buffer));
        if (status == INPUT_EOF) {
            putchar('\n');
            break;
        }

        if (status == INPUT_EMPTY || status == INPUT_OVERLONG) {
            printf("Invalid input. Enter a number from 1 to 4.\n");
            continue;
        }

        char *end = NULL;
        long choice = strtol(choice_buffer, &end, 10);

        if (end == choice_buffer || *end != '\0') {
            printf("Invalid input. Enter a number from 1 to 4.\n");
            continue;
        }

        switch (choice) {
            case 1:
                compress_log();
                break;
            case 2:
                decompress_log();
                break;
            case 3:
                verify_restored_log();
                break;
            case 4:
                printf("Exiting the utility...\n");
                return 0;
            default:
                printf("Invalid option. Enter a number from 1 to 4.\n");
                break;
        }
    }

    return 0;
}

