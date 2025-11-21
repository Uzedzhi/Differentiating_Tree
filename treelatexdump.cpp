#include <stdio.h>
#include <string.h>
#include "tree.h"

#include "treelatexdump.h"

void WriteLatexHeader() {
    FILE *fp = fopen(DumpLatexFileName, "w");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\documentclass[12pt]{article}\n"
                "\\usepackage[utf8]{inputenc}\n"
                "\\usepackage{calc}\n"
                "\\usepackage{graphicx}\n"
                "\\usepackage[T2A]{fontenc}\n"
                "\\usepackage[russian]{babel}\n"
                "\\usepackage{xcolor}\n"
                "\\usepackage{amsmath}\n"
                "\\usepackage{amssymb}\n"
                "\\date{\\today}\n"
                "\\author{}\n"
                "\\title{DiffTreeDump}\n"
                "\\begin{document}\n"
                "\\maketitle\n\n");
    fclose(fp);
}

#define CHECK(direction, type_t) \
    node->direction->type == type_t
#define IS_SAME_VAR_NAMES(direction) \
    strcmp(node->direction->value.var.name, DiffVarName) == 0

VarPosTypes GetVarPosType(Node_t * node, const char * DiffVarName) {
    if (CHECK(left, TYPE_OP) || (CHECK(left, TYPE_VAR) && IS_SAME_VAR_NAMES(left))) {
        if (CHECK(right, TYPE_OP)|| (CHECK(right, TYPE_VAR) && IS_SAME_VAR_NAMES(right))) {
            return VARVAR;
        }
        else {
            return VARNUM;
        }
    }
    else {
        if (CHECK(right, TYPE_OP) || (CHECK(right, TYPE_VAR) && IS_SAME_VAR_NAMES(right))) {
            return NUMVAR;
        }
        else {
            return NUMNUM;
        }
    }
}
#undef CHECK
#undef IS_SAME_VAR_NAMES

