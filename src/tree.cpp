#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "dirent.h"

#include "../my_libs/sassert.h"
#include "../my_libs/error_manage.h"
#include "tree.h"
#include "treedump.h"
#include "treeverifier.h"
#include "treelatexdump.h"
#include "treeoptimize.h"
#include "treehelpers.h"

error_t error = {};

void TreeCtorDiff_internal(DiffTree_t * tree) {
    sassert(tree, ERR_PTR_NULL);

    tree->variables = (Var_t *) calloc(ARR_INIT_SIZE, sizeof(Var_t));
    sassert(tree->variables, ERR_PTR_NULL);

    for (size_t i = 0; i < ARR_INIT_SIZE; i++) {
        tree->variables[i].value = POISON;
    }
}

void NodeDtorDiff(Node_t **node) {
    sassert(node, ERR_PTR_NULL);

    if (*node != NULL && (*node)->left != NULL)
        NodeDtorDiff(&(*node)->left);
    if (*node != NULL && (*node)->right != NULL)
        NodeDtorDiff(&(*node)->right);
    if ((*node) != NULL && (*node)->type == TYPE_VAR && (*node)->value.var.name != NULL) {
        free((*node)->value.var.name);
        (*node)->value.var = {};
    }
    free(*node);
    *node = NULL;
}
void TreeDtorDiff(DiffTree_t* tree) {
    if (tree != NULL) {
        if (tree->root != NULL) {
            NodeDtorDiff(&(tree->root));
        }
        free(tree->variables);
        free(tree);
    }

}

void write_nodes_to_file(Node_t *node, FILE *fp) {
    if (node->type == TYPE_VAR)
        fprintf(fp, "(\"%s\"", (node->value).var.name);
    else
        fprintf(fp, "(\"%lf\")", (node->value).lf);
    if (node->left != NULL) {
        write_nodes_to_file(node->left, fp);
    }
    if (node->right != NULL) {
        write_nodes_to_file(node->right, fp);
    }
    if (node->left == NULL && node->right == NULL)
        fprintf(fp, "nilnil)");
    else
        fprintf(fp, ")");
}

int write_Tree_to_file(DiffTree_t *tree, const char * log_file_name) {
    sassert(tree, ERR_PTR_NULL);

    FILE *fp = fopen(log_file_name, "w");
    sassert(fp,   ERR_PTR_NULL);
    CHECK_FUNC_AND_RET_IF_ERR(TreeVerify(tree));

    fprintf(fp, "num_of_nodes: %zu\n", tree->num_of_nodes);
    write_nodes_to_file(tree->root, fp);

    fclose(fp);
    return OK;
}

double CalculateTree(DiffTree_t *tree) {
    sassert(tree, ERR_PTR_NULL);

    double answer = GetCalculatedAnswer(tree, tree->root, POISON, POISON);
    if (answer == NAN) {
        add_error(ERR_CALCULATION_INCORRECT, "answer is incorrect");
    }

    return answer;
}

