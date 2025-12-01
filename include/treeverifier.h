#ifndef TREEVERIFIER_H
#define TREEVERIFIER_H

#include "../my_libs/error_manage.h"
#include "tree.h"

TreeErr NodeVerify(Node_t *node, Node_t *root_ptr);
TreeErr AllNodesVerify(Node_t *node, int *counter, DiffTree_t *tree);
TreeErr TreeVerify(DiffTree_t * tree);

#endif //TREEVERIFIER_H