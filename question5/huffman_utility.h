#ifndef HUFFMAN_UTILITY_H
#define HUFFMAN_UTILITY_H
#include <stdint.h>
#define SYMBOL_COUNT 256
#define MAX_TREE_NODES 511
#define MAX_CODE_LEN 256

typedef struct HuffmanNode {
    unsigned char ch;
    uint64_t freq;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
} HuffmanNode;

typedef enum {
    INPUT_SUCCESS,
    INPUT_OVERLONG,
    INPUT_EMPTY,
    INPUT_EOF
} InputStatus;

HuffmanNode *build_huffman_tree(
    const uint64_t freq[SYMBOL_COUNT]);

void free_tree(HuffmanNode *root);

void get_codes(
    HuffmanNode *root,
    char codes[SYMBOL_COUNT][MAX_CODE_LEN],
    char *current_code,
    int depth);

void compress_log(void);
void decompress_log(void);
void verify_restored_log(void);

InputStatus read_line(char *buffer, int buffer_size);

#endif