#define AL GetCalculatedAnswer(tree, cur_node->left, KnownVarHash, KnownVarValue)
#define AR GetCalculatedAnswer(tree, cur_node->right, KnownVarHash, KnownVarValue)
#define NEGATIVE(node) node->type == TYPE_NUM && node->value.lf <= 0
#define ISNUM(node, num) node->type == TYPE_NUM && is_same(node->value.lf, num)
#define L cur_node->left
#define R cur_node->right
double GetCalculatedAnswer(DiffTree_t *tree, Node_t *cur_node, size_t KnownVarHash, double KnownVarValue) {
    size_t hash_cur = 0;
    size_t i = 0;
    bool found = false;
    switch(cur_node->type) {
        case TYPE_OP:
            switch((cur_node->value).operation) {
                case OP_ADD:    return AL + AR;
                case OP_DIV:    if (is_same(AR, 0))
                                    return NAN;
                                return AL / AR;
                case OP_MUL:    return AL * AR;
                case OP_SUB:    return AL - AR;
                case OP_POW:    return pow(AL, AR);
                case OP_SIN:    return sin(AR);
                case OP_COS:    return cos(AR);
                case OP_TG:     return tan(AR);
                case OP_LN:     return log(AR);

                case OP_CTG:    if (is_same((sin(AR)), 0))
                                    return NAN;
                                return cos(AR) / sin(AR);


                case OP_LOG:    if (is_same(log(AL), 0))
                                    return NAN;
                                return log(AR) / log(AL);
                case OP_SH:     return sinh(AR);
                case OP_CH:     return cosh(AR);
                case OP_TH:     return tanh(AR);

                case OP_CTH:     if (is_same(tanh(AR), 0))
                                    return NAN;
                                return 1 / tanh(AR);

                case OP_ARCSIN: return asin(AR);
                case OP_ARCCOS: return acos(AR);
                case OP_ARCTG:  return atan(AR);
                case OP_ARCCTG: return M_PI_2 - atan(AR);
            }
            break;
        case TYPE_NUM:
            return (cur_node->value).lf;
        case TYPE_VAR:
            hash_cur = sdbm((cur_node->value).var.name);
            while (tree->variables[i].value != POISON) {
                if (tree->variables[i].hash == hash_cur) {
                    found = true;
                    break;
                }
                i++;
            }
            if (hash_cur == KnownVarHash)
                tree->variables[i].value = KnownVarValue;
            else if (!found) {
                printf("\nwhat value does %s have? ", cur_node->value.var.name);
                while(scanf("%lf", &(tree->variables[i].value)) != 1) {
                    skip_line(stdin);
                    printf("\n please try again: "); // todo to funcs
                }
            }
            cur_node->value.var.value = tree->variables[i].value;
            tree->variables[i].hash = hash_cur;
            tree->variables[i].name = cur_node->value.var.name;
            return cur_node->value.var.value;
        default:
            return NAN;
    }

    return NAN;
}
#undef AL
#undef AR

#define RETURN_AND_DUMP(diff_node, ...) {\
    Node_t *result_node = diff_node;\
    char str[MAX_STR_SIZE] = {};\
    snprintf(str, MAX_STR_SIZE - 1, __VA_ARGS__);\
    if (need_dump) {\
        WriteLatexStepOfDifferentiation(fp, node, result_node, DiffVarName, str);\
    }\
    return result_node;\
}
#define NUM_(num) NewNode(TYPE_NUM, (TreeElem_u) {.lf  = (double) num}, NULL, NULL)
#define VAR_ NewNode(TYPE_VAR, (TreeElem_u) {.var = {strdup(VarName), sdbm(VarName), a}}, NULL, NULL)
#define OP_(oper, node1, node2) NewNode(TYPE_OP, (TreeElem_u) {.operation = (oper)}, (node1), (node2))
#define ADD_(node1, node2)      OP_(OP_ADD,     node1, node2)
#define MUL_(node1, node2)      OP_(OP_MUL,     node1, node2)
#define LN_(node1)              OP_(OP_LN ,     NUM_(0), node1)
#define LOG_(node1, node2)      OP_(OP_LOG,     node1, node2)
#define DIV_(node1, node2)      OP_(OP_DIV,     node1, node2)
#define POW_(node1, node2)      OP_(OP_POW,     node1, node2)
#define SUB_(node1, node2)      OP_(OP_SUB,     node1, node2)
#define SIN_(node1, node2)      OP_(OP_SIN,     node1, node2)
#define COS_(node1, node2)      OP_(OP_COS,     node1, node2)
#define CH_(node1)              OP_(OP_CH,      NUM_(0), node1)
#define SH_(node1)              OP_(OP_SH,      NUM_(0), node1)
#define CTH_(node1)             OP_(OP_CTH,     NUM_(0), node1)
#define TH_(node1)              OP_(OP_TH,      NUM_(0), node1)
#define dL differentiate(fp, node->left, num_of_nodes, DiffVarName, need_dump)
#define dR differentiate(fp, node->right, num_of_nodes, DiffVarName, need_dump)
#define cR CopyNodes(node->right)
#define cL CopyNodes(node->left)

