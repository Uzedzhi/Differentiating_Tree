#ifndef TREEHELPERS_H
#define TREEHELPERS_H

#include "tree.h"

TreeErr GetTypeAndValue(DiffTree_t *tree, int *pos, char * buffer, TreeType_t *type, TreeElem_u *value);
TypeOp_t GetOperationType(char * str);
size_t Factorial(size_t n);
void skip_spaces(int *pos, char *buffer);
void print_help();
int GetUserAnswer(const char * StrIfIncorrect, int bottom, int top);
Node_t * GetExpr(char **expression);
Node_t * GetNum(char **expression);
Node_t * GetOper(char **expression);
Node_t * GetVar(char **expression);
Node_t * GetE(char ** expression);
Node_t * GetP(char ** expression);
Node_t * GetT(char ** expression);
Node_t * GetG(char ** expression);
void SkipSpaces(char ** expression);
void place_parents(Node_t *node, size_t *num_of_nodes);
Node_t * CopyNodes(Node_t *CopyNode);
char * get_next_word_in_quotes(int *pos, char *file_buffer);
size_t sdbm(const char * str);
Node_t * NewNode(TreeType_t type, TreeElem_u value, Node_t *left, Node_t *right);
bool is_same(double a, double b);
Node_t * create_node();
void scan_line_without_slashn(char *str);
size_t get_file_size(FILE *fp);
void skip_line(FILE *fp);

#endif // TREEHELPERS_H