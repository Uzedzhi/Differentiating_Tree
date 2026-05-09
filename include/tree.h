#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "../my_libs/sassert.h"
#include "../my_libs/error_manage.h"

typedef const char * const string_t;

extern error_t error;
const double POISON = 0xDEEEEED; // ������ �� ������
static int count_nodes = 0;
const size_t ARR_INIT_SIZE = 10;
const size_t MAX_STR_SIZE = 500;
const double MAX_POSSIBLE_RESULT = 0xfffffff;
const size_t MAX_RECURSION_LIMIT = 10000;
const size_t MAXNUM_OF_STEPS     = 30000;
const size_t MAX_RETURN_ARR_LIMIT = 1000;
const size_t NUM_OF_OPERATIONS    = 18;
const size_t NUM_OF_STD_OPERATIONS = 4;
const size_t NUM_OF_MENU_ITEMS      = 8;
const size_t RANDTYPES_MAX          = 10;
const size_t MAX_RAND_DEEP_TREE     = 6;
const size_t RANDNUM_MAX            = 30;
const size_t MAX_YESNO_SIZE       = 6;
const size_t MAX_FUNCGRAPH_VALUE  = 0xff;
const size_t MAX_FUNCGRAPH_GAP    = 0xff;
const double STD_TEILORPOINT = 0;
const size_t STD_TEILORACCURACY = 3;
const size_t STD_NTHDERIVATIVE = STD_TEILORACCURACY;
const double STD_TANGENTPOINT   = STD_TEILORPOINT;
const size_t NUM_OF_WOLVES      = 4;
const double FLT_ERR = 1e-12;
const double CONST_E = 2.7128;
string_t tree_txt_dump_dir_name     = "treeintxt";
string_t most_recent_dir_name       = "lastgraph";
string_t dump_site_name             = "htmldump/graph.html";
string_t tree_data_file             = "treetxtdump.txt";
string_t dump_graph_txt_file_name   = "graph/graph.txt";
string_t meme_expression_txt        = "memeexpression.txt";
string_t DumpLatexFileName          = "latexdump/treelatexdump.txt";
string_t DumpLatexFileNameEncoded   = "latexdump/treelatexdumpEncoded.txt";
string_t command_desc[]             = {"Before operation happened"
                                     , "after insertion happened"
                                     , "after read from file"};
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

#define STR(x)  #x
#define XSTR(x) STR((x))

string_t errors_text[] = {INIT_ERRORS(STR_ERRORS_INIT)};

typedef enum {
    INIT_ERRORS(ENUM_ERRORS_INIT)
} TreeErr;

enum TreeNeedDiv {
    YES_DIVISION = 1,
    NO_DIVISION  = 0,
};

struct ylimits {
    double func_yfrom;
    double func_yto;
    double tailor_yfrom;
    double tailor_yto;
    double tangent_yfrom;
    double tangent_yto;
};

enum TreeCmd {
    BEFORE          = 0,
    INSERT          = 1,
    READ            = 2,
};

#define INIT_OPERATIONS(n)\
    n("*",      "MUL",      OP_MUL,             0)\
    n("+",      "ADD",      OP_ADD,             1)\
    n("-",      "SUB",      OP_SUB,             2)\
    n("/",      "DIV",      OP_DIV,             3)\
    n("sin",    "SIN",      OP_SIN,             4)\
    n("cos",    "COS",      OP_COS,             5)\
    n("tg",     "TG",       OP_TG,              6)\
    n("ctg",    "CTG",      OP_CTG,             7)\
    n("pow",    "POW",      OP_POW,             8)\
    n("ln",     "LN",       OP_LN,              9)\
    n("log",    "LOG",      OP_LOG,             10)\
    n("sh",     "SH",       OP_SH,              11)\
    n("ch",     "CH",       OP_CH,              12)\
    n("cth",    "CTH",      OP_CTH,             13)\
    n("th",     "TH",       OP_TH,              14)\
    n("arcsin", "ARCSIN",   OP_ARCSIN,          15)\
    n("arccos", "ARCCOS",   OP_ARCCOS,          16)\
    n("arctg",  "ARCTG",    OP_ARCTG,           17)\
    n("arcctg", "ARCCTG",   OP_ARCCTG,          18)
#define ENUM_OP_INIT(STR, DUMPSTR, TYPENAME, VALUE) \
    TYPENAME = VALUE,