DiffTree_t *GetTangentTree(DiffTree_t *tree, DiffTree_t *DiffTree, double a, const char * VarName) {
    sassert(tree, ERR_PTR_NULL);
    sassert(DiffTree, ERR_PTR_NULL);


    double FxResult     = GetCalculatedAnswer(tree, tree->root, sdbm(VarName), a);
    double DiffFxResult = GetCalculatedAnswer(DiffTree, DiffTree->root, sdbm(VarName), a);
    TreeCtorDiff(TangentTree);
    TangentTree->root = ADD_(
                            MUL_(
                                NUM_(DiffFxResult),
                                SUB_(
                                    VAR_,
                                    NUM_(a)
                                )
                            ),
                            NUM_(FxResult)
                        );
    TangentTree->num_of_nodes = GetNumOfNodes(TangentTree->root);
    return TangentTree;
}

double GetAnswerNewtonsMethod(DiffTree_t * func, const char * VarName) {
    size_t num_of_nodes = 0;
    double cur_point = 0.0;
    double TangentRootAnswer = 0.0;
    double FuncAnswerWithTangentRoot = NAN;
    TreeCtorDiff(DiffTree);
    
    DiffTree->root      = differentiate(stdout, func->root, &num_of_nodes, VarName, false);
    double FxResult     = 0.0;
    double DiffFxResult = 0.0;
    size_t iterations = 0;

    while (!is_same(FuncAnswerWithTangentRoot, 0.0) && iterations < 100) {
        FxResult            = GetCalculatedAnswer(func, func->root, sdbm(VarName), cur_point);
        DiffFxResult        = GetCalculatedAnswer(DiffTree, DiffTree->root, sdbm(VarName), cur_point);
        if (DiffFxResult == 0)
            TangentRootAnswer = cur_point;
        else
            TangentRootAnswer = (-FxResult) / (DiffFxResult) + cur_point;
        FuncAnswerWithTangentRoot = GetCalculatedAnswer(func, func->root, sdbm(VarName), TangentRootAnswer);
        cur_point = TangentRootAnswer;
        iterations++;
    }
    if (iterations == 100)
        cur_point = NAN;

    TreeDtorDiff(DiffTree);
    return cur_point;

}

Node_t * DiffTypeNum(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                     const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(NUM_(0), "differential of number is 0");
}

Node_t * DiffTypeVar(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                     const char *DiffVarName, bool need_dump) {
    if (strcmp(node->value.var.name, DiffVarName) == 0) {
        RETURN_AND_DUMP(NUM_(1), "differential of variable, which we are differentiating with is 1");
    }
    else {
        RETURN_AND_DUMP(NUM_(0), "differential of variable, which we are NOT differentiating with is 0");
    }
}

Node_t * DiffOpAdd(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(ADD_(dL, dR), "differential of addition (x + y)\' = x\' + y\'");
}

Node_t * DiffOpSub(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(SUB_(dL, dR), "differential of difference (x - y)\' = x\' - y\'");
}

Node_t * DiffOpMul(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(ADD_(
                        MUL_(dL, cR),
                        MUL_(cL, dR)
                    ), "differential of multiplication (x * y)' = x * y' + x' * y");
}

Node_t * DiffOpDiv(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(DIV_(
                        SUB_(
                            MUL_(dL, cR),
                            MUL_(cL, dR)
                        ),
                        POW_(cR, NUM_(2))
                    ), "differential of division (x / y)\' = (x\' * y - x * y\') / y^2");
}

