#ifndef TREEOPTIMIZE_H
#define TREEOPTIMIZE_H

#include "tree.h"

TreeErr FoldConstantsInNode(Node_t *cur_node, bool *is_folded);
TreeErr FoldNode(Node_t * node, bool *is_folded);
TreeErr FoldToNum(Node_t *node, double value, bool *is_folded);
TreeErr FoldToVar(Node_t *node, bool *is_folded);
TreeErr FoldConstantsInNode(Node_t *cur_node, bool *is_folded);
TreeErr FoldIfUnnecessaryOperation(Node_t * node, bool *is_folded);
TreeErr RemoveUnnecessary(Node_t *cur_node, bool *is_folded);
void OptimizeTree(Node_t * DiffNode, size_t num_of_derivative);
#endif // TREEOPTIMIZE_H