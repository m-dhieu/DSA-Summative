#include "huffman_utility.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INPUT_FILE "device_log.txt"
#define ARCHIVE_FILE "device_log.huff"
#define RESTORED_FILE "device_log_restored.txt"
#define MAGIC "HUF1"
#define MAGIC_SIZE 4U
#define HEADER_SIZE (MAGIC_SIZE + 8U + (SYMBOL_COUNT * 8U))

typedef struct {
    HuffmanNode *items[MAX_TREE_NODES];
    int size;
} MinHeap;

static void swap_nodes(HuffmanNode **a, HuffmanNode **b) 
{
    HuffmanNode *temp = *a;
    *a = *b;
    *b = temp;
}

static void heapify(MinHeap *heap, int index)
{
    for (;;) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        if (left < heap->size &&
            heap->items[left]->freq < heap->items[smallest]->freq) {
            smallest = left;
        }
        if (right < heap->size &&
            heap->items[right]->freq < heap->items[smallest]->freq) {
            smallest = right;
        }
        if (smallest == index) {break;}
        swap_nodes(&heap->items[index], &heap->items[smallest]);
        index = smallest;
    }
}

static void heap_push(MinHeap *heap, HuffmanNode *node)
{
    int index = heap->size++;
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap->items[parent]->freq <= node->freq) {break;}
        heap->items[index] = heap->items[parent];
        index = parent;
    }
    heap->items[index] = node;
}

static HuffmanNode *heap_pop(MinHeap *heap)
{
    if (heap->size == 0) {return NULL;}
    HuffmanNode *minimum = heap->items[0];
    HuffmanNode *last = heap->items[--heap->size];
    if (heap->size > 0) {
        heap->items[0] = last;
        heapify(heap, 0);
    }
    return minimum;
}

