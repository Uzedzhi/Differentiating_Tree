#ifndef TREE_H
#define TREE_H

#include <stdio.h>

#include "../my_libs/sassert.h"
#include "../my_libs/error_manage.h"

typedef const char * const string_t;

extern error_t error;
const double POISON = 0xDEEEEED;
static int count_nodes = 0;
const size_t ARR_INIT_SIZE = 10;
const size_t MAX_STR_SIZE = 500;
const size_t MAX_RECURSION_LIMIT = 10000;
const size_t MAX_RETURN_ARR_LIMIT = 1000;
const size_t MAX_YESNO_SIZE       = 6;
const double FLT_ERR = 1e-6;
const double CONST_E = 2.7128;
string_t tree_txt_dump_dir_name = "treeintxt";
string_t most_recent_dir_name = "lastgraph";
string_t dump_site_name = "graph.html";
string_t tree_data_file = "treetxtdump.txt";
string_t dump_graph_txt_file_name = "graph/graph.txt";
string_t DumpLatexFileName = "treelatexdump.txt";
string_t DumpLatexFileNameEncoded = "treelatexdumpEncoded.txt";
string_t DumpLatexPdfFileName = "latex.pdf";
string_t command_desc[] = {"Before operation happened", "after insertion happened", "after read from file"};
const size_t command_count = sizeof(command_desc) / sizeof(string_t);

#define INIT_ERRORS(n)\
    n(OK                                        ,   0)\
    n(ERR_INVALID_RELATION_WITH_DAD_AND_SON     ,   1)\
    n(ERR_INVALID_SIZE                          ,   2)\
    n(ERR_INCORRECT_CONNECTIONS                 ,   3)\
    n(ERR_CMD_INVALID                           ,   4)\
    n(ERR_PTR_NULL                              ,   5)\
    n(ERR_NODE_POINTS_TO_LOWER_RANK             ,   6)\
    n(ERR_NO_PARENT                             ,   7)\
    n(ERR_INVALID_HEADER                        ,   8)\
    n(ERR_INVALID_TYPE                          ,   9)\
    n(ERR_CALCULATION_INCORRECT                 ,   10)\
    n(ERR_FILE_INVALID                          ,   11)\
    n(ERR_FOLD_DIVISION_BY_ZERO                 ,   12)\
    n(TREE_NFOLDED                              ,   13)\
    n(TREE_FOLDED                               ,   14)\
    n(ERR_FOLD_INVALID_ARGUMENT                 ,   15)

#define ENUM_ERRORS_INIT(ERR_CODE, VALUE) \
    ERR_CODE = VALUE,

#define STR_ERRORS_INIT(ERR_CODE, VALUE) \
    #ERR_CODE, 

string_t errors_text[] = {INIT_ERRORS(STR_ERRORS_INIT)};

enum TreeErr {
    INIT_ERRORS(ENUM_ERRORS_INIT)
};

enum TreeNeedDiv {
    YES_DIVISION = 1,
    NO_DIVISION  = 0,
};

enum TreeCmd {
    BEFORE          = 0,
    INSERT          = 1,
    READ            = 2,
};

#define INIT_OPERATIONS(n)\
    n("*", "MUL", OP_MUL, 0)\
    n("+", "ADD", OP_ADD, 1)\
    n("-", "SUB", OP_SUB, 2)\
    n("/", "DIV", OP_DIV, 3)\
    n("sin", "SIN", OP_SIN, 4)\
    n("cos", "COS", OP_COS, 5)\
    n("tg", "TG", OP_TG, 6)\
    n("ctg", "CTG", OP_CTG, 7)\
    n("pow", "POW", OP_POW, 8)\
    n("ln", "LN", OP_LN, 9)\
    n("log", "LOG", OP_LOG, 10)
#define ENUM_OP_INIT(STR, DUMPSTR, TYPENAME, VALUE) \
    TYPENAME = VALUE,
#define STR_OP_INIT(STR, ...) \
    STR,
#define STR_OP_DUMP_INIT(STR, DUMPSTR, ...)\
    DUMPSTR,

enum TypeOp_t {
    NOP = -1,
    INIT_OPERATIONS(ENUM_OP_INIT)
};

string_t AllOperationsTxt[] = {INIT_OPERATIONS(STR_OP_INIT)};
string_t AllOperationsDumpTxt[] = {INIT_OPERATIONS(STR_OP_DUMP_INIT)};
string_t AllValueTypesTxt[] = {"OPER", "VAR", "NUM"};

enum TreeType_t {
    NTYPE       = -1,
    TYPE_OP     = 0,
    TYPE_VAR    = 1,
    TYPE_NUM    = 2,
};

enum dirType {
    RIGHT_DIRECTION = 0,
    LEFT_DIRECTION  = 1,
    NO_DIRECTION    = 2
};

typedef struct Var_t {
    char *name;
    size_t hash;
    double value;
} Var_t;

union TreeElem_u {
    double lf;
    TypeOp_t operation;
    Var_t var;
};

typedef char * TreeElem_t;
typedef struct Node_t {
    TreeType_t type;
    TreeElem_u value;
    Node_t* parent;
    Node_t* left;
    Node_t* right;
    int rank;
} Node_t;

typedef struct DiffTree_t {
    size_t num_of_nodes;
    Node_t* root;
    Var_t * variables;
} DiffTree_t;

#define ISVALUEERROR(value) value != TREE_FOLDED && value != TREE_NFOLDED && value != OK

#ifndef NDEBUG
#define TreeCtorDiff(tree_name) \
    DiffTree_t* tree_name = (DiffTree_t *) calloc(1, sizeof(DiffTree_t));\
    TreeCtorDiff_internal(tree_name);\
    DUMPTREE(tree, "initializing tree with name" #tree_name "at %s:%d, %s", __FILE__, __LINE__, __func__);
#else
#define TreeCtorDiff(tree_name) \
    DiffTree_t* tree_name = (DiffTree_t *) calloc(1, sizeof(DiffTree_t));\
    TreeCtorDiff_internal(tree_name);
#endif

void TreeDtorDiff(DiffTree_t *tree);
void NodeDtorDiff(Node_t **node);
size_t get_file_size(FILE *fp);
void scan_line_without_slashn(char *str);
Node_t * create_node();
void skip_line(FILE *fp);
size_t sdbm(const char * str);
Node_t * CopyNodes(Node_t *CopyNode);
TreeErr MakeTreeFromFile(DiffTree_t *tree, const char * file_name);
Node_t * read_node(DiffTree_t *tree, int * pos, int *num_of_nodes, char * buffer);
TreeErr GetTypeAndValue(DiffTree_t *tree, int *pos, char * buffer, TreeType_t *type, TreeElem_u *value);
void TreeCtorDiff_internal(DiffTree_t * tree);
double GetCalculatedAnswer(DiffTree_t *tree, Node_t *cur_node);
TreeErr write_Tree_to_file(DiffTree_t *tree, const char * log_file_name);

#endif // TREE_H