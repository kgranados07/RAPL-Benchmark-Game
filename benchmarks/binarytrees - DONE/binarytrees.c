// The Computer Language Benchmarks Game
// Based on the C++ program from Jon Harrop, Alex Mizrahi, and Bruno Coutinho.

#define MAXIMUM_LINE_WIDTH 60

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>

typedef off_t off64_t;

typedef struct apr_pool_block {
    struct apr_pool_block *next;
    size_t size;
    void *data;
} apr_pool_block;

typedef struct apr_pool_t {
    apr_pool_block *head;
} apr_pool_t;

static void apr_initialize(void) {}
static void apr_terminate(void) {}

static int apr_pool_create_unmanaged(apr_pool_t **pool) {
    *pool = calloc(1, sizeof(apr_pool_t));
    return (*pool == NULL);
}

static void *apr_palloc(apr_pool_t *pool, size_t size) {
    void *data = calloc(1, size);
    apr_pool_block *block = calloc(1, sizeof(apr_pool_block));

    if (!data || !block) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    block->data = data;
    block->size = size;
    block->next = pool->head;
    pool->head = block;

    return data;
}

static void apr_pool_clear(apr_pool_t *pool) {
    while (pool->head) {
        apr_pool_block *next = pool->head->next;
        free(pool->head->data);
        free(pool->head);
        pool->head = next;
    }
}

static void apr_pool_destroy(apr_pool_t *pool) {
    apr_pool_clear(pool);
    free(pool);
}

typedef intptr_t intnative_t;

typedef struct tree_node {
    struct tree_node *left_Node;
    struct tree_node *right_Node;
} tree_node;

static inline tree_node *create_Tree(
    const intnative_t tree_Depth,
    apr_pool_t *const memory_Pool)
{
    tree_node *const root_Node =
        apr_palloc(memory_Pool, sizeof(tree_node));

    if (tree_Depth > 0) {
        root_Node->left_Node =
            create_Tree(tree_Depth - 1, memory_Pool);
        root_Node->right_Node =
            create_Tree(tree_Depth - 1, memory_Pool);
    } else {
        root_Node->left_Node = NULL;
        root_Node->right_Node = NULL;
    }

    return root_Node;
}

static inline intnative_t compute_Tree_Checksum(
    const tree_node *const root_Node)
{
    if (root_Node->left_Node) {
        return compute_Tree_Checksum(root_Node->left_Node) +
               compute_Tree_Checksum(root_Node->right_Node) + 1;
    }

    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <tree_depth>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const intnative_t minimum_Tree_Depth = 4;

    intnative_t maximum_Tree_Depth = atoi(argv[1]);

    if (maximum_Tree_Depth < minimum_Tree_Depth + 2)
        maximum_Tree_Depth = minimum_Tree_Depth + 2;

    apr_initialize();

    apr_pool_t *memory_Pool;

    apr_pool_create_unmanaged(&memory_Pool);

    tree_node *stretch_Tree =
        create_Tree(maximum_Tree_Depth + 1, memory_Pool);

    printf("stretch tree of depth %jd\t check: %jd\n",
           (intmax_t)(maximum_Tree_Depth + 1),
           (intmax_t)compute_Tree_Checksum(stretch_Tree));

    apr_pool_destroy(memory_Pool);

    apr_pool_create_unmanaged(&memory_Pool);

    tree_node *long_Lived_Tree =
        create_Tree(maximum_Tree_Depth, memory_Pool);

    char output_Buffer[maximum_Tree_Depth + 1][MAXIMUM_LINE_WIDTH + 1];

    intnative_t current_Tree_Depth;

#pragma omp parallel for
    for (current_Tree_Depth = minimum_Tree_Depth;
         current_Tree_Depth <= maximum_Tree_Depth;
         current_Tree_Depth += 2) {

        intnative_t iterations =
            1 << (maximum_Tree_Depth - current_Tree_Depth +
                  minimum_Tree_Depth);

        apr_pool_t *thread_Memory_Pool;
        apr_pool_create_unmanaged(&thread_Memory_Pool);

        intnative_t total_Trees_Checksum = 0;

        for (intnative_t i = 1; i <= iterations; ++i) {

            tree_node *tree =
                create_Tree(current_Tree_Depth,
                            thread_Memory_Pool);

            total_Trees_Checksum +=
                compute_Tree_Checksum(tree);

            apr_pool_clear(thread_Memory_Pool);
        }

        apr_pool_destroy(thread_Memory_Pool);

        sprintf(output_Buffer[current_Tree_Depth],
                "%jd\t trees of depth %jd\t check: %jd\n",
                (intmax_t)iterations,
                (intmax_t)current_Tree_Depth,
                (intmax_t)total_Trees_Checksum);
    }

    for (current_Tree_Depth = minimum_Tree_Depth;
         current_Tree_Depth <= maximum_Tree_Depth;
         current_Tree_Depth += 2) {

        printf("%s", output_Buffer[current_Tree_Depth]);
    }

    printf("long lived tree of depth %jd\t check: %jd\n",
           (intmax_t)maximum_Tree_Depth,
           (intmax_t)compute_Tree_Checksum(long_Lived_Tree));

    apr_pool_destroy(memory_Pool);

    apr_terminate();

    return EXIT_SUCCESS;
}