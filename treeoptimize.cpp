#include <math.h>
#include "tree.h"

#define PLACEVALUE(operation) node->value.lf = operation
#define DESTRUCT_CHILDREN(node) NodeDtorDiff(&(node->left)); NodeDtorDiff(&(node->right)); node->left == NULL; node->right == NULL;
#define ISNUM(node, num) node->type == TYPE_NUM && fabs(node->value.lf - num) <= FLT_ERR
TreeErr FoldNode(Node_t * node, bool *is_folded) {
    double left = node->left->value.lf;
    double right = node->right->value.lf;
    switch(node->value.operation) {
        case OP_ADD: PLACEVALUE(left + right); break;
        case OP_SUB: PLACEVALUE(left - right); break;
        case OP_MUL: PLACEVALUE(left * right); break;
        case OP_DIV: 
            if (fabs(right) <= FLT_ERR) 
                ADD_ERROR_AND_RETURN(ERR_FOLD_DIVISION_BY_ZERO, "division by zero happened at operation: DIV, values: %lf / 0", left); 
            PLACEVALUE(left / right); 
            DESTRUCT_CHILDREN(node)
            break;

        case OP_POW: PLACEVALUE(pow(left, right));  break;
        case OP_SIN: PLACEVALUE(sin(right));        break;
        case OP_COS: PLACEVALUE(cos(right));        break;
        case OP_TG:  PLACEVALUE(tan(right));        break;
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
            if (right <= 0)
                ADD_ERROR_AND_RETURN(ERR_FOLD_INVALID_ARGUMENT, "argument of logarifm cant be <= 0! Your argument: %lf", right); 
            PLACEVALUE(log10(right));
            break;
        
    }
    node->type = TYPE_NUM;
    DESTRUCT_CHILDREN(node);
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
    node->value.var = var;
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

TreeErr FoldIfUnnecessaryOperation(Node_t * node, bool *is_folded) {
    if (node->type == TYPE_OP && node->left->type != TYPE_OP && node->right->type != TYPE_OP) {
        double left = node->left->value.lf;
        double right = node->right->value.lf;
        switch(node->value.operation) {
            case OP_ADD: if (ISNUM(node->left, 0) || ISNUM(node->right, 0))  FoldToNum(node, (left != 0) ? left : right, is_folded); break;
            case OP_SUB: if (ISNUM(node->left, 0) || ISNUM(node->right, 0))  FoldToNum(node, (left != 0) ? left : right, is_folded); break;
            case OP_MUL: if (ISNUM(node->left, 0) || ISNUM(node->right, 0))  FoldToNum(node, 0, is_folded);                          break;
            case OP_DIV:
                if (ISNUM(node->right, 0))
                    ADD_ERROR_AND_RETURN(ERR_FOLD_DIVISION_BY_ZERO, "divided by zero");
                if (ISNUM(node->left, 0)) {
                    FoldToNum(node, 0, is_folded);
                    *is_folded = true;
                }
                break;

            case OP_POW:
                if (ISNUM(node->right, 1)) {
                    FoldToVar(node, is_folded);
                    *is_folded = true;
                    break;
                }
                if (ISNUM(node->left, 0)) {
                    if (right <= 0) {
                        ADD_ERROR_AND_RETURN(ERR_FOLD_INVALID_ARGUMENT, "0 ^ (value <= 0) is undefined"); 
                    }
                    FoldToNum(node, 0, is_folded);
                    break;
                }
                if (ISNUM(node->right, 0) || ISNUM(node->left, 1))
                    FoldToNum(node, 1, is_folded);
                    break;
                break;
            default:
                return TREE_NFOLDED;
        }
        return TREE_FOLDED;
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