void WriteLatexStepOfDifferentiation(Node_t * func, Node_t *diff_func, 
                                     const char * diff_var, const char *str) {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\subsection{Дифференцируем часть выражения:}\n\n");
    fprintf(fp, "\\[\n");
    DumpNodesToLatexRec(func, fp);
    fprintf(fp, "\n\\]\n\n");
    fprintf(fp, "\\textcolor{blue}{Так как:} ");

    VarPosTypes PowType = NPOS;
    switch(func->type) {
        case TYPE_NUM:
            fprintf(fp, "Производная константы равна нулю\n\n");
            fprintf(fp, "\\[\n(C)' = 0\n\\]\n\n");

            fprintf(fp, "\\textcolor{red}{То результат:}\n");
            fprintf(fp, "\\[\n");
            fprintf(fp, "\\resizebox{\\minof{\\width}{\\textwidth}}{!}{\\displaystyle\n(");
            DumpNodesToLatexRec(diff_func, fp);
            fprintf(fp, "\n}\n\\]\n\n");
            break;
            
        case TYPE_VAR:
            if (strcmp(func->value.var.name, diff_var) == 0) {
                fprintf(fp, "Производная переменной по самой себе равна 1\n\n");
                fprintf(fp, "\\[\n\\frac{d%s}{d%s} = 1\n\\]\n\n", diff_var, diff_var);
            } else {
                fprintf(fp, "Производная переменной %s по %s равна 0}\n\n", 
                            func->value.var.name, diff_var);
                fprintf(fp, "\\[\n\\frac{d}{d%s}(%s) = 0\n\\]\n\n", 
                        diff_var, func->value.var.name);
            }
            fprintf(fp, "\\textcolor{red}{То результат:} ");
            fprintf(fp, "\\[\n");
            DumpNodesToLatexRec(diff_func, fp);
            fprintf(fp, "\n\\]\n\n");
            break;
            
        case TYPE_OP:
            switch(func->value.operation) {
                case OP_ADD:
                    fprintf(fp, "Производная суммы равна сумме производных\n\n"
                                "\\[\n(x + y)' = x' + y'\n\\]\n\n");
                    break;
                case OP_SUB:
                    fprintf(fp, "Производная разности равна разности производных\n\n"
                                "\\[\n(u - v)' = u' - v'\n\\]\n\n");
                    break;
                case OP_MUL:
                    fprintf(fp, "Производная произведения вычисляется по формуле\n\n"
                                "\\[\n(u \\cdot v)' = u' \\cdot v + u \\cdot v'\n\\]\n\n");
                    break;
                case OP_DIV:
                    fprintf(fp, "Производная частного вычисляется по формуле\n\n"
                                "\\[\n(\\frac{u}{v})' = \\frac{u' \\cdot v - u \\cdot v'}{v^2}\n\\]\n\n");
                    break;
                case OP_SIN:
                    fprintf(fp, "Производная синуса это косинус\n\n"
                                "\\[\n(sin(x))\' = cos(x) * x\'\n\\]\n\n");
                    break;
                case OP_TG:
                    fprintf(fp, "Производная тангенса вычисляется по формуле\n\n"
                                "\\[\n(tg(x))\' = \\frac{x\'}{cos^2(x)}\n\\]\n\n");
                    break;
                case OP_CTG:
                    fprintf(fp, "Производная котангенса вычисляется по формуле\n\n"
                                "\\[\n(ctg(x))\' = \\frac{x\'}{sin^2(x)}\n\\]\n\n");
                    break;
                case OP_POW:
                    PowType = GetVarPosType(func, diff_var);
                    switch(PowType) {
                        case NUMNUM:
                            fprintf(fp, "Производная числа в степени другого числа это 0\n\n"
                                        "\\[(a^b)\' = 0\n\\]\n\n");
                            break;
                        case VARNUM:
                            fprintf(fp, "Производная переменной в степени числа вычисляется по формуле\n\n"
                                        "\\[(x^a)\' = a \\cdot {x}^{a - 1} \\cdot x\'\n\\]\n\n");
                            break;
                        case NUMVAR:
                            fprintf(fp, "Производная числа в степени переменной вычисляется по формуле\n\n"
                                        "\\[(a^x)\' = a^x \\cdot ln(a) \\cdot x\'\n\\]\n\n");
                            break;
                        case VARVAR:
                            fprintf(fp, "Производная переменной в степени переменной вычисляется по формуле\n\n"
                                        "\\[(u^v)\' =  {e}^{vln(u)} \\cdot (vln(u))\'\n\\]\n\n");
                            break;
                    }
                    break;
                case OP_LN:
                    fprintf(fp, "Производная натурального логарифма вычисляется по формуле\n\n"
                                "\\[\n(ln(x))\' = \\frac{x\'}{x}\n\\]\n\n");
                    break;
                case OP_LOG:
                    PowType = GetVarPosType(func, diff_var);
                    switch(PowType) {
                        case NUMNUM:
                            fprintf(fp, "Производная логарифма по основанию a от b (просто число) равна 0\n\n"
                                        "\\[(log_{a} (b))\' = 0\n\\]\n\n");
                            break;
                        case VARNUM:
                            fprintf(fp, "Производная логарифма по основанию x от a равна\n\n"
                                        "\\[(log_{x} (a))\' = -\\frac{(log_{x}^2 (a) \\cdot x\'}{x \\cdot ln(a)}\n\\]\n\n");
                            break;
                        case NUMVAR:
                            fprintf(fp, "Производная логарифма по основанию a от x равна\n\n"
                                        "\\[(log_{a} (x))\' = \\frac{x\'}{x \\cdot ln(a)}\n\\]\n\n");
                            break;
                        case VARVAR:
                            fprintf(fp, "Производная логарифма по основанию u от v равна\n\n"
                                        "\\[(log_{u} (v))\' = \\frac{\\frac{v\'}{v} \\cdot ln(u) - \\frac{u\'}{u} \\cdot ln(u)}{ln^2(u)}\n\\]\n\n");
                            break;
                    }
                    break;
            }

            fprintf(fp, "\\textcolor{red}{То результат:}\\\n");
            fprintf(fp, "\\[\n");
            fprintf(fp, "\\resizebox{\\minof{\\width}{\\textwidth}}{!}{\\displaystyle\n(");
            DumpNodesToLatexRec(func, fp);
            fprintf(fp, ")' = ");
            DumpNodesToLatexRec(diff_func, fp);
            fprintf(fp, "\n}\n\\]\n\n");
            break;
    }
    
    fprintf(fp, "\\hrulefill\n\n");
    fclose(fp);
}

void WriteLatexResult(Node_t *diff_func, const char * diff_var, const char * str) {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\section{%s}\n\n", str);
    fprintf(fp, "\\resizebox{\\minof{\\width}{\\textwidth}}{!}{$\\displaystyle\\boxed{f'(%s) = \n", diff_var);
    DumpNodesToLatexRec(diff_func, fp);
    fprintf(fp, "}$}\n");


    fclose(fp);
}