static HuffmanNode *create_node(
    unsigned char ch,
    uint64_t freq,
    HuffmanNode *left,
    HuffmanNode *right)
{
    HuffmanNode *node = malloc(sizeof(*node));
    if (node == NULL) {return NULL;}
    node->ch = ch;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

void free_tree(HuffmanNode *root)
{
    if (root == NULL) {return;}
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

HuffmanNode *build_huffman_tree(
    const uint64_t freq[SYMBOL_COUNT])
{
    MinHeap heap = {{0}, 0};
    for (int i = 0; i < SYMBOL_COUNT; ++i) {
        if (freq[i] == 0) {continue;}
        HuffmanNode *leaf = create_node(
            (unsigned char)i, freq[i], NULL, NULL);
        if (leaf == NULL) {
            while (heap.size > 0) {
                free_tree(heap_pop(&heap));
            }
            return NULL;
        }
        heap_push(&heap, leaf);
    }

    if (heap.size == 0) {return NULL;}

    if (heap.size == 1) {
        HuffmanNode *leaf = heap_pop(&heap);
        HuffmanNode *root = create_node(0, leaf->freq, leaf, NULL);
        if (root == NULL) {free_tree(leaf);}
        return root;
    }

    while (heap.size > 1) {
        HuffmanNode *left = heap_pop(&heap);
        HuffmanNode *right = heap_pop(&heap);
        if (UINT64_MAX - left->freq < right->freq) {
            free_tree(left);
            free_tree(right);
            while (heap.size > 0) {free_tree(heap_pop(&heap));}
            return NULL;
        }
        HuffmanNode *parent = create_node(
            0, left->freq + right->freq, left, right);
        if (parent == NULL) {
            free_tree(left);
            free_tree(right);
            while (heap.size > 0) {free_tree(heap_pop(&heap));}
            return NULL;
        }
        heap_push(&heap, parent);
    }

    return heap_pop(&heap);
}

void get_codes(
    HuffmanNode *root,
    char codes[SYMBOL_COUNT][MAX_CODE_LEN],
    char *current_code,
    int depth)
{
    if (root == NULL || depth >= MAX_CODE_LEN) {return;}
    if (root->left == NULL && root->right == NULL) {
        current_code[depth] = '\0';
        memcpy(codes[root->ch], current_code, (size_t)depth + 1U);
        return;
    }
    if (depth >= MAX_CODE_LEN - 1) {return;}
    current_code[depth] = '0';
    get_codes(root->left, codes, current_code, depth + 1);
    current_code[depth] = '1';
    get_codes(root->right, codes, current_code, depth + 1);
}

static int write_u64(FILE *file, uint64_t value)
{
    unsigned char bytes[8];
    for (int i = 0; i < 8; ++i) {
        bytes[i] = (unsigned char)(value & 0xffU);
        value >>= 8;
    }
    return fwrite(bytes, 1, sizeof(bytes), file) == sizeof(bytes);
}

static int read_u64(FILE *file, uint64_t *value)
{
    unsigned char bytes[8];
    if (fread(bytes, 1, sizeof(bytes), file) != sizeof(bytes)) {return 0;}
    *value = 0;
    for (int i = 7; i >= 0; --i) {*value = (*value << 8) | bytes[i];}
    return 1;
}

static int write_header(
    FILE *file,
    uint64_t original_size,
    const uint64_t freq[SYMBOL_COUNT])
{
    if (fwrite(MAGIC, 1, MAGIC_SIZE, file) != MAGIC_SIZE ||
        !write_u64(file, original_size)) {return 0;}
    for (int i = 0; i < SYMBOL_COUNT; ++i) {
        if (!write_u64(file, freq[i])) {return 0;}
    }
    return 1;
}

static int read_header(
    FILE *file,
    uint64_t *original_size,
    uint64_t freq[SYMBOL_COUNT])
{
    char magic[MAGIC_SIZE];
    if (fread(magic, 1, sizeof(magic), file) != sizeof(magic) ||
        memcmp(magic, MAGIC, MAGIC_SIZE) != 0 ||
        !read_u64(file, original_size)) {return 0;}
    uint64_t total = 0;
    for (int i = 0; i < SYMBOL_COUNT; ++i) {
        if (!read_u64(file, &freq[i])) {return 0;}
        if (UINT64_MAX - total < freq[i]) {return 0;}
        total += freq[i];
    }
    return total == *original_size;
}

static int get_file_size(FILE *file, uint64_t *size)
{
    if (fseek(file, 0, SEEK_END) != 0) {return 0;}
    long end = ftell(file);
    if (end < 0 || fseek(file, 0, SEEK_SET) != 0) {return 0;}
    *size = (uint64_t)end;
    return 1;
}

static int expected_bit_count(
    const uint64_t freq[SYMBOL_COUNT],
    char codes[SYMBOL_COUNT][MAX_CODE_LEN],
    uint64_t *bit_count)
{
    uint64_t total = 0;
    for (int i = 0; i < SYMBOL_COUNT; ++i) {
        if (freq[i] == 0) {continue;}
        size_t length = strlen(codes[i]);
        if (length == 0 ||
            freq[i] > (UINT64_MAX - total) / length) {return 0;}
        total += freq[i] * (uint64_t)length;
    }
    *bit_count = total;
    return 1;
}

InputStatus read_line(char *buffer, int buffer_size)
{
    if (buffer == NULL || buffer_size < 2) {
        return INPUT_OVERLONG;
    }
    if (fgets(buffer, buffer_size, stdin) == NULL) {
        return INPUT_EOF;
    }
    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
        return buffer[0] == '\0' ? INPUT_EMPTY : INPUT_SUCCESS;
    }
    int ch = getchar();
    if (ch == EOF) {
        return buffer[0] == '\0' ? INPUT_EMPTY : INPUT_SUCCESS;
    }
    while (ch != '\n' && ch != EOF) {ch = getchar();}
    return INPUT_OVERLONG;
}

void compress_log(void)
{
    FILE *input = fopen(INPUT_FILE, "rb");
    if (input == NULL) {
        printf("ERROR: Cannot open %s.\n", INPUT_FILE);
        return;
    }

    uint64_t original_size;
    if (!get_file_size(input, &original_size) ||
        original_size > SIZE_MAX) {
        printf("ERROR: Can't determine a supported input size.\n");
        fclose(input);
        return;
    }

    unsigned char *data = NULL;
    if (original_size > 0) {
        data = malloc((size_t)original_size);
        if (data == NULL ||
            fread(data, 1, (size_t)original_size, input)
                != (size_t)original_size) {
            printf("ERROR: Could not read complete input file.\n");
            free(data);
            fclose(input);
            return;
        }
    }

    if (fclose(input) != 0) {
        printf("ERROR: Failed to close input file.\n");
        free(data);
        return;
    }

    uint64_t freq[SYMBOL_COUNT] = {0};
    for (uint64_t i = 0; i < original_size; ++i) {++freq[data[i]];}

    printf("\nCharacter Frequency Analysis\n");
    printf("%-14s %s\n", "Byte", "Frequency");

    for (int i = 0; i < SYMBOL_COUNT; ++i) {
        if (freq[i] == 0) {continue;}
        if (i == '\n') {
            printf("%-14s %" PRIu64 "\n", "'\\n'", freq[i]);
        } else if (i == '\r') {
            printf("%-14s %" PRIu64 "\n", "'\\r'", freq[i]);
        } else if (i == '\t') {
            printf("%-14s %" PRIu64 "\n", "'\\t'", freq[i]);
        } else if (i >= 32 && i <= 126) {
            printf("'%c'              %" PRIu64 "\n", i, freq[i]);
        } else {
            printf("0x%02X            %" PRIu64 "\n", i, freq[i]);
        }
    }

    HuffmanNode *root = NULL;
    char codes[SYMBOL_COUNT][MAX_CODE_LEN] = {{0}};
    char current_code[MAX_CODE_LEN] = {0};
    if (original_size > 0) {
        root = build_huffman_tree(freq);
        if (root == NULL) {
            printf("ERROR: Could not build Huffman tree.\n");
            free(data);
            return;
        }
        get_codes(root, codes, current_code, 0);
    }

    FILE *archive = fopen(ARCHIVE_FILE, "wb");
    if (archive == NULL) {
        printf("ERROR: Cannot create %s.\n", ARCHIVE_FILE);
        free_tree(root);
        free(data);
        return;
    }

    int success = write_header(archive, original_size, freq);
    unsigned char bit_buffer = 0;
    int bits_in_buffer = 0;
    for (uint64_t i = 0; success && i < original_size; ++i) {
        const char *code = codes[data[i]];
        if (code[0] == '\0') {
            success = 0;
            break;
        }
        for (size_t j = 0; code[j] != '\0'; ++j) {
            bit_buffer = (unsigned char)(
                (bit_buffer << 1) | (code[j] == '1'));
            if (++bits_in_buffer == 8) {
                if (fwrite(&bit_buffer, 1, 1, archive) != 1) {
                    success = 0;
                    break;
                }
                bit_buffer = 0;
                bits_in_buffer = 0;
            }
        }
    }

    if (success && bits_in_buffer > 0) {
        bit_buffer <<= (8 - bits_in_buffer);
        if (fwrite(&bit_buffer, 1, 1, archive) != 1) {success = 0;}
    }

    free_tree(root);
    free(data);
    if (fclose(archive) != 0) {success = 0;}
    if (!success) {
        printf("ERROR: Compression failed...incomplete archive removed.\n");
        remove(ARCHIVE_FILE);
        return;
    }

    FILE *check = fopen(ARCHIVE_FILE, "rb");
    uint64_t compressed_size = 0;
    if (check == NULL || !get_file_size(check, &compressed_size)) {
        printf("ERROR: Could not measure archive size.\n");
        if (check != NULL) {fclose(check);}
        return;
    }
    if (fclose(check) != 0) {
        printf("ERROR: Failed to close archive after measurement.\n");
        return;
    }

    printf("\nOriginal size:       %" PRIu64 " bytes\n", original_size);
    printf("Compressed size:     %" PRIu64 " bytes\n", compressed_size);

    if (original_size == 0) {
        printf("Compression ratio:   N/A (empty input)\n");
        printf("Space saved:         N/A (empty input)\n");
    } else {
        double ratio =
            100.0 * (double)compressed_size / (double)original_size;
        double savings = 100.0 - ratio;
        printf("Compression ratio:   %.2f%% of original size\n", ratio);
        printf("Space saved:         %.2f%%\n", savings);
    }

    printf("Archive created: %s\n", ARCHIVE_FILE);
}

void decompress_log(void)
{
    FILE *archive = fopen(ARCHIVE_FILE, "rb");
    if (archive == NULL) {
        printf("ERROR: Cannot open %s.\n", ARCHIVE_FILE);
        return;
    }

    uint64_t original_size;
    uint64_t freq[SYMBOL_COUNT];
    if (!read_header(archive, &original_size, freq)) {
        printf("ERROR: Invalid or truncated archive header.\n");
        fclose(archive);
        return;
    }

    uint64_t archive_size;
    if (!get_file_size(archive, &archive_size) ||
        archive_size < HEADER_SIZE) {
        printf("ERROR: Invalid archive size.\n");
        fclose(archive);
        return;
    }

    if (original_size == 0) {
        if (archive_size != HEADER_SIZE) {
            printf("ERROR: Empty archive contains unexpected data.\n");
            fclose(archive);
            return;
        }
        FILE *output = fopen(RESTORED_FILE, "wb");
        if (output == NULL) {
            printf("ERROR: Cannot create %s.\n", RESTORED_FILE);
            fclose(archive);
            return;
        }
        int close_ok = fclose(output) == 0;
        fclose(archive);
        if (!close_ok) {
            printf("ERROR: Failed to finalize restored file.\n");
            return;
        }
        printf("SUCCESS: Empty file restored.\n");
        verify_restored_log();
        return;
    }

    int unique_count = 0;
    for (int i = 0; i < SYMBOL_COUNT; ++i) {
        if (freq[i] > 0) {++unique_count;}
    }
    if (unique_count == 0) {
        printf("ERROR: Nonempty archive contains no symbols.\n");
        fclose(archive);
        return;
    }

    HuffmanNode *root = build_huffman_tree(freq);
    if (root == NULL) {
        printf("ERROR: Could not rebuild Huffman tree.\n");
        fclose(archive);
        return;
    }

    char codes[SYMBOL_COUNT][MAX_CODE_LEN] = {{0}};
    char current_code[MAX_CODE_LEN] = {0};
    get_codes(root, codes, current_code, 0);
    uint64_t bit_count;
    if (!expected_bit_count(freq, codes, &bit_count) ||
        bit_count > UINT64_MAX - 7U) {
        printf("ERROR: Invalid encoded length.\n");
        free_tree(root);
        fclose(archive);
        return;
    }

    uint64_t payload_size = (bit_count + 7U) / 8U;
    if (payload_size > UINT64_MAX - HEADER_SIZE ||
        archive_size != HEADER_SIZE + payload_size ||
        fseek(archive, (long)HEADER_SIZE, SEEK_SET) != 0) {
        printf("ERROR: Archive size doesn't match its metadata.\n");
        free_tree(root);
        fclose(archive);
        return;
    }

    FILE *output = fopen(RESTORED_FILE, "wb");
    if (output == NULL) {
        printf("ERROR: Cannot create %s.\n", RESTORED_FILE);
        free_tree(root);
        fclose(archive);
        return;
    }

    uint64_t decoded_freq[SYMBOL_COUNT] = {0};
    uint64_t written = 0;
    uint64_t bits_read = 0;
    HuffmanNode *current = root;
    unsigned char byte = 0;
    int success = 1;
    while (written < original_size && bits_read < bit_count) {
        if (fread(&byte, 1, 1, archive) != 1) {
            success = 0;
            break;
        }
        for (int bit_index = 7;
             bit_index >= 0 &&
             written < original_size &&
             bits_read < bit_count;
             --bit_index) {
            int bit = (byte >> bit_index) & 1;
            current = bit ? current->right : current->left;
            ++bits_read;
            if (current == NULL) {
                success = 0;
                break;
            }
            if (current->left == NULL && current->right == NULL) {
                if (fputc(current->ch, output) == EOF) {
                    success = 0;
                    break;
                }
                ++decoded_freq[current->ch];
                ++written;
                current = root;
            }
        }
        if (!success) {
            break;
        }
    }

    if (success && bit_count % 8U != 0U) {
        unsigned int padding = (unsigned int)(8U - bit_count % 8U);
        unsigned char mask = (unsigned char)((1U << padding) - 1U);
        if ((byte & mask) != 0U) {success = 0;}
    }

    if (current != root ||
        written != original_size ||
        bits_read != bit_count) {success = 0;}

    for (int i = 0; success && i < SYMBOL_COUNT; ++i) {
        if (decoded_freq[i] != freq[i]) {success = 0;}
    }

    if (fclose(output) != 0) {success = 0;}
    if (fclose(archive) != 0) {success = 0;}

    free_tree(root);

    if (!success) {
        printf("ERROR: Archive is corrupt or decompression failed.\n");
        printf("Restored output may be incomplete.\n");
        return;
    }

    printf("SUCCESS: Decompression complete: %s\n", RESTORED_FILE);
    verify_restored_log();
}

void verify_restored_log(void)
{
    FILE *original = fopen(INPUT_FILE, "rb");
    FILE *restored = fopen(RESTORED_FILE, "rb");
    if (original == NULL || restored == NULL) {
        printf("Verification: FAILED (a required file is missing).\n");
        if (original != NULL) {fclose(original);}
        if (restored != NULL) {fclose(restored);}
        return;
    }

    int equal = 1;
    for (;;) {
        int a = fgetc(original);
        int b = fgetc(restored);
        if (a != b) {
            equal = 0;
            break;
        }
        if (a == EOF) {
            if (ferror(original) || ferror(restored)) {equal = 0;}
            break;
        }
    }

    if (fclose(original) != 0) {equal = 0;}
    if (fclose(restored) != 0) {equal = 0;}

    if (equal) {
        printf("Verification: SUCCESS\n");
        printf("Restored file matches the original byte for byte.\n");
    } else {
        printf("Verification: FAILED\n");
        printf("Restored file differs from the original.\n");
    }
}