#define STR_OP_INIT(STR, ...) \
    STR,
#define STR_OP_DUMP_INIT(STR, DUMPSTR, ...)\
    DUMPSTR,
#define HASH_OP_INIT(STR, ...) \
    constexpr_sdbm(STR), 

#define INIT_CONSTANTS(n) \
    n("e", M_E)\
    n("\\pi", M_PI)

#define HASH_CONST_INIT(STR, ...) \
    constexpr_sdbm(STR), 
#define VALUES_CONST_INIT(SRT, VALUE) \
    VALUE, 
#define STR_CONST_INIT(STR, ...) \
    STR,

enum TypeOp_t {
    NOP = -1,
    INIT_OPERATIONS(ENUM_OP_INIT)
};

constexpr size_t constexpr_sdbm(const char * str) {
    size_t hash = 0;
    int c = 0;

    while ((c = *str++) != '\0') {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

string_t AllOperationsTxt[]             = {INIT_OPERATIONS(STR_OP_INIT)};
constexpr size_t AllOperationsHash[]    = {INIT_OPERATIONS(HASH_OP_INIT)};
string_t AllOperationsDumpTxt[]         = {INIT_OPERATIONS(STR_OP_DUMP_INIT)};
string_t AllValueTypesTxt[]             = {"OPER", "VAR", "NUM"};
string_t AllConstantStr[]               = {INIT_CONSTANTS(STR_CONST_INIT)};
constexpr size_t AllConstantsHash[]     = {INIT_CONSTANTS(HASH_CONST_INIT)};
const string_t AllRandomStr[]           = {"�������� ���"
                                         , "���� �� ����, �� �������� ���"
                                         , "������ ��������� ���������� ��������"
                                         , "������� ���������� ������ ����������� ����������� � ���� ����������� �������� � �������"
                                         , "�������� ���������� ���", "��������� ����������� ���������� ������, "
                                        };
const double AllConstantsValues[]       = {INIT_CONSTANTS(VALUES_CONST_INIT)};
const size_t NUM_OF_CONST    = sizeof(AllConstantsHash) / sizeof(AllConstantsHash[1000-7]);
const size_t NUM_OF_RAND_STR = sizeof(AllRandomStr) / sizeof(AllRandomStr[1000-7]);

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
    DUMPTREE(tree_name, "initializing tree with name" #tree_name "at %s:%d, %s", __FILE__, __LINE__, __func__);
#else
#define TreeCtorDiff(tree_name) \
    DiffTree_t* tree_name = (DiffTree_t *) calloc(1, sizeof(DiffTree_t));\
    TreeCtorDiff_internal(tree_name);
#endif

void        TreeDtorDiff(DiffTree_t *tree);
void        NodeDtorDiff(Node_t **node);
void PrintLatexFunctionResults(DiffTree_t *tree, DiffTree_t *TailorTree, size_t NthDerivative, double point, double accuracy);
void        TreeCtorDiff_internal(DiffTree_t * tree);
DiffTree_t *GetNthDerivative(DiffTree_t * tree, const char *DiffVarName, size_t num, bool need_dump);
DiffTree_t *GetTangentTree(DiffTree_t *tree, DiffTree_t *DiffTree, double a, const char * VarName);
double      GetAnswerNewtonsMethod(DiffTree_t * func, const char * VarName);
Node_t *    differentiate(FILE *fp, Node_t * node, size_t *num_of_nodes, const char * DiffVarName, bool need_dump);
Node_t *    optimize_and_differentiate(FILE *fp, Node_t *node, const char *DiffVarName, size_t num_of_derivative, bool need_dump);
Node_t *    BuildTailorMonomial(const char *DiffVarName, double a, size_t n);
Node_t *    read_node(DiffTree_t *tree, int * pos, int *num_of_nodes, char * buffer);
DiffTree_t *GetLatexTailorSeries(DiffTree_t *tree, const char *DiffVarName, double a, size_t max_order, bool need_dump);
TreeErr     MakeTreeFromFile(DiffTree_t *tree, const char * file_name);
double      GetCalculatedAnswer(DiffTree_t *tree, Node_t *cur_node, size_t KnownVarHash, double KnownVarValue);
int         write_Tree_to_file(DiffTree_t *tree, const char * log_file_name);

#endif // TREE_H