Node_t * DiffOpPow(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    switch(GetVarPosType(node, DiffVarName)) {
        case NUMNUM:
            RETURN_AND_DUMP(NUM_(0), "differential of num to the power of num is 0\n\n"
                        "\\(a^b)\' = 0\n");
        case VARNUM:
            RETURN_AND_DUMP(MUL_(
                                MUL_(
                                    cR,
                                    POW_(
                                        cL, 
                                        SUB_(cR, NUM_(1))
                                    )
                                ),
                                dL
                            ), "differential of var to the power of num is (x^a)\' = a * x^(a-1) * x\'\n\n");
        case NUMVAR:
            RETURN_AND_DUMP(MUL_(
                                MUL_(
                                    LN_(cL),
                                    POW_(
                                        cL, 
                                        cR
                                    )
                                ),
                                dR
                            ), "differential of num to the power of var is (a^x)\' = a^x * ln(a) * x\'\n\n");
        case VARVAR:
            RETURN_AND_DUMP(MUL_(
                                POW_(
                                    NUM_(CONST_E), 
                                    MUL_(
                                        cR,
                                        LN_(cL)
                                    )
                                ),
                                ADD_(
                                    MUL_(
                                        dR,
                                        LN_(cL)
                                    ),
                                    DIV_(
                                        cR,
                                        cL
                                    )
                                )
                            ), "differential of var to the power of var is (u^v)\' = e^(v*ln(u)) * (v\' * ln(u) + v/u)\n\n");
    }
    return NULL;
}

Node_t * DiffOpLn(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                  const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(DIV_(
                        dR,
                        cR
                    ), "differential of ln is (ln(x))\' = x\' / x\n\n");
}

Node_t * DiffOpLog(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    switch(GetVarPosType(node, DiffVarName)) {
        case NUMNUM:
            RETURN_AND_DUMP(NUM_(0), "differential of log of two nums is 0\n\n");
        case VARNUM:
            RETURN_AND_DUMP(MUL_(
                                MUL_(
                                    DIV_(
                                        POW_(
                                            LOG_(cL, cR),
                                            NUM_(2)
                                        ),
                                        MUL_(
                                            cR,
                                            LN_(cR)
                                        )
                                    ),
                                    NUM_(-1)
                                ),
                                dR
                            ), "differential of log when var is at the base is (log_x (a))\' = -(log^(-2)_x (a) * x\') / (x * ln(a))\n\n");
        case NUMVAR:
            RETURN_AND_DUMP(DIV_(
                                dR,
                                MUL_(
                                    LN_(cL),
                                    cR
                                )
                            ), "differential of log when at the base is num is (log_a (x))\' = x\' / (x * ln(a))\n\n");
        case VARVAR:
            RETURN_AND_DUMP(DIV_(
                                SUB_(
                                    MUL_(
                                        DIV_(
                                            dR,
                                            cR
                                        ),
                                        LN_(cL)
                                    ),
                                    MUL_(
                                        DIV_(
                                            dL,
                                            cL
                                        ),
                                        LN_(cR)
                                    )
                                ),
                                POW_(
                                    LN_(cL),
                                    NUM_(2)
                                )
                            ), "differential of log when everywhere is var (log_u (v))\' = (v\' / v * u - u\' / u * v) / ln^2(u)\n\n");
    }
    return NULL;
}

Node_t * DiffOpSin(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(MUL_(
                        COS_(NUM_(0), cR),
                        dR
                    ), "differential of sin (sin(x))\' = cos(x) * x\'");
}

Node_t * DiffOpCos(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(MUL_(
                        MUL_(
                            SIN_(NUM_(0), cR),
                            NUM_(-1)
                        ),
                        dR
                    ), "differential of cos (cos(x))\' = -sin(x) * x\'");
}

Node_t * DiffOpTg(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                  const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(DIV_(
                        dR,
                        POW_(
                            COS_(NUM_(0), cR),
                            NUM_(2)
                        )
                    ), "differential of tg (tg(x))\' = x\' / cos^2(x)");
}

Node_t * DiffOpCtg(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(DIV_(
                        dR,
                        POW_(
                            SIN_(NUM_(0), cR),
                            NUM_(2)
                        )
                    ), "differential of ctg (ctg(x))\' = x\' / sin^2(x)");
}

Node_t * DiffOpArcsin(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                      const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(DIV_(
                        dR,
                        POW_(
                            SUB_(NUM_(1), POW_(cR, NUM_(2))),
                            NUM_(0.5)
                        )
                    ), "differential of arcsin is (arcsin(x))' = x' / sqrt(1 - x^2)\n\n");
}

