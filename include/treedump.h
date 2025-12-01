#ifndef TREEDUMP_H
#define TREEDUMP_H

#include <string.h>
#include "tree.h"
#include "../my_libs/error_manage.h"

#define DUMPNODE(node, need_division, ...) {\
    DiffTree_t * tree10 = (DiffTree_t *) calloc(1, sizeof(DiffTree_t));\
    TreeCtorDiff_internal(tree10);\
\
    tree10->root = node;\
    tree10->num_of_nodes = GetNumOfNodes(node);\
    create_tree_graph(tree10);\
    char str[MAX_STR_SIZE] = {};\
    snprintf(str, MAX_STR_SIZE - 1, __VA_ARGS__);\
    print_to_html(tree10, need_division, str);\
\
    free(tree10->variables);\
    free(tree10);\
}

#define DUMPTREE(tree, ...) {\
    create_tree_graph(tree);\
\
    char str[MAX_STR_SIZE] = {};\
    snprintf(str, MAX_STR_SIZE - 1, __VA_ARGS__);\
    print_to_html(tree, YES_DIVISION, str);\
}

size_t GetNumOfNodes(Node_t * node);
void get_timed_file_name(char log_file_name[]);
TreeErr create_tree_graph(DiffTree_t *tree);
TreeErr print_nodes_to_dump_file(Node_t * node, DiffTree_t * tree, FILE *fp, dirType type_of_direction, int *counter);
error_t open_live_server();
void print_divider(FILE * fp);
void PrintSiteToes();
void PrintSiteHeader();
void print_to_html(DiffTree_t *tree, TreeNeedDiv needs_division, const char * str);
#endif // TREEDUMP_H