#ifndef TREEDUMP_H
#define TREEDUMP_H

#include "tree.h"
#include "../my_libs/error_manage.h"

void print_site_header();
void get_timed_file_name(char log_file_name[]);
error_t create_tree_graph(DiffTree_t *tree);
TreeErr print_nodes_to_dump_file(Node_t * node, DiffTree_t * tree, FILE *fp, dirType type_of_direction, int *counter);
error_t open_live_server();
void print_divider(FILE * fp);
void print_site_toes();
void print_site_header();
void print_to_html(DiffTree_t *tree, TreeCmd command, TreeElem_t value);

#endif // TREEDUMP_H