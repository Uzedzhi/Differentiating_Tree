#include <math.h>
#include <string.h>
#include "tree.h"
#include "treedump.h"
#include "treelatexdump.h"
#include "treeoptimize.h"

#define PLACEVALUE(operation) node->value.lf = operation
#define DESTRUCT_CHILDREN(node) NodeDtorDiff(&(node->left)); NodeDtorDiff(&(node->right)); node->left = NULL; node->right = NULL;
#define ISNUM(node, num) node->type == TYPE_NUM && fabs(node->value.lf - num) <= FLT_ERR
#define ISVAR(node) node->type == TYPE_VAR
TreeErr FoldNode(Node_t * node, bool *is_folded) {
    double left = node->left->value.lf;
    double right = node->right->value.lf;
    switch(node->value.operation) {
        case OP_ADD:    PLACEVALUE(left + right);         break;
        case OP_SUB:    PLACEVALUE(left - right);         break;
        case OP_MUL:    PLACEVALUE(left * right);         break;
        case OP_DIV:    
            if (fabs(right) <= FLT_ERR) 
                ADD_ERROR_AND_RETURN(ERR_FOLD_DIVISION_BY_ZERO, "division by zero happened at operation: DIV, values: %lf / 0", left); 
            PLACEVALUE(left / right);
            break;

        case OP_POW:    PLACEVALUE(pow(left, right));     break;
        case OP_SIN:    PLACEVALUE(sin(right));           break;
        case OP_COS:    PLACEVALUE(cos(right));           break;
        case OP_TG:     PLACEVALUE(tan(right));           break;
        case OP_ARCSIN: PLACEVALUE(asin(right));          break;
        case OP_ARCCOS: PLACEVALUE(acos(right));          break;
        case OP_ARCTG:  PLACEVALUE(atan(right));          break;
        case OP_ARCCTG: PLACEVALUE(M_PI_2 - atan(right)); break;
        case OP_CTG: 
            if (fabs(tan(right)) <= FLT_ERR) 
                ADD_ERROR_AND_RETURN(ERR_FOLD_DIVISION_BY_ZERO, "when evaluating ctg = 1 / tg(%lf) , tg is 0. Division By zero", right); 
            PLACEVALUE(1.0 / tan(right));
            break;

        case OP_LN:  
            if (right <= 0)
                ADD_ERROR_AND_RETURN(ERR_FOLD_INVALID_ARGUMENT, "argument of logarifm cant be <= 0! Your argument: %lf", right); 
            PLACEVALUE(log(right));
            break;

        case OP_LOG: 
            if (left <= 0 || fabs(left - 1.0) <= FLT_ERR || right <= 0)
                ADD_ERROR_AND_RETURN(ERR_FOLD_INVALID_ARGUMENT, "argument of logarifm cant be <= 0! Your argument: %lf", right); 
            PLACEVALUE(log(right) / log(left));
            break;
        
    }
    node->type = TYPE_NUM;
    DESTRUCT_CHILDREN(node);
    return TREE_FOLDED;
}

TreeErr FoldToNode(Node_t *node, Node_t *keep_child, bool *is_folded) {
    Node_t *remove_child = (keep_child == node->left) ? node->right : node->left;
    
    node->type = keep_child->type;
    node->value = keep_child->value;
    
    if (node->type == TYPE_VAR) {
        node->value.var.name = strdup(keep_child->value.var.name);
    }
    
    Node_t *new_left = keep_child->left;
    Node_t *new_right = keep_child->right;
    
    keep_child->left = NULL;
    keep_child->right = NULL;
    
    NodeDtorDiff(&keep_child);
    NodeDtorDiff(&remove_child);
    
    node->left = new_left;
    node->right = new_right;
    if (node->left != NULL)
        node->left->parent = node;
    if (node->right != NULL)
        node->right->parent = node;
    
    *is_folded = true;
    return TREE_FOLDED;
}

TreeErr FoldToNum(Node_t *node, double value, bool *is_folded) {
    node->type = TYPE_NUM;
    node->value.lf = value;
    DESTRUCT_CHILDREN(node);
    *is_folded = true;
    return TREE_FOLDED;
}

TreeErr FoldToVar(Node_t *node, bool *is_folded) {
    Var_t var = {};
    if (node->left->type == TYPE_VAR)
        var = node->left->value.var;
    if (node->right->type == TYPE_VAR)
        var = node->right->value.var;

    node->type = TYPE_VAR;
    node->value.var.hash = var.hash;
    node->value.var.value = var.value;
    node->value.var.name = strdup(var.name);
    DESTRUCT_CHILDREN(node);
    *is_folded = true;
    return TREE_FOLDED;
}


