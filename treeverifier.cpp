#include "treeverifier.h"
#include "tree.h"
#include "../my_libs/error_manage.h"

extern error_t error;

TreeErr NodeVerify(Node_t *node, Node_t *root_ptr) {
    if (node == NULL)
        return OK;
    if ((node->left != NULL && node->left->rank < node->rank) || (node->right != NULL && node->right->rank < node->rank)) {
        ADD_ERROR_AND_RETURN(ERR_NODE_POINTS_TO_LOWER_RANK, "L or R points to invalid rank:\n"
                                                            "cur: %p, c_r: %d, l_r: %d",
                                                            node, (node->left != NULL) ? node->left->rank : 0, (node->right != NULL) ? node->right->rank : 0);
        }
    if (node->parent == NULL && node != root_ptr)
        ADD_ERROR_AND_RETURN(ERR_NO_PARENT, "parent: %p\n", node->parent);

    bool is_left_incorrect  = node->left  != NULL && node != node->left->parent;
    bool is_right_incorrect = node->right != NULL && node != node->right->parent;
    
    if (is_left_incorrect || is_right_incorrect) {
        char error_left[MAX_STR_SIZE]  = {};
        char error_right[MAX_STR_SIZE] = {};
        if (node->left != NULL)
            snprintf(error_left,  MAX_STR_SIZE - 1, "left sons parent:  %p\nbut actual parent has addr: %p\n\n", node->left->parent, node);
        if (node->right != NULL)
            snprintf(error_right, MAX_STR_SIZE - 1, "right sons parent: %p\nbut actual parent has addr: %p\n\n", node->right->parent, node);

        if (is_left_incorrect && is_right_incorrect) 
            ADD_ERROR_AND_RETURN(ERR_INVALID_RELATION_WITH_DAD_AND_SON, "%s%s", error_left, error_right);
        if (is_left_incorrect)
            ADD_ERROR_AND_RETURN(ERR_INVALID_RELATION_WITH_DAD_AND_SON, "%s", error_left);
        if (is_right_incorrect)
            ADD_ERROR_AND_RETURN(ERR_INVALID_RELATION_WITH_DAD_AND_SON, "%s", error_right);
    }
    return OK;
}

TreeErr TreeVerify(DiffTree_t *tree) {
    sassert(tree, ERR_PTR_NULL);

    int counter = 0;
    TreeErr err = AllNodesVerify(tree->root, &counter, tree);
    // if (counter != tree->num_of_nodes) {
    //     ADD_ERROR_AND_RETURN(ERR_INVALID_SIZE, "size: %d, exp_size: %zu", counter, tree->num_of_nodes);
    // }

    return err;
}

TreeErr AllNodesVerify(Node_t* node, int *counter, DiffTree_t * tree) {
    if (node == NULL)
        return OK;
    CHECK_FUNC_AND_RET_IF_ERR(NodeVerify(node, tree->root));
    // if (*counter > tree->num_of_nodes)
    //     ADD_ERROR_AND_RETURN(ERR_INVALID_SIZE, "size: %d, max_size: %zu", *counter, tree->num_of_nodes);
    
    (*counter)++;
    if (node->left != NULL)
        CHECK_FUNC_AND_RET_IF_ERR(AllNodesVerify(node->left,  counter, tree));
    if (node->right != NULL)
        CHECK_FUNC_AND_RET_IF_ERR(AllNodesVerify(node->right,  counter, tree));

    return OK;
}