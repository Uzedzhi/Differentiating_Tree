#ifndef TREELATEXDUMP_H
#define TREELATEXDUMP_H

#include <string.h>
#include "tree.h"

enum VarPosTypes {
    NPOS    = -1,
    VARVAR  = 0,
    VARNUM  = 1,
    NUMVAR  = 2,
    NUMNUM  = 3,
};

#define STARTTXTDUMPS() \
    PrintSiteHeader();\
    WriteLatexHeader();

#define FINISHTXTDUMPS()\
    PrintSiteToes();\
    WriteLatexToes();\
    printf(GREEN "succesessfully printed dump to latex!\n");

#define INCOLOREDBOX(colorbg, colorbd, ...) \
    fprintf(fp, "\\begin{coloredbox}{" #colorbg "!5!white}{" #colorbd "!50!white}\n");\
    __VA_ARGS__\
    fprintf(fp, "\\end{coloredbox}\n");

#define INRESIZEBOXWITHOUTMINOF(lines_of_code) \
    fprintf(fp, "\\resizebox{\\textwidth}{!}{$\\displaystyle ");\
    lines_of_code\
    fprintf(fp, " $}\n");
#define INRESIZEBOX(lines_of_code) \
    fprintf(fp, "\\resizebox{\\minof{\\width}{\\textwidth}}{!}{$\\displaystyle ");\
    lines_of_code\
    fprintf(fp, " $}\n");

#define WriteLatexResultInFile(fp,  diff_func, diff_var, num_of_derivative, ...) {\
    char str[MAX_STR_SIZE] = {};\
    snprintf(str, MAX_STR_SIZE - 1, __VA_ARGS__);\
    WriteLatexResultInFile_internal(fp, diff_func, diff_var, num_of_derivative, str);\
}


Node_t * MakeRandomExpression();
Node_t * differentiate(FILE *fp, Node_t * node, size_t *num_of_nodes, const char * DiffVarName, bool need_dump);
VarPosTypes GetVarPosType(Node_t * node, const char * DiffVarName);
TreeErr MakePdfFromLatex();
char * PrintGnuplotGraphOfFunc(DiffTree_t *tree, DiffTree_t *Tailortree, DiffTree_t *Tangenttree
                             , const char * VarName, double FuncRoot
                             , double xfrom, double xto, ylimits limits);
void WriteLatexHeader();
void WriteLatexToes();
void DumpNodeToLatex(Node_t * node);
void WriteLatexFuncPic(Node_t *func, Node_t *TailorFunc, Node_t *TangentFunc, const char * PicFileName, double FuncRoot, double TangentX, double TailorX);
void PrintLatexTailorHead(DiffTree_t *tree, const char * DiffVarName);
void WriteLatexStepOfDifferentiation(FILE *fp, Node_t * func, Node_t *diff_func, const char * diff_var, const char * str);
void WriteLatexResult(Node_t *diff_func, const char * diff_var, size_t num_of_derivative, const char * str);
void WriteLatexResultInFile_internal(FILE * fp, Node_t *diff_func, const char * diff_var, size_t num_of_derivative, const char * str);
void WriteLatexMeme(Node_t *diff_func, const char *diff_var, const char * MemeImageName);
void DumpNodesToLatexRec(Node_t * cur_node, FILE *fp, bool need_parenthises);
void WriteLatexDifferential(Node_t * func, const char * diff_var);
TreeErr GetLimitsAndPointsFromFile(const char *file_name, double *xfrom, double *xto, ylimits * limits
                                 , double *TailorPoint, size_t *TailorAccuracy, size_t *NthDerivative, double *TangentPoint);
void PrintLatexVarsInTxt(DiffTree_t *tree, size_t point, size_t NthDerivative, size_t accuracy);
void LatexPrintCringe(const char *diffVarName);
#endif // TREELATEXDUMP_H