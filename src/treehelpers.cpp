#include <ctype.h>

#include "tree.h"
#include "treedump.h"
#include "treelatexdump.h"
#include "treehelpers.h"

char * get_next_word_in_quotes(int *pos, char *file_buffer) {
    sassert(file_buffer,    ERR_PTR_NULL);
    sassert(pos,            ERR_PTR_NULL);

    size_t word_length = strchr(file_buffer + *pos, '\"') - file_buffer - *pos;
    char * data = (char *) calloc(word_length + 1, sizeof(char));
    sassert(data, ERR_PTR_NULL);

    strncpy(data, file_buffer + *pos, word_length);
    return data;
}

size_t sdbm(const char * str) {
    size_t hash = 0;
    int c = 0;

    while ((c = *str++) != '\0') {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
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

TypeOp_t GetOperation(char *expression) {
    for (size_t i = 0; i < NUM_OF_OPERATIONS; i++) {
        if (strncmp(expression, AllOperationsTxt[i], strlen(AllOperationsTxt[i])) == 0) {
            return (TypeOp_t) i;
        }
    }
    return NOP;
}

char * get_var_from_buffer(char * expression) {
    char * var_name = (char *) calloc(MAX_STR_SIZE, sizeof(char));
    sassert(var_name, ERR_PTR_NULL);

    size_t pos = 0;
    while (isalpha(*expression)) {
        var_name[pos++] = *expression;
        expression++;
    }

    return var_name;
}

Node_t * GetVar(char **expression) {
    char * var_name = get_var_from_buffer(*expression);

    Var_t var = {var_name, sdbm(var_name), POISON};
    Node_t *value = NewNode(TYPE_VAR, {.var = var}, NULL, NULL);
    (*expression) += strlen(var_name);
    
    return value;
}

Node_t *GetOper(char **expression) {
    TypeOp_t operation = GetOperation(*expression);

    (*expression) += strlen(AllOperationsTxt[operation]);
    (*expression)++;
    SkipSpaces(expression);
    Node_t *value = NewNode(TYPE_OP, {.operation = operation}, 
                    NewNode(TYPE_NUM, {.lf = 0.0}, NULL, NULL), 
                    GetE(expression)
                );
    SkipSpaces(expression);
    (*expression)++;

    return value;
}

Node_t *GetNum(char **expression) {
    double value = 0.0;

    while (**expression >= '0' && **expression <= '9') {
       value = value * 10 + **expression - '0';
       (*expression)++;
    }

    return NewNode(TYPE_NUM, {.lf = value}, NULL, NULL);
}

Node_t * GetExpr(char **expression) {
    Node_t *value = NULL;
    TypeOp_t operation = GetOperation(*expression);

    if (**expression >= '0' && **expression <= '9') {
        value = GetNum(expression);
    }
    else if (operation != NOP)
        value = GetOper(expression);
    else
        value = GetVar(expression);
    return value;
}


void SkipSpaces(char ** expression) {
    while (isspace(**expression))
        (*expression)++;
}

Node_t * GetP(char **expression) {
    if (**expression == '(') {
        (*expression)++;
        SkipSpaces(expression);

        Node_t * value = GetE(expression);
        SkipSpaces(expression);
        if (**expression != ')') {
            fprintf(stderr, "SYNTAX ERROR AT SPOT: %s\n", *expression);
            return NULL;
        }

        (*expression)++;
        return value;
    }
    return GetExpr(expression);
}

Node_t * GetM(char **expression) {
    Node_t * value = GetP(expression);
    SkipSpaces(expression);

    while (**expression == '^') {
        (*expression)++;
        SkipSpaces(expression);

        Node_t * value2 = GetP(expression);
        SkipSpaces(expression);
        value = NewNode(TYPE_OP, {.operation = OP_POW}, value, value2);
    }
    return value;
}

Node_t * GetT(char **expression) {
    Node_t * value = GetM(expression);
    SkipSpaces(expression);

    while (**expression == '*' || **expression == '/') {
        char op = **expression;
        (*expression)++;
        SkipSpaces(expression);

        Node_t * value2 = GetM(expression);
        SkipSpaces(expression);
        if (op == '*')
            value = NewNode(TYPE_OP, {.operation = OP_MUL}, value, value2);
        else
            value = NewNode(TYPE_OP, {.operation = OP_DIV}, value, value2);
    }
    return value;
}

Node_t * GetE(char **expression) {
    Node_t * value = GetT(expression);
    SkipSpaces(expression);

    while (**expression == '+' || **expression == '-') {
        char op = **expression;
        (*expression)++;
        SkipSpaces(expression);

        Node_t * value2 = GetT(expression);
        SkipSpaces(expression);
        if (op == '+')
            value = NewNode(TYPE_OP, {.operation = OP_ADD}, value, value2);
        else
            value = NewNode(TYPE_OP, {.operation = OP_SUB}, value, value2);
    }
    return value;
}

Node_t * GetG(char **expression) {
    Node_t * value = GetE(expression);

    if (**expression != '\0') {
        fprintf(stderr, "1 SYNTAX ERROR AT SPOT: <%s>\n", *expression);
        return NULL;
    }

    return value;
}

int GetUserAnswer(const char * StrIfIncorrect, int bottom, int top) {
    int answer = 0;
    int num_scanned = scanf("%d", &answer);

    while (answer < bottom || answer > top) {
        printf("%s", StrIfIncorrect);
        num_scanned = scanf("%d", &answer);
        skip_line(stdin);
    }
    return answer;
}

void print_help() {
    printf("you need to specify file name with --file-name");
}

size_t Factorial(size_t n) {
    size_t res = 1;
    for (size_t i = 2; i <= n; i++)
        res *= i;
    return res;
}

size_t get_file_size(FILE *fp) {
    sassert(fp, ERR_PTR_NULL);

    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    return file_size;
}

Node_t * create_node() {
    Node_t * node = (Node_t *) calloc(1, sizeof(Node_t));
    sassert(node, ERR_PTR_NULL);

    return node;
}

bool is_same(double a, double b) {
    return fabs(a - b) < FLT_ERR;
}

TypeOp_t GetOperationType(char * str) {
    sassert(str, ERR_PTR_NULL);
    int pos = 0;
    char * word = get_next_word_in_quotes(&pos, str);
    size_t str_hash = sdbm(word);

    for (size_t i = 0; i < NUM_OF_OPERATIONS; i++) {
        if (str_hash == AllOperationsHash[i]) {
            free(word);
            return (TypeOp_t) i;
        }
    }
    free(word);
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

    char *next_word =  get_next_word_in_quotes(pos, buffer);
    size_t next_word_hash = sdbm(next_word);
    bool is_const = false;
    for (size_t i = 0; i < sizeof(AllConstantsHash) / sizeof(size_t); i++) {
        if (next_word_hash == AllConstantsHash[i]) {
            (*value).lf = AllConstantsValues[i];
            *type = TYPE_NUM;
            is_const = true;
            free(next_word);
            break;
        }
    }
    if (is_const)
        break;
    
    if (isalpha(NextChar)) { // if accepted symbol and not operation -> its variable
        (*value).var.name   = next_word;
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