TreeErr FoldConstantsInNode(Node_t *cur_node, bool *is_folded) {
    if (cur_node->left == NULL || cur_node->right == NULL)
        return TREE_NFOLDED;
    if (cur_node->left->type == TYPE_NUM && cur_node->right->type == TYPE_NUM) {
        *is_folded = true;
        return FoldNode(cur_node, is_folded);
    }

    TreeErr left_return     = FoldConstantsInNode(cur_node->left, is_folded);
    TreeErr right_return    = FoldConstantsInNode(cur_node->right, is_folded);
    if (ISVALUEERROR(left_return))
        return left_return;
    if (ISVALUEERROR(right_return))
        return right_return;
    
    if (left_return == TREE_FOLDED && right_return == TREE_FOLDED)
        return FoldNode(cur_node, is_folded);
    return TREE_NFOLDED;
}

void OptimizeTree(Node_t * DiffNode, size_t num_of_derivative) {
    bool is_folded = true;
    TreeErr return_val = TREE_NFOLDED;
    #ifdef OPTIMIZEDUMP
        bool need_dump = true;
    #else
        bool need_dump = false;
    #endif
    
    if (need_dump) {
            WriteLatexResult(DiffNode, "x", num_of_derivative, "Результат до оптимизации");
        }
    while (is_folded == true) {
        is_folded = false;
        TreeErr return_val_1_optimization = FoldConstantsInNode(DiffNode, &is_folded);
        if (need_dump) {
            WriteLatexResult(DiffNode, "x", num_of_derivative, "Результат после оптимизации сверткой констант:");
        }
        TreeErr return_val_2_optimization = RemoveUnnecessary(DiffNode, &is_folded);
        if (need_dump) {
            WriteLatexResult(DiffNode, "x", num_of_derivative, "Результат после оптимизации удаления ненужного:");
        }
        if (ISVALUEERROR(return_val_1_optimization) || ISVALUEERROR(return_val_2_optimization))
            break;
    }
}

TreeErr FoldIfUnnecessaryOperation(Node_t * node, bool *is_folded) {
    if (node->type == TYPE_OP) {
        double left = node->left->value.lf;
        double right = node->right->value.lf;
        switch(node->value.operation) {
            case OP_ADD:
                if (ISNUM(node->left, 0))
                    return FoldToNode(node, node->right, is_folded);
                if (ISNUM(node->right, 0))
                    return FoldToNode(node, node->left, is_folded);
                break;
            case OP_SUB:
                if (ISNUM(node->right, 0))
                    return FoldToNode(node, node->left, is_folded);
                if (ISVAR(node->left) && ISVAR(node->right) &&
                    strcmp(node->left->value.var.name, node->right->value.var.name) == 0)
                    return FoldToNum(node, 0, is_folded);
                break;
            case OP_MUL:
                if (ISNUM(node->left, 0) || ISNUM(node->right, 0))
                    return FoldToNum(node, 0, is_folded);       
                if (ISNUM(node->right, 1))
                    return FoldToNode(node, node->left, is_folded);
                if (ISNUM(node->left, 1))
                    return FoldToNode(node, node->right, is_folded);
                break;
            case OP_DIV:
                if (ISNUM(node->right, 0))
                    ADD_ERROR_AND_RETURN(ERR_FOLD_DIVISION_BY_ZERO, "divided by zero");
                if (ISNUM(node->left, 0))
                    return FoldToNum(node, 0, is_folded);
                if (ISVAR(node->left) && ISVAR(node->right))
                    return FoldToNum(node, 1, is_folded);
                if (ISNUM(node->right, 1))
                    return FoldToNode(node, node->left, is_folded);
                break;

            case OP_POW:
                if (ISVAR(node->left) && ISNUM(node->right, 1))
                    return FoldToVar(node, is_folded);
                if (ISNUM(node->left, 0)) {
                    if (right <= 0) {
                        ADD_ERROR_AND_RETURN(ERR_FOLD_INVALID_ARGUMENT, "0 ^ (value <= 0) is undefined"); 
                    }
                    return FoldToNum(node, 0, is_folded);
                }
                if (ISNUM(node->right, 0) || ISNUM(node->left, 1)) {
                    return FoldToNum(node, 1, is_folded);
                }
                break;
            default:
                return TREE_NFOLDED;
        }
        return TREE_NFOLDED;
    }
    return TREE_NFOLDED;
}

TreeErr RemoveUnnecessary(Node_t *cur_node, bool *is_folded) {
    if (cur_node->left == NULL || cur_node->right == NULL)
        return TREE_NFOLDED;

    TreeErr left_return     = RemoveUnnecessary(cur_node->left, is_folded);
    TreeErr right_return    = RemoveUnnecessary(cur_node->right, is_folded);
    if (ISVALUEERROR(left_return))
        return left_return;
    if (ISVALUEERROR(right_return))
        return right_return;

    return FoldIfUnnecessaryOperation(cur_node, is_folded);
}

#undef DESTRUCT_CHILDREN
#undef PLACEVALUE
#undef ISZERO