Node_t * DiffOpArccos(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                      const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(MUL_(
                        NUM_(-1),
                        DIV_(
                            dR,
                            POW_(
                                SUB_(NUM_(1), POW_(cR, NUM_(2))),
                                NUM_(0.5)
                            )
                        )
                    ), "differential of arccos is (arccos(x))' = -x' / sqrt(1 - x^2)\n\n");
}

Node_t * DiffOpArctg(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                     const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(DIV_(
                        dR,
                        ADD_(
                            NUM_(1),
                            POW_(cR, NUM_(2))
                        )
                    ), "differential of arctg is (arctg(x))' = x' / (1 + x^2)\n\n");
}

Node_t * DiffOpArcctg(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                      const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(MUL_(
                        NUM_(-1),
                        DIV_(
                            dR,
                            ADD_(
                                NUM_(1),
                                POW_(cR, NUM_(2))
                            )
                        )
                    ), "differential of arcctg is (arcctg(x))' = -x' / (1 + x^2)\n\n");
}

Node_t * DiffOpSh(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                  const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(MUL_(
                        dR,
                        CH_(cR)
                    ), "differential of sh is (sh(x))\' = x\' * ch(x)\n\n");
}

Node_t * DiffOpCh(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                  const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(MUL_(
                        dR,
                        SH_(cR)
                    ), "differential of ch is (ch(x))\' = x\' * sh(x)\n\n");
}

Node_t * DiffOpTh(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                  const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(DIV_(
                        dR,
                        POW_(
                            CH_(cR),
                            NUM_(2)
                        )
                    ), "differential of th is (th(x))\' = x\' / ch^2(x)\n\n");
}

Node_t * DiffOpCth(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                   const char *DiffVarName, bool need_dump) {
    RETURN_AND_DUMP(DIV_(
                        MUL_(
                            NUM_(-1),
                            dR
                        ),
                        POW_(
                            SH_(cR),
                            NUM_(2)
                        )
                    ), "differential of cth is (cth(x))\' = -x\' / sh^2(x)\n\n");
}

