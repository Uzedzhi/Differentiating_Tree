#ifndef TREELATEXDUMP_H
#define TREELATEXDUMP_H

#include "tree.h"

enum VarPosTypes {
    NPOS    = -1,
    VARVAR  = 0,
    VARNUM  = 1,
    NUMVAR  = 2,
    NUMNUM  = 3,
};



void WriteLatexHeader();
void WriteLatexToes();
void DumpNodeToLatex(Node_t * node);
TreeErr MakePdfFromLatex();
VarPosTypes GetVarPosType(Node_t * node, const char * DiffVarName);
void WriteLatexStepOfDifferentiation(Node_t * func, Node_t *diff_func, const char * diff_var, const char * str);
void WriteLatexResult(Node_t *diff_func, const char * diff_var, const char * str);
void DumpNodesToLatexRec(Node_t * cur_node, FILE *fp);
void WriteLatexDifferential(Node_t * func, const char * diff_var);
#endif // TREELATEXDUMP_H