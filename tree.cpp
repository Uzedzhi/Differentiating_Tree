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

error_t error = {};

Node_t * create_node() {
    Node_t * node = (Node_t *) calloc(1, sizeof(Node_t));
    sassert(node, ERR_PTR_NULL);

    return node;
}

void TreeCtorDiff_internal(DiffTree_t * tree) {
    sassert(tree, ERR_PTR_NULL);

    tree->root = create_node();
    tree->num_of_nodes = 1;
}

void NodeDtorDiff(Node_t **node) {
    sassert(node, ERR_PTR_NULL);

    if (*node != NULL && (*node)->left != NULL)
        NodeDtorDiff(&(*node)->left);
    if (*node != NULL && (*node)->right != NULL)
        NodeDtorDiff(&(*node)->right);
    if ((*node)->type == TYPE_VAR && (*node)->value.var.name != NULL)
        free((*node)->value.var.name);
    free(*node);
    *node = NULL;
}
void TreeDtorDiff(DiffTree_t* tree) {
    if (tree != NULL) {
        if (tree->root != NULL) {
            NodeDtorDiff(&(tree->root));
        }
        free(tree->root);
        free(tree);
    }

    print_site_toes();
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

static unsigned long sdbm(const char * str) {
    unsigned long hash = 0;
    int c = 0;

    while ((c = *str++) != '\0') {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

bool is_same(double a, double b) {
    return abs(a - b) < FLT_ERR;
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
    switch(cur_node->type) {
        case TYPE_OP:
            switch((cur_node->value).operation) {
                case OP_ADD: return AL + AR;
                case OP_DIV: return AL / AR;
                case OP_MUL: return AL * AR;
                case OP_SUB: return AL - AR;
            }
            break;
        case TYPE_NUM:
            return (cur_node->value).lf;
        case TYPE_VAR:
            hash_cur = sdbm((cur_node->value).var.name);
            if (is_same(cur_node->value.var.value, POISON)) {
                printf("\nwhat value does %s have? ", cur_node->value.var.name);
                while(scanf("%lf", &(cur_node->value.var.value)) != 1) {
                    printf("\n please try again: ");
                }
            }
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

    size_t word_length = strchr(file_buffer, '\"') - file_buffer;
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

    (*pos) += strchr(buffer, '\"') + 1 - buffer;
    return OK;
    
}

Node_t * NewNode(TreeType_t type, TreeElem_u value, Node_t *left, Node_t *right) {
    Node_t * node = create_node();
    sassert(node, ERR_PTR_NULL);

    node->type  = type;
    node->value = value;
    node->left  = left;
    node->right = right;

    return node;
}

Node_t * CopyNodes(Node_t *CopyNode) {
    Node_t * node = create_node();
    sassert(node, ERR_PTR_NULL);
    
    node->rank  = CopyNode->rank;
    node->value = CopyNode->value;
    node->type  = CopyNode->type;

    if (CopyNode->left != NULL)
        node->left  = CopyNodes(CopyNode->left);
    if (CopyNode->right != NULL)
        node->right = CopyNodes(CopyNode->right);

    return node;
}

void place_parents(Node_t *node, size_t *num_of_nodes) {
    if (node->left != NULL) {
        node->left->parent = node;
        (*num_of_nodes)++;
        place_parents(node->left, num_of_nodes);
    }
    if (node->right != NULL) {
        node->right->parent = node;
        (*num_of_nodes)++;
        place_parents(node->right, num_of_nodes);
    }
}

DiffTree_t * dT(Node_t *node, const char * DiffVarName) {
    sassert(node, ERR_PTR_NULL);

    size_t num_of_nodes = 0;
    TreeCtorDiff(tree);

    tree->root = differentiate(node, DiffVarName);

    place_parents(tree->root, &num_of_nodes);
    tree->num_of_nodes = num_of_nodes + 1;
    
    create_tree_graph(tree);
    print_to_html(tree, INSERT, 0);
    return tree;
}

#define ONE_ NewNode(TYPE_NUM, (TreeElem_u) {1.0}, NULL, NULL)
#define ZERO_ NewNode(TYPE_NUM, (TreeElem_u) {0.0}, NULL, NULL)
#define ADD_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {OP_ADD}, node1, node2)
#define MUL_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {OP_MUL}, node1, node2)
#define DIV_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {OP_DIV}, node1, node2)
#define SUB_(node1, node2) NewNode(TYPE_OP, (TreeElem_u) {OP_SUB}, node1, node2)
#define dL differentiate(node->left, DiffVarName)
#define dR differentiate(node->right, DiffVarName)
#define cR CopyNodes(node->left)
#define cL CopyNodes(node->left)

Node_t * differentiate(Node_t * node, const char * DiffVarName) {
    sassert(node, ERR_PTR_NULL);
    switch(node->type) {
        case TYPE_NUM:
            return ZERO_;
        case TYPE_OP:
            switch((node->value).operation) {
                case OP_ADD:
                    return ADD_(dL, dR);
                case OP_DIV:
                    return NULL;
                case OP_MUL:
                    return ADD_(MUL_(dL, cR), MUL_(cL, dR));
                case OP_SUB:
                    return SUB_(dL, dR);
            }
        case TYPE_VAR:
            if (strcmp(node->value.var.name, DiffVarName) == 0)
                return ONE_;
            else
                return ZERO_;
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
    char *ptr_to_start_of_file_buffer = file_buffer;
    sassert(file_buffer, ERR_PTR_NULL);

    size_t actually_read = fread(file_buffer, sizeof(char), file_size, fp);
    if (actually_read == 0)
        ADD_ERROR_AND_RETURN(ERR_FILE_INVALID, "read 0 bytes in file %s", file_name);
    fclose(fp);

    int pos = 0;
    int num_of_nodes = 0;
    tree->root = read_node(tree, &pos, &num_of_nodes, file_buffer);
    tree->num_of_nodes = num_of_nodes;

    create_tree_graph(tree);
    print_to_html(tree, READ, 0);

    free(file_buffer);
    return OK;
}

void print_help() {
    printf("you need to specify file name with --file-name");
}

int main(int argc, char * argv[]) {
    if (argc != 2 || strncmp(argv[1], "--", 2) != 0) {
        print_help();
        return OK;
    }

    char * file_name = argv[1] + 2;

    TreeCtorDiff(tree);
    MakeTreeFromFile(tree, file_name);
    if (error.is_error == true)
        print_error(error, errors_text);

    DiffTree_t * treediff = dT(tree->root, "x");
    TreeDtorDiff(tree);
    TreeDtorDiff(treediff);
    return 0;
}