Node_t * differentiate(FILE *fp, Node_t *node, size_t *num_of_nodes, 
                       const char *DiffVarName, bool need_dump) {
    sassert(node, ERR_PTR_NULL);

    switch(node->type) {
        case TYPE_NUM: return DiffTypeNum(fp, node, num_of_nodes, DiffVarName, need_dump);
        case TYPE_VAR: return DiffTypeVar(fp, node, num_of_nodes, DiffVarName, need_dump);
        case TYPE_OP:
            switch(node->value.operation) {
                case OP_ADD:    return DiffOpAdd(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_SUB:    return DiffOpSub(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_MUL:    return DiffOpMul(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_DIV:    return DiffOpDiv(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_POW:    return DiffOpPow(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_LN:     return DiffOpLn(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_LOG:    return DiffOpLog(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_SIN:    return DiffOpSin(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_COS:    return DiffOpCos(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_TG:     return DiffOpTg(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_CTG:    return DiffOpCtg(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_ARCSIN: return DiffOpArcsin(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_ARCCOS: return DiffOpArccos(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_ARCTG:  return DiffOpArctg(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_ARCCTG: return DiffOpArcctg(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_SH:     return DiffOpSh(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_CH:     return DiffOpCh(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_TH:     return DiffOpTh(fp, node, num_of_nodes, DiffVarName, need_dump);
                case OP_CTH:    return DiffOpCth(fp, node, num_of_nodes, DiffVarName, need_dump);
                default:        return NULL;
            }
        default:
            return NULL;
    }
}

Node_t * BuildTailorMonomial(const char *DiffVarName, double a, size_t n) {
    Var_t Var = {strdup(DiffVarName), sdbm(DiffVarName), a};
    return  POW_(
                SUB_(
                    NewNode(TYPE_VAR, {.var = Var}, NULL, NULL),
                    NUM_(a)
                ),
                NUM_(n)
            );
}


#define DUMPNODE_IFNEEDDUMP(need_dump, ...) \
    if (need_dump)\
        DUMPNODE(__VA_ARGS__);

DiffTree_t *GetLatexTailorSeries(DiffTree_t *tree, const char *DiffVarName, double a, size_t max_order, bool need_dump) {
    sassert(tree, ERR_PTR_NULL);
    sassert(tree->root, ERR_PTR_NULL);

    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    TreeCtorDiff(TreeCopy);
    TreeCopy->root = CopyNodes(tree->root);

    TreeCtorDiff(tailor);
    tailor->root = NUM_(0);

    for (size_t n = 0; n <= max_order; n++) {
        if (n != 0)
            TreeCopy->root = optimize_and_differentiate(fp, TreeCopy->root, DiffVarName, n, false);
        Node_t *monomial = BuildTailorMonomial(DiffVarName, a, n);

        size_t fact = Factorial(n);
        double result = GetCalculatedAnswer(TreeCopy, TreeCopy->root, sdbm(DiffVarName), a);
        if (isnan(result)) {
            result = NAN;
            add_error(ERR_CALCULATION_INCORRECT, "failed to calculate tailors formula, some operation is invalid "
                                                 "calculating in point: %lf, with accuracy up to: %zu", a, max_order);
            NodeDtorDiff(&monomial);
            break;
        }
        tailor->root = ADD_(tailor->root, 
                            DIV_(
                                MUL_(
                                    NUM_(result), 
                                    CopyNodes(monomial)
                                ), 
                                NUM_(fact)
                            ));
        OptimizeTree(tailor->root, n);
        NodeDtorDiff(&monomial);
    }
    tailor->num_of_nodes = GetNumOfNodes(tailor->root);
    TreeDtorDiff(TreeCopy);

    if (error.is_error == true) {
        print_error(errors_text);
        return NULL;
    }
    
    fprintf(fp, "\\newcommand{\\TaylorSeriesApprox}{f(x) \\approx");
    DumpNodesToLatexRec(tailor->root, fp, false);
    fprintf(fp, "}");
    fclose(fp);

    return tailor;
}
#undef RETURN_AND_DUMP
#undef NUM_
#undef OP_
#undef ADD_
#undef MUL_
#undef LN_
#undef LOG_
#undef DIV_
#undef POW_
#undef SUB_
#undef SIN_
#undef COS_
#undef CH_
#undef SH_
#undef CTH_
#undef TH_
#undef ARCSIN
#undef ARCCOS
#undef ARCTG
#undef ARCCTG
#undef dL
#undef dR
#undef cR
#undef cL

TreeErr MakeTreeFromFile(DiffTree_t *tree, const char * file_name) {
    sassert(tree,       ERR_PTR_NULL);
    sassert(file_name,  ERR_PTR_NULL);

    FILE *fp = fopen(file_name, "r");
    sassert(fp, ERR_PTR_NULL);

    size_t file_size = get_file_size(fp);

    char *file_buffer = (char *) calloc(file_size + 1, sizeof(char));
    char *start_of_file_buffer = file_buffer;
    sassert(file_buffer, ERR_PTR_NULL);

    file_buffer = fgets(file_buffer, file_size, fp);
    fclose(fp);

    int pos = 0;
    int num_of_nodes = 0;
    tree->root = GetG(&file_buffer);
    tree->num_of_nodes = GetNumOfNodes(tree->root);
    if (tree->root == NULL)
        ADD_ERROR_AND_RETURN(ERR_PTR_NULL, "tree is null");
    OptimizeTree(tree->root, 0);

    DUMPTREE(tree, "read tree from file");

    free(start_of_file_buffer);
    return OK;
}

Node_t * optimize_and_differentiate(FILE *fp, Node_t *node, const char *DiffVarName, size_t num_of_derivative, bool need_dump) {
    Node_t * node_copy = CopyNodes(node);
    size_t num_of_nodes = 0;

    Node_t *DiffNode = differentiate(fp, node_copy, &num_of_nodes, DiffVarName, need_dump);
    NodeDtorDiff(&node);
    NodeDtorDiff(&node_copy);

    OptimizeTree(DiffNode, num_of_derivative);
    if (need_dump) {
        fprintf(fp, "\\begin{coloredbox}{green!5!white}{green!60!black}\n"
                    "\\textbf{����� ������������ ����������� ����������� �������� ���������:}\\newline\n");
        INRESIZEBOX(
                    fprintf(fp, "$f^{(%zu)}(x) = ", num_of_derivative);
                    DumpNodesToLatexRec(DiffNode, fp, false);
                    fprintf(fp, "$\n");
        );
        fprintf(fp, "\\end{coloredbox}\n");
    }
    return DiffNode;
}

void PrintLatexFunctionResults(DiffTree_t *tree, DiffTree_t *TailorTree, size_t NthDerivative, double point, double accuracy) {
    sassert(tree,       ERR_PTR_NULL);
    sassert(TailorTree, ERR_PTR_NULL);
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\section{����� ������ �������:}\n");

    INCOLOREDBOX(blue, black,
            fprintf(fp, "\\textbf{����������� �������}$:\\newline\n");
            fprintf(fp, "\\func");
    )
    fprintf(fp, "\n\n");

    
    INCOLOREDBOX(red, black,
        fprintf(fp, "\\textbf{��� ������� �� $o(x - \\TailorPoint)^\\TailorAccuracy}$\\newline\n");
        INRESIZEBOX(
            fprintf(fp, "f(x) \\approx ");
            DumpNodesToLatexRec(TailorTree->root, fp, false);
            fprintf(fp, " + o{(x - \\TailorPoint)}^{\\TailorAccuracy}}\n");
        )
    )
    fprintf(fp, "\n\n");


    for (size_t i = 1; i <= NthDerivative; i++) {
        INCOLOREDBOX(green, black,
            fprintf(fp, "\\textbf{%zu �����������}$:\\newline\n", i);
            fprintf(fp, "\\Derivative%c\n", 'a' + (char) i - 1);
        )
    }

    fclose(fp);
}

DiffTree_t *GetNthDerivative(DiffTree_t * tree, const char *DiffVarName, size_t num, bool need_dump) {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    TreeCtorDiff(TreeCopy);
    TreeCopy->root = CopyNodes(tree->root);
    TreeCopy->num_of_nodes = GetNumOfNodes(TreeCopy->root);
    if (need_dump)
        fprintf(fp, "\\section{����������� ���� �������� (��, �����, ������ �� %zu)}\n"
                    "\\label{sec:derivatives}\n", num);

    for (size_t i = 1; i <= num; i++) {
            int RandomWolf = rand() % NUM_OF_WOLVES + 1;
            if (need_dump) {
                switch (i) {
                    case 1:
                        fprintf(fp, "\\subsection{������ �����������}\n\n"
                                    "\\begin{coloredbox}{purple!5!white}{purple!60!black}\n"
                                    "\\textbf{��������� ������� �1:}\\\\[0.4em]\n"
                                    "\\[\n"
                                    "  \\Derivativea\n"
                                    "\\]\n"
                                    "������������� ��� ��� ������� ���������� � �����\n"
                                    "��������� ����� � �������� ��������, ������� ��������\n"
                                    "\\end{coloredbox}\n\n"
                                    "\\begin{imgbox}{gray!20}{memes/wolf%d.jpg}\n"
                                    "���� ������ ������ ������ � �������� - �������� ��� �����������\n"
                                    "���� ��� ���� ����� ���������� � ��� ��������\n"
                                    "\\end{imgbox}\n\n", RandomWolf);
                        break;
                    case 2:
                        fprintf(fp, "\\subsection{������ �����������}\n\n"
                                    "\\begin{coloredbox}{yellow!5!white}{yellow!60!black}\n"
                                    "\\textbf{��������� ������� �2}\\\\[0.4em]\n"
                                    "\\[\n"
                                    "  \\Derivativeb\n"
                                    "\\]\n"
                                    "������ ������� �������� ��� �� �� �������� ���������, � �� ��������� �������� ���������.\n"
                                    "��� ������ � ���������� ������!!!!\n"
                                    "\\end{coloredbox}\n\n"
                                    "\\begin{coloredbox}{green!5!white}{green!60!black}\n"
                                    "\\textbf{����������� ������.}\\\\[0.4em]\n"
                                    "��, ���������, �� ������������ ���� ��������\n"
                                    "\\end{coloredbox}\n\n");
                        break;
                    case 3:
                        fprintf(fp, "\\subsection{������ �����������}\n\n"
                                    "\\begin{coloredbox}{orange!5!white}{orange!50!black}\n"
                                    "\\textbf{��������� ��������������� ������� �3.}\\\\[0.4em]\n"
                                    "\\[\n"
                                    "  \\Derivativec\n"
                                    "\\]\n"
                                    "������ ������� ��������� �� �������� ��������� �������� ��������� �� �������� ���������\n"
                                    "� ����� ��� ���������?\n"
                                    "\\end{coloredbox}\n\n"
                                    "\\begin{coloredbox}{green!5!white}{green!60!black}\n"
                                    "\\textbf{����������� ������.}\\\\[0.4em]\n");
                        if (TreeCopy->num_of_nodes > 15)
                            fprintf(fp, "������������� � ������������� ��� 39843 �������\n");
                        else
                            fprintf(fp, "��� �����, ���� ������� ��� ������\n");
                        fprintf(fp, "\\end{coloredbox}\n\n"
                                    "\\begin{imgbox}{gray!20}{memes/wolf%d.jpg}\n"
                                    "��������� ����� ���� %zu ����������� �� ��� ����, ����� ������� ������� n-���,\n"
                                    "��� ��������� � ��� �������, � ���, ��� ������������\n"
                                    "\\end{imgbox}\n\n", RandomWolf, i);
                        break;
                    default:
                        fprintf(fp, "\\subsection{%zu �����������}\n\n"
                                    "\\begin{coloredbox}{orange!5!white}{orange!50!black}\n"
                                    "\\textbf{��������� ��������������� ������� �%zu.}\\\\[0.4em]\n"
                                    "\\[\n"
                                    "  \\Derivative%c\n"
                                    "\\]\n"
                                    "�� ��� ��� ������, ������ ��� ����� �����������, �� ���,\n"
                                    "����������� �������?!\n"
                                    "\\end{coloredbox}\n\n"
                                    "\\begin{coloredbox}{green!5!white}{green!60!black}\n"
                                    "\\textbf{����������� ������.}\\\\[0.4em]\n", i, i, 'a' + (char) i - 1);
                        fprintf(fp, "Why Are We here, just to suffer?\n");
                        fprintf(fp, "\\end{coloredbox}\n\n"
                                    "\\begin{imgbox}{gray!20}{memes/wolf%d.jpg}\n"
                                    "�� �� �� ����� �������, �� ��� ������ �� �� ��\n"
                                    "����� ��� ������� � �������� �����������\n"
                                    "\\end{imgbox}\n\n", RandomWolf);
                }
            fprintf(fp, "\\begin{coloredbox}{blue!15!white}{blue!60!black}\n"
                        "\\textbf{��� �� �������� ���� ���������?\\newline���������� ����� �� ���� ������ �� ������!}\n"
                        "\\end{coloredbox}");
            }
        TreeCopy->root = optimize_and_differentiate(fp, TreeCopy->root, DiffVarName, i, need_dump);
        char ch = 'a' + i - 1;
        if (!need_dump) {
            fprintf(fp, "\\newcommand{\\Derivative%c}{\\resizebox{\\minof{\\width}{\\textwidth}}{!}{$\\displaystyle f^{(%zu)}(%s) = ", ch, i, DiffVarName);
            DumpNodesToLatexRec(TreeCopy->root, fp, false);
            fprintf(fp, "$}}\n\n");
        }
    }

    fclose(fp);
    return TreeCopy;
}