void WriteLatexDifferential(Node_t * func, const char * diff_var) {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\section{Нужно найти производную по %s функции}\n\n", diff_var);
    fprintf(fp, "\\[\nf(%s) = ", diff_var);
    DumpNodesToLatexRec(func, fp);
    fprintf(fp, "\n\\]\n\n");
    
    fprintf(fp, "\\section{Заметим очевидное:}\n\n");
    fclose(fp);
}

void WriteLatexToes() {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\end{document}\n");
    fclose(fp);
}

TreeErr MakePdfFromLatex() {
    char EncodeInUTF8[MAX_STR_SIZE] = {};
    snprintf(EncodeInUTF8, MAX_STR_SIZE - 1, "iconv -f cp1251 -t utf-8 %s > %s", DumpLatexFileName, DumpLatexFileNameEncoded);
    if (system(EncodeInUTF8) != 0) {
        add_error(ERR_CMD_INVALID, "%s", EncodeInUTF8);
    }

    char command[MAX_STR_SIZE];
    snprintf(command, MAX_STR_SIZE - 1, "pdflatex -interaction=nonstopmode %s", DumpLatexFileNameEncoded);
    
    if (system(command) != 0) {
        ADD_ERROR_AND_RETURN(ERR_CMD_INVALID, "%s", command);
    }
    return OK;
}


#define DUMPLEFT DumpNodesToLatexRec(cur_node->left, fp);
#define DUMPRIGHT DumpNodesToLatexRec(cur_node->right, fp);
void DumpNodesToLatexRec(Node_t * cur_node, FILE *fp) {
    if (cur_node == NULL) return;
    
    switch(cur_node->type) {
        case TYPE_NUM:
            if ((long long) cur_node->value.lf == cur_node->value.lf)
                fprintf(fp, "%lld", (long long) cur_node->value.lf);
            else
                fprintf(fp, "%.2f", cur_node->value.lf);

            break;
        case TYPE_VAR:
            fprintf(fp, "%s", cur_node->value.var.name);
            break;
        case TYPE_OP:
            switch(cur_node->value.operation) {
                case OP_ADD:
                    // fprintf(fp, "(");
                    DUMPLEFT
                    fprintf(fp, " + ");
                    DUMPRIGHT
                    // fprintf(fp, ")");
                    break; 
                case OP_SUB:
                    // fprintf(fp, "(");
                    DUMPLEFT
                    fprintf(fp, " - ");
                    DUMPRIGHT
                    // fprintf(fp, ")");
                    break;   
                case OP_MUL:
                    // fprintf(fp, "(");
                    DUMPLEFT
                    fprintf(fp, " \\cdot ");
                    DUMPRIGHT
                    // fprintf(fp, ")");
                    break;
                case OP_DIV:
                    fprintf(fp, "\\frac{");
                    DUMPLEFT
                    fprintf(fp, "}{");
                    DUMPRIGHT
                    fprintf(fp, "}");
                    break;
                case OP_SIN:
                    fprintf(fp, "sin(");
                    DUMPRIGHT
                    fprintf(fp, ")");
                    break;
                case OP_COS:
                    fprintf(fp, "cos(");
                    DUMPRIGHT
                    fprintf(fp, ")");
                    break;
                case OP_POW:
                    fprintf(fp, "({");
                    DUMPLEFT
                    fprintf(fp, "}^{");
                    DUMPRIGHT
                    fprintf(fp, "})");
                    break;
                case OP_LN:
                    fprintf(fp, "ln(");
                    DUMPRIGHT
                    fprintf(fp, ")");
                    break;
                case OP_TG:
                    fprintf(fp, "tg(");
                    DUMPRIGHT
                    fprintf(fp, ")");
                    break;
                case OP_CTG:
                    fprintf(fp, "ctg(");
                    DUMPRIGHT
                    fprintf(fp, ")");
                    break;
                case OP_LOG:
                    fprintf(fp, "log_{");
                    DUMPLEFT
                    fprintf(fp, "} (");
                    DUMPRIGHT
                    fprintf(fp, ")");
            }
            break;
    }
}
#undef DUMPLEFT
#undef DUMPRIGHT

void DumpNodeToLatex(Node_t * node) {
    sassert(node, ERR_PTR_NULL);

    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    DumpNodesToLatexRec(node, fp);
    fclose(fp);
}