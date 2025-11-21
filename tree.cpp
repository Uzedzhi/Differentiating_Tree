#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "dirent.h"

#define ERROR_ADD_DEBUG

#include "../my_libs/sassert.h"
#include "../my_libs/error_manage.h"
#include "tree.h"
#include "treedump.h"
#include "treeverifier.h"
#include "treelatexdump.h"
#include "treeoptimize.h"

error_t error = {};

Node_t * create_node() {
    Node_t * node = (Node_t *) calloc(1, sizeof(Node_t));
    sassert(node, ERR_PTR_NULL);

    return node;
}

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
    if ((*node)->type == TYPE_VAR && (*node)->value.var.name != NULL) {
        free((*node)->value.var.name);
        (*node)->value.var.name = NULL;
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

void skip_line(FILE *fp) {
    sassert(fp, ERR_PTR_NULL);

    int ch = 0;
    while ((ch = getc(fp)) != '\n' && ch != EOF)
        ;
}

void scan_line_without_slashn(char *str) {
    sassert(str, ERR_PTR_NULL);

    if (fgets(str, MAX_STR_SIZE, stdin)) {
        size_t str_len = strlen(str);
        if (str_len && str[str_len - 1] == '\n')
            str[str_len - 1] = '\0';
    }
}

size_t get_file_size(FILE *fp) {
    sassert(fp, ERR_PTR_NULL);

    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    return file_size;
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

TreeErr write_Tree_to_file(DiffTree_t *tree, const char * log_file_name) {
    sassert(tree, ERR_PTR_NULL);

    FILE *fp = fopen(log_file_name, "w");
    sassert(fp,   ERR_PTR_NULL);
    CHECK_FUNC_AND_RET_IF_ERR(TreeVerify(tree));

    fprintf(fp, "num_of_nodes: %zu\n", tree->num_of_nodes);
    write_nodes_to_file(tree->root, fp);

    fclose(fp);
    return OK;
}

size_t sdbm(const char * str) {
    size_t hash = 0;
    int c = 0;

    while ((c = *str++) != '\0') {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

bool is_same(double a, double b) {
    return fabs(a - b) < FLT_ERR;
}

double CalculateTree(DiffTree_t *tree) {
    sassert(tree, ERR_PTR_NULL);

    double answer = GetCalculatedAnswer(tree, tree->root);
    if (answer == NAN) {
        add_error(ERR_CALCULATION_INCORRECT, "answer is incorrect");
    }

    return answer;
}

#define AL GetCalculatedAnswer(tree, cur_node->left)
#define AR GetCalculatedAnswer(tree, cur_node->right)
double GetCalculatedAnswer(DiffTree_t *tree, Node_t *cur_node) {
    size_t hash_cur = 0;
    size_t i = 0;
    bool found = false;
    switch(cur_node->type) {
        case TYPE_OP:
            switch((cur_node->value).operation) {
                case OP_ADD: return AL + AR;
                case OP_DIV: return AL / AR;
                case OP_MUL: return AL * AR;
                case OP_SUB: return AL - AR;
                case OP_POW: return pow(AL, AR);
                case OP_SIN: return sin(AR);
                case OP_COS: return cos(AR);
                case OP_TG:  return tan(AR);
                case OP_CTG: return cos(AR) / sin(AR);
                case OP_LN:  return log(AR);
                case OP_LOG: return log(AR) / log(AL);
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
            if (!found) {
                printf("\nwhat value does %s have? ", cur_node->value.var.name);
                while(scanf("%lf", &(tree->variables[i].value)) != 1) {
                    skip_line(stdin);
                    printf("\n please try again: ");
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

char * get_next_word_in_quotes(int *pos, char *file_buffer) {
    sassert(file_buffer, ERR_PTR_NULL);

    size_t word_length = strchr(file_buffer + *pos, '\"') - file_buffer - *pos;
    char * data = (char *) calloc(word_length + 1, sizeof(char));
    sassert(data, ERR_PTR_NULL);

    strncpy(data, file_buffer + *pos, word_length);
    return data;
}

TypeOp_t GetOperationType(char * str) {
    sassert(str, ERR_PTR_NULL);

    for (size_t i = 0; i < sizeof(AllOperationsTxt) / sizeof(string_t); i++) {
        if (strncmp(AllOperationsTxt[i], str, strlen(AllOperationsTxt[i])) == 0) {
            return (TypeOp_t) i;
        }
    }
    return NOP;
}

TreeErr GetTypeAndValue(DiffTree_t *tree, int *pos, char * buffer, TreeType_t *type, TreeElem_u *value) {
    char NextChar = buffer[*pos];
    
    BEGIN
    if (isdigit(NextChar)) { // is value
        *type = TYPE_NUM;
        (*value).lf = atof(buffer + *pos);
        break;
    }

    (*value).operation = GetOperationType(buffer + *pos); // is some kind of operation
    if ((*value).operation != NOP) {
        *type = TYPE_OP;
        break;
    }

    if (isalpha(NextChar)) { // if accepted symbol and not operation -> its variable
        (*value).var.name   = get_next_word_in_quotes(pos, buffer);
        (*value).var.hash   = sdbm((*value).var.name);
        (*value).var.value  = POISON;
        *type = TYPE_VAR;
        break;
    }

    // if error
    return ERR_INVALID_TYPE;
    END

    (*pos) += strchr(buffer + *pos, '\"') + 1 - buffer - *pos;
    return OK;
    
}

Node_t * NewNode(TreeType_t type, TreeElem_u value, Node_t *left, Node_t *right) {
    Node_t * node = create_node();
    sassert(node, ERR_PTR_NULL);

    node->type  = type;
    node->value = value;
    if (node->type == TYPE_VAR)
        node->value.var.name = strdup(node->value.var.name);
    node->left  = left;
    node->right = right;
    if (node->left != NULL)
        node->left->parent = node;
    if (node->right != NULL)
        node->right->parent = node;
    return node;
}

Node_t * CopyNodes(Node_t *CopyNode) {
    Node_t * node = create_node();
    sassert(node, ERR_PTR_NULL);
    
    node->rank  = CopyNode->rank;
    node->value = CopyNode->value;
    node->type  = CopyNode->type;
    if (node->type == TYPE_VAR)
        node->value.var.name = strdup(node->value.var.name);

    if (CopyNode->left != NULL) {
        node->left  = CopyNodes(CopyNode->left);
        if (node->left != NULL)
            node->left->parent = node;
    }
    if (CopyNode->right != NULL) {
        node->right = CopyNodes(CopyNode->right);
        if (node->right != NULL)
            node->right->parent = node;
    }

    return node;
}

void place_parents(Node_t *node, size_t *num_of_nodes) {
    if (node->left != NULL) {
        (*num_of_nodes)++;
        place_parents(node->left, num_of_nodes);
    }
    if (node->right != NULL) {
        (*num_of_nodes)++;
        place_parents(node->right, num_of_nodes);
    }
}

#define RETURN_AND_DUMP(diff_node, ...) {\
    result_node = diff_node;\
    char str[MAX_STR_SIZE] = {};\
    snprintf(str, MAX_STR_SIZE - 1, __VA_ARGS__);\
    DUMPNODE(node, YES_DIVISION, "before differentiating");\
    DUMPNODE(result_node, NO_DIVISION, __VA_ARGS__);\
    WriteLatexStepOfDifferentiation(node, result_node, DiffVarName, str);\
    return result_node;\
}
#define NUM_(num) NewNode(TYPE_NUM, (TreeElem_u) {.lf  = (double) num}, NULL, NULL)
#define ADD_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {.operation = OP_ADD}, node1, node2)
#define MUL_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {.operation = OP_MUL}, node1, node2)
#define LN_(node1) NewNode(TYPE_OP, (TreeElem_u) {.operation = OP_LN}, NUM_(0), node1)
#define LOG_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {.operation = OP_LOG}, node1, node2)
#define DIV_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {.operation = OP_DIV}, node1, node2)
#define POW_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {.operation = OP_POW}, node1, node2)
#define SUB_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {.operation = OP_SUB}, node1, node2)
#define SIN_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {.operation = OP_SIN}, node1, node2)
#define COS_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {.operation = OP_COS}, node1, node2)
#define dL differentiate(tree, node->left, num_of_nodes, DiffVarName)
#define dR differentiate(tree, node->right, num_of_nodes, DiffVarName)
#define cR CopyNodes(node->right)
#define cL CopyNodes(node->left)

Node_t * differentiate(DiffTree_t *tree, Node_t * node, size_t *num_of_nodes, const char * DiffVarName) {
    sassert(node, ERR_PTR_NULL);
    Node_t *result_node = NULL;
    switch(node->type) {
        case TYPE_NUM:
            RETURN_AND_DUMP(NUM_(0), "differential of number is 0");
        case TYPE_OP:
            switch(node->value.operation) {
                case OP_ADD:
                    RETURN_AND_DUMP(ADD_(dL, dR), "differential of addition (x + y)\' = x\' + y\'");
                case OP_DIV:
                    RETURN_AND_DUMP(DIV_(
                                        SUB_(
                                            MUL_(dL, cR),
                                            MUL_(cL, dR)
                                        ),
                                        MUL_(cR, cR)
                                    ), "differential of division (x * y)\' = x * y\' + x\' * y");
                case OP_MUL:
                    RETURN_AND_DUMP(ADD_(
                                        MUL_(dL, cR),
                                        MUL_(cL, dR)
                                    ), "differential of multiplication (x * y)' = x * y' + x' * y");
                case OP_SUB:
                    RETURN_AND_DUMP(SUB_(dL, dR), "differential of difference (x - y)\' = x\' s- y\'");
                case OP_SIN:
                    RETURN_AND_DUMP(MUL_(
                                        COS_(NUM_(0), cR),
                                        dR
                                    ), "differential of sin (sin(x))\' = cos(x)  * x\'");
                case OP_COS:
                    RETURN_AND_DUMP(MUL_(
                                        MUL_(
                                            SIN_(NUM_(0), cR),
                                            NUM_(-1)
                                        ),
                                        dR
                                    ), "differential of cos (cos(x))\' = -sin(x)  * x\'");
                case OP_TG:
                    RETURN_AND_DUMP(DIV_(
                                        dR,
                                        MUL_(
                                            COS_(NUM_(0), cR),
                                            COS_(NUM_(0), cR)
                                        )
                                    ), "differential of tg (tg(x))\' = x\' / cos^2(x)\'");
                case OP_CTG:
                    RETURN_AND_DUMP(DIV_(
                                        dR,
                                        MUL_(
                                            SIN_(NUM_(0), cR),
                                            SIN_(NUM_(0), cR)
                                        )
                                    ), "differential of ctg (ctg(x))\' = x\' / sin^2(x)\'");
                case OP_POW:
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
                case OP_LN:
                    RETURN_AND_DUMP(DIV_(
                                        dR,
                                        cR
                                    ), "differential of ln is (ln(x))\' = x\' / x\n\n");
                case OP_LOG:
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
            }
        case TYPE_VAR:
            if (strcmp(node->value.var.name, DiffVarName) == 0) {
                RETURN_AND_DUMP(NUM_(1), "differential of variable, which we are differentiating with is 1");
            }
            else {
                RETURN_AND_DUMP(NUM_(0), "differential of variable, which we are NOT differentiating with is 0");
            }
        
    }

    return NULL;
}

#undef VAR_
#undef NUM_
#undef ADD_
#undef MUL_
#undef DIV_
#undef SUB_
#undef dL
#undef dR
#undef cR
#undef cL

DiffTree_t * dT(Node_t *node, const char * DiffVarName) {
    sassert(node, ERR_PTR_NULL);

    size_t num_of_nodes = 0;
    TreeCtorDiff(tree);

    tree->root = differentiate(tree, node, &num_of_nodes, DiffVarName);
    tree->num_of_nodes = GetNumOfNodes(tree->root);
    DUMPTREE(tree, "differentiated tree");
    return tree;
}

Node_t * read_node(DiffTree_t *tree, int * pos, int *num_of_nodes, char * buffer) {
    sassert(pos, ERR_PTR_NULL);

    if (buffer[*pos] == '(') {
        Node_t * node = create_node();
        (*pos) += 2; // skip ("

        if (GetTypeAndValue(tree, pos, buffer, &(node->type), &(node->value)) != OK) {
            add_error(ERR_INVALID_TYPE, "place of error: %s", buffer);
            return NULL;
        }
        node->left  = read_node(tree, pos, num_of_nodes, buffer);
        if (node->left != NULL)
            node->left->parent = node;
        node->right = read_node(tree, pos, num_of_nodes ,buffer);
        (*num_of_nodes)++;
        if (node->right != NULL) 
            node->right->parent = node;
        
        (*pos)++; // skip )
        return node;
    }
    else if (strncmp(buffer + *pos, "nilnil", 6) == 0) {
        (*pos) += 6;
        return NULL;
    }

    return NULL;
}

TreeErr MakeTreeFromFile(DiffTree_t *tree, const char * file_name) {
    sassert(tree,       ERR_PTR_NULL);
    sassert(file_name,  ERR_PTR_NULL);

    FILE *fp = fopen(file_name, "r");
    sassert(fp, ERR_PTR_NULL);

    size_t file_size = get_file_size(fp);

    char *file_buffer = (char *) calloc(file_size + 1, sizeof(char));
    sassert(file_buffer, ERR_PTR_NULL);

    size_t actually_read = fread(file_buffer, sizeof(char), file_size, fp);
    if (actually_read == 0)
        ADD_ERROR_AND_RETURN(ERR_FILE_INVALID, "read 0 bytes in file %s", file_name);
    fclose(fp);

    int pos = 0;
    int num_of_nodes = 0;
    tree->root = read_node(tree, &pos, &num_of_nodes, file_buffer);
    tree->num_of_nodes = num_of_nodes;

    DUMPTREE(tree, "read tree from file");

    free(file_buffer);
    return OK;
}

void print_help() {
    printf("you need to specify file name with --file-name");
}

#define STARTTXTDUMPS() \
    PrintSiteHeader();\
    WriteLatexHeader();

#define FINISHTXTDUMPS()\
    PrintSiteToes();\
    WriteLatexToes();



int main(int argc, char * argv[]) {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    if (argc != 2 || strncmp(argv[1], "--", 2) != 0) {
        print_help();
        return OK;
    }
    STARTTXTDUMPS();

    char * file_name = argv[1] + 2;
    TreeCtorDiff(tree);
    MakeTreeFromFile(tree, file_name);
    if (error.is_error == true)
        print_error(error, errors_text);

    WriteLatexDifferential(tree->root, "x");
    DiffTree_t * treediff = dT(tree->root, "x");
    WriteLatexResult(treediff->root, "x", "Результат до оптимизации:");

    bool is_folded = true;
    TreeErr return_val = TREE_NFOLDED;
    while (is_folded == true) {
        is_folded = false;
        TreeErr return_val_1_optimization = FoldConstantsInNode(treediff->root, &is_folded);
        WriteLatexResult(treediff->root, "x", "Результат после оптимизации сверткой констант:");
        DUMPTREE(treediff, "after const folding optimization");

        TreeErr return_val_2_optimization = RemoveUnnecessary(treediff->root, &is_folded);
        WriteLatexResult(treediff->root, "x", "Результат после оптимизации удаления ненужного:");
        DUMPTREE(treediff, "after removing unnecessary things optimization");

        if (ISVALUEERROR(return_val_1_optimization) || ISVALUEERROR(return_val_2_optimization))
            break;
    }

    double answer = CalculateTree(treediff);
    printf("answer is %lf\n", answer);

    TreeDtorDiff(tree);
    TreeDtorDiff(treediff);

    FINISHTXTDUMPS();
    MakePdfFromLatex();

    if (error.is_error == true) {
        print_error(error, errors_text);
        return error.recently_added_error_int;
    }
    return 0;
}