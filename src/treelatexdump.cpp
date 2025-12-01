#include <stdio.h>
#include <math.h>
#include <string.h>
#include "tree.h"

#include "treelatexdump.h"
#include "treedump.h"
#include "treehelpers.h"

static size_t CurUniquePicId_gl = 0;

void WriteLatexHeader() {
    FILE *fp = fopen(DumpLatexFileName, "w");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\documentclass[a4paper,12pt]{article}\n"
                "\\usepackage[utf8]{inputenc}\n"
                "\\usepackage[T2A]{fontenc}\n"
                "\\usepackage[russian]{babel}\n"
                "\n"
                "\\usepackage{geometry}\n"
                "\\geometry{a4paper, margin=2cm}\n"
                "\n"
                "\\usepackage{tikz}\n"
                "\\usepackage{graphicx}\n"
                "\\usepackage{calc}\n"
                "\\usepackage{xcolor}\n"
                "\\usepackage{amsmath}\n"
                "\\usepackage{amssymb}\n"
                "\\usepackage{hyperref}\n"
                "\\usepackage{enumitem}\n"
                "\\usepackage{tcolorbox}\n"
                "\\tcbuselibrary{skins,breakable}\n"
                "\n"
                "\\hypersetup{\n"
                "    colorlinks=true,\n"
                "    linkcolor=blue,\n"
                "    filecolor=magenta,\n"
                "    urlcolor=cyan\n"
                "}\n"
                "\\urlstyle{same}\n"
                "\\newtcolorbox{coloredbox}[2]{\n"
                "colback=#1,\n"
                "colframe=#2,\n"
                "breakable,\n"
                "enhanced,\n"
                "boxrule=0.8pt,\n"
                "arc=2pt,\n"
                "left=6pt,right=6pt,top=6pt,bottom=6pt\n"
                "}\n"
                "\n"
                "\\newenvironment{imgbox}[2]{\n"
                "\\begin{tcolorbox}[\n"
                "enhanced,\n"
                "sidebyside,\n"
                "lefthand ratio=0.2,\n"
                "colback=#1,\n"
                "colframe=black,\n"
                "sidebyside align=center seam\n"
                "]\n"
                "\\includegraphics[width=\\linewidth]{#2}\n"
                "\\tcblower\n"
                "}{\n"
                "\\end{tcolorbox}\n"
                "}\n"
                "\n");
    fclose(fp);
}

void LatexPrintCringe(const char *diffVarName) {
    sassert(diffVarName, ERR_PTR_NULL);

    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp,
        "\\section{Ряд Тейлора: так близко но так далеко\\\n"
        "(когда разложил до $o(x^1)$)}\n"
        "\\label{sec:taylor}\n\n"
        "\\begin{coloredbox}{blue!5!white}{blue!60!black}\n"
        "\\textbf{Точка разложения.}\\\\[0.4em]\n"
        "Рассматриваем точку\n"
        "\\[\n"
        "  a = {\\TailorPoint},\n"
        "\\]\n"
        "и разложение до порядка $o\\bigl((x-\\TailorPoint)^{\\TailorAccuracy}\\bigr)$.\n"
        "\\end{coloredbox}\n\n"
        "\\begin{coloredbox}{purple!5!white}{purple!50!black}\n"
        "\\textbf{Локальный вид функции.}\\\\[0.4em]\n"
        "В окрестности точки $a$ функция ведёт себя так:\n"
        "\\[\n"
        "  \\TaylorSeriesApprox + o(x - \\TailorPoint)^{\\TailorAccuracy}\n"
        "\\]\n"
        "Есть выражение делаем из мухи слона, а здесь делаем из слона муху\n"
        "Все труды бедного дерева шли ради этого момента\n"
        "\\end{coloredbox}\n\n"
        "\\begin{imgbox}{gray!20}{memes/wolf4.jpg}\n"
        "\\textbf{Мотивационный спич.}\\\\[0.4em]\n"
        "Выйди за свою зону комфорта($U_{\\delta }(b)$), и только тогда,\n"
        "даже при всех $\\varepsilon > 0$, ты сможешь найти свое $\\delta$\n"
        "\\end{imgbox}"
        
        "\n\n"
    );

    fclose(fp);
}

void PrintLatexVarsInTxt(DiffTree_t *tree, size_t point, size_t NthDerivative, size_t accuracy) {
    FILE * fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\newcommand{\\TailorPoint}{%zu}\n"
                "\\newcommand{\\TailorAccuracy}{%zu}\n"
                "\\newcommand{\\NthDerivative}{%zu}\n", point, accuracy, NthDerivative);

    fprintf(fp, "\\newcommand{\\func}{f(x) = ");
    DumpNodesToLatexRec(tree->root, fp, false);
    fprintf(fp, "}\n");
    fprintf(fp, "\\title{Дифуррициатор}\n"
                "\\author{Не важно кто ты, важно насколько быстро ты РаСтЕшЬ\\\\\n\\\\\n"
                "Если ваша производная 0, то не радуйтесь что вы находитесь\\\\\n"
                "в своем максимуме, ведь это может быть минимум\n}\n"
                "\\date{\\today}\n"
                "\\begin{document}\n"
                "\\maketitle\n"
                "\\tableofcontents\n"
                "\\newpage\n"
                "\n\n"
                "\\section{Знакомство с функцией}\n"
                "\\label{sec:intro}\n\n"
                "\\begin{coloredbox}{green!5!white}{green!50!black}\n"
                "\\textbf{Главный допрашиваемый пациент.}\\\\[0.4em]\n"
                "\\[\n"
                "  \\boxed{\\func}\n"
                "\\]\n"
                "В этой серии мы будем делать всякое и\n"
                "даже больше с этой безобидной функцией\n"
                "\\end{coloredbox}\n\n"
                "\\begin{imgbox}{gray!15}{memes/wolf1.jpg}\n"
                "Каждую функцию можно исследовать, но не каждую понять.\n"
                "\\end{imgbox}\n");
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

void WriteLatexStepOfDifferentiation(FILE *fp, Node_t * func, Node_t *diff_func, 
                                     const char * diff_var, const char *str) {
    fprintf(fp, "\n\\vspace*{0.5cm}");
    fprintf(fp, "\\begin{coloredbox}{blue!2!white}{blue!50!black}\n");
    fprintf(fp, "\n\nДифференцируем часть выражения:\\ \n\n");
    fprintf(fp, "\\[\n");
    DumpNodesToLatexRec(func, fp, false);
    fprintf(fp, "\n\\]\n\n");
    
    fprintf(fp, "\\textcolor{blue}{Так как:} ");

    VarPosTypes PowType = NPOS;
    int RandStr = rand() % NUM_OF_RAND_STR;
    switch(func->type) {
        case TYPE_NUM:
            fprintf(fp, "Производная константы равна нулю\n\n");
            fprintf(fp, "\\[\n(C)' = 0\n\\]\n\n");

            fprintf(fp, "\\textcolor{red}{То %s результат:}\n", AllRandomStr[RandStr]);
            fprintf(fp, "\\[\n");
            fprintf(fp, "\\resizebox{\\minof{\\width}{\\textwidth}}{!}{$\\displaystyle\n(");
            DumpNodesToLatexRec(diff_func, fp, false);
            fprintf(fp, "\n)$}\n\\]\n\n");
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
            fprintf(fp, "\\textcolor{red}{То %s результат:}\n", AllRandomStr[RandStr]);
            fprintf(fp, "\\[\n");
            DumpNodesToLatexRec(diff_func, fp, false);
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
                case OP_COS:
                    fprintf(fp, "Производная косинуса это почти синус\n\n"
                                "\\[\n(cos(x))\' = sin(x) * (-1) * x\'\n\\]\n\n");
                    break;
                case OP_TG:
                    fprintf(fp, "Производная тангенса вычисляется по формуле\n\n"
                                "\\[\n(tg(x))\' = \\frac{x\'}{cos^2(x)}\n\\]\n\n");
                    break;
                case OP_CTG:
                    fprintf(fp, "Производная котангенса вычисляется по формуле\n\n"
                                "\\[\n(ctg(x))\' = \\frac{x\'}{sin^2(x)}\n\\]\n\n");
                    break;
                case OP_ARCSIN:
                    fprintf(fp, "Производная арксинуса вычисляется по формуле\n\n"
                                "\\[\n(\\arcsin x)' = \\frac{x'}{\\sqrt{1 - x^2}}\n\\]\n\n");
                    break;
                case OP_ARCCOS:
                    fprintf(fp, "Производная арккосинуса вычисляется по формуле\n\n"
                                "\\[\n(\\arccos x)' = -\\frac{x'}{\\sqrt{1 - x^2}}\n\\]\n\n");
                    break;
                case OP_ARCTG:
                    fprintf(fp, "Производная арктангенса вычисляется по формуле\n\n"
                                "\\[\n(\\arctg x)' = \\frac{x'}{1 + x^2}\n\\]\n\n");
                    break;
                case OP_ARCCTG:
                    fprintf(fp, "Производная арккотангенса вычисляется по формуле\n\n"
                                "\\[\n(\\arcctg x)' = -\\frac{x'}{1 + x^2}\n\\]\n\n");
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

            fprintf(fp, "\\textcolor{red}{То %s результат:}\n", AllRandomStr[RandStr]);
            fprintf(fp, "\\[\n");
            fprintf(fp, "\\resizebox{\\minof{\\width}{\\textwidth}}{!}{$\\displaystyle\n(");
            DumpNodesToLatexRec(func, fp, false);
            fprintf(fp, ")' = ");
            DumpNodesToLatexRec(diff_func, fp, false);
            fprintf(fp, "\n$}\n\\]\n");
            break;
    }
    
    fprintf(fp, "\\end{coloredbox}\n"
                "\\hrulefill\n\n");
}

void WriteLatexResultInFile_internal(FILE *fp, Node_t *diff_func, const char * diff_var, size_t num_of_derivative, const char * str) {
    fprintf(fp, "\\subsection{%s}\\\n\n", str);
    fprintf(fp, "\\resizebox{\\minof{\\width}{\\textwidth}}{!}{$\\displaystyle\\boxed{f");
    for (size_t i = 0; i < num_of_derivative; i++) {
        fprintf(fp, "\'");
    }
    fprintf(fp, "(%s) = \n", diff_var);
    DumpNodesToLatexRec(diff_func, fp, false);
    fprintf(fp, "}$}\n");
}

void WriteLatexContentTable() {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, " ");

    fclose(fp);
}

void WriteLatexResult(Node_t *diff_func, const char * diff_var, size_t num_of_derivative, const char * str) {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\subsection{%s}\n\n", str);
    fprintf(fp, "\\resizebox{\\minof{\\width}{\\textwidth}}{!}{$\\displaystyle\\boxed{f");
    for (size_t i = 0; i < num_of_derivative; i++) {
        fprintf(fp, "\'");
    }
    fprintf(fp, "(%s) = \n", diff_var);
    DumpNodesToLatexRec(diff_func, fp, false);
    fprintf(fp, "}$}\n");

    fclose(fp);
}

void WriteLatex(const char * diff_var, const char * str) {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\section{%s}\n\n", str);
    fclose(fp);
}

bool check_if_unar(TypeOp_t oper) {
    switch(oper) {
        case OP_MUL:
        case OP_ADD:
        case OP_SUB:
        case OP_DIV:
        case OP_LOG:
        case OP_POW:
            return false;
        case OP_TG:
        case OP_CTG:
        case OP_LN:
        case OP_SH:
        case OP_CH:
        case OP_CTH:
        case OP_TH:
        case OP_ARCSIN:
        case OP_ARCCOS:
        case OP_ARCTG:
        case OP_ARCCTG:
        case OP_SIN:
        case OP_COS:
            return true;
    }
    return false;
}

void PrintLatexTailorHead(DiffTree_t *tree, const char * DiffVarName) {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);
    
    fprintf(fp, "\\section[Вычисляем формулу тейлора}{Вычисление формулы тейлора для функции");
    INRESIZEBOX(fprintf(fp, "f(%s) = ", DiffVarName); 
                DumpNodeToLatex(tree->root);
                );
    fclose(fp);
    
}

Node_t * PlaceRandomExpression(Node_t *node, size_t *iterations) {
    (*iterations)++;
    Node_t * left = NULL;
    Node_t * right = NULL; 

    // if not standart operation ( + - * / )
    // when roll random again
    TypeOp_t RandOperation = (TypeOp_t) (rand() % NUM_OF_OPERATIONS);
    if (RandOperation >= NUM_OF_STD_OPERATIONS)
        RandOperation = (TypeOp_t) (rand() % NUM_OF_OPERATIONS);
    
    // we dont want many nums so we roll again if we see one
    TreeType_t RandType = (TreeType_t) (rand() % RANDTYPES_MAX);
    if (RandType == TYPE_NUM)
        TreeType_t RandType = (TreeType_t) (rand() % RANDTYPES_MAX);
    
    bool is_unar = check_if_unar(RandOperation);
    bool is_const = false;

    // In 20% of times we set RandNum to some random const value
    size_t RandNum = rand() % RANDNUM_MAX;
    if (RandNum > 0.8 * RANDNUM_MAX) {
        is_const = true;
        RandNum = rand() % (NUM_OF_CONST);
    }

    // if deep enough tree
    // when only var or num
    if (*iterations > MAX_RAND_DEEP_TREE) 
        RandType = (TreeType_t) (rand() % 2);

    switch(RandType) {
        case 0: // type_var
            node = NewNode(TYPE_VAR, (TreeElem_u) {.var = {strdup("x"), sdbm("x"), 0}}, NULL, NULL);
            DUMPNODE(node, YES_DIVISION, "after some random magic 1");
            return node;
        case 1: // type_num
            if (is_const)
                node = NewNode(TYPE_NUM, (TreeElem_u) {.lf = AllConstantsValues[RandNum]}, NULL, NULL);
            else
                node = NewNode(TYPE_NUM, (TreeElem_u) {.lf = (double) RandNum}, NULL, NULL);
            DUMPNODE(node, YES_DIVISION, "after some random magic 2");
            return node;
        default: // type_op
            if (is_unar) {
                left = NewNode(TYPE_NUM, {.lf = 0.0}, NULL, NULL);
                right = PlaceRandomExpression(NULL, iterations);
            }
            else {
                if ((RandNum % 2) != 0) {// imitating 50/50 choice
                    left = PlaceRandomExpression(NULL, iterations);
                    right = PlaceRandomExpression(NULL, iterations);
                }
                else {
                    right = PlaceRandomExpression(NULL, iterations);
                    left = PlaceRandomExpression(NULL, iterations);
                }
            }
            node = NewNode(TYPE_OP, (TreeElem_u) {.operation = RandOperation}, left, right);
            DUMPNODE(node, YES_DIVISION, "after some random magic 3");
            return node;
    }
}

void PrintGnuplotHeaderInFile(FILE *fp, const char * VarName, const char * file_name, double xfrom, double xto) {
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "set terminal pngcairo size 800,600\n");
    fprintf(fp, "set output '%s'\n", file_name);
    fprintf(fp, "set xlabel '%s'\n", VarName);
    fprintf(fp, "set ylabel 'f(%s)'\n", VarName);
    fprintf(fp, "set grid\n");
    fprintf(fp, "set pointsize 0.5\n");
    fprintf(fp, "set xzeroaxis lt 1 lw 1 lc rgb \"black\"\n");
    fprintf(fp, "set yzeroaxis lt 1 lw 1 lc rgb \"black\"\n");
    fprintf(fp, "set xrange [%lf:%lf]\n", xfrom, xto);
}

void GetTreeGraphInTxt(DiffTree_t * tree, const char * file_name, double xfrom, double xto, const char *VarName, bool need_ylimit) {
    sassert(tree, ERR_PTR_NULL);
    sassert(file_name, ERR_PTR_NULL);

    FILE *fp = fopen(file_name, "w");
    sassert(fp, ERR_PTR_NULL);

    bool HasAtLeastOnePoint = false;
    double step = fabs((xto - xfrom) / MAXNUM_OF_STEPS);
    size_t VarHash = sdbm(VarName);
    double prevy = NAN;

    for (double x = xfrom; x <= xto; x += step) {
        double y = GetCalculatedAnswer(tree, tree->root, VarHash, x);
        if ((need_ylimit && fabs(y) > 10) || isnan(y) || fabs(y) > MAX_FUNCGRAPH_VALUE || (!isnan(prevy) && fabs(prevy - y) > MAX_FUNCGRAPH_GAP)) {
            fprintf(fp, "%lf, NAN\n", x);
        }
        else {
            HasAtLeastOnePoint = true;
            fprintf(fp, "%lf %lf\n", x, y);
        }
        prevy = y;
    }
    if (HasAtLeastOnePoint == false) {
        fprintf(fp, "0 0\n");
    }

    fclose(fp);
}

char * PrintGnuplotGraphOfFunc(DiffTree_t *tree, DiffTree_t *Tailortree, DiffTree_t *Tangenttree, const char * VarName, double xfrom, double xto) {
    sassert(tree, ERR_PTR_NULL);
    sassert(Tailortree, ERR_PTR_NULL);
    sassert(Tangenttree, ERR_PTR_NULL);

    char unique_file_name[MAX_STR_SIZE] = {};
    snprintf(unique_file_name, MAX_STR_SIZE - 1, "funcgraphs/funcgraph%zu.png", CurUniquePicId_gl);
    CurUniquePicId_gl++;

    GetTreeGraphInTxt(tree,         "data/data1.txt", xfrom, xto, VarName, false);
    GetTreeGraphInTxt(Tailortree,   "data/data2.txt", xfrom, xto, VarName, true);
    GetTreeGraphInTxt(Tangenttree,  "data/data3.txt", xfrom, xto, VarName, true);

    FILE *fp = popen("gnuplot", "w");
    sassert(fp, ERR_PTR_NULL);

    PrintGnuplotHeaderInFile(fp, VarName, unique_file_name, xfrom, xto);
    fprintf(fp, "plot \'data/data1.txt\' with points lc rgb \"red\" title \"Func\", \\\n"
                     "\'data/data2.txt\' with points lc rgb \"blue\" title \"Taylor of Func\", \\\n"
                     "\'data/data3.txt\' with points lc rgb \"green\" title \"angent to Func\"");
    pclose(fp);
    return strdup(unique_file_name);
}

Node_t * MakeRandomExpression() {
    Node_t * node = NULL;
    size_t iterations = 0;
    node = PlaceRandomExpression(node, &iterations);
    return node;
}

void WriteLatexFuncPic(Node_t *func, Node_t *TailorFunc, Node_t *TangentFunc, const char * PicFileName, double TangentX, double TailorX) {
    sassert(PicFileName, ERR_PTR_NULL);

    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\section{Графики и иллюстрации всего этого д... ерева}\n"
                "\\label{sec:plots}\n\n");
    fprintf(fp, "\\subsection{График функции $f(x) = ");
    DumpNodesToLatexRec(func, fp, false);
    fprintf(fp, "$, касательной к этой функции в точке %lf и график Тейлора этой функции в точке %lf:}\n", TangentX, TailorX);
    fprintf(fp, "\\begin{tikzpicture}\n"
                "\\node[inner sep=0] (img) at (15, 0) {\n"
                "    \\includegraphics[width=10cm]{%s}\n"
                "};\n"
                "\\end{tikzpicture}\n\n", PicFileName);
    fclose(fp);
}

void WriteLatexMeme(Node_t *func, const char *diff_var, const char * MemeImageName) {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    Node_t * diff_func = CopyNodes(func);
    DUMPNODE(diff_func, YES_DIVISION, "treedump");

    fprintf(fp, "\n\\subsection{мемчик}");
    fprintf(fp, "\n"
                "\\begin{tikzpicture}\n"
                "\\node[inner sep=0] (img) at (15, 0) {\n"
                "    \\includegraphics[width=10cm]{%s}\n"
                "};\n"
                "\n"
                "\\node[anchor=west] at ([xshift=-7cm,yshift=4cm]img.west) {\n", MemeImageName);
    INRESIZEBOX(fprintf(fp, "f(x) = ");
                DumpNodesToLatexRec(diff_func, fp, false);)
    fprintf(fp, "};\n\n");

    fprintf(fp, "\\node[anchor=west] at ([xshift=-7cm,yshift=0cm]img.west) {\n");
    DUMPNODE(diff_func, YES_DIVISION, "dumped functions 0 derivative");
    INRESIZEBOX(fprintf(fp, "f\'(x) = ");
                diff_func = optimize_and_differentiate(fp, diff_func, "x", 1, false);
                DumpNodesToLatexRec(diff_func, fp, false);)
    DUMPNODE(diff_func, YES_DIVISION, "dumped functions 1 derivative");
    fprintf(fp, "};\n\n");
    
    fprintf(fp, "\\node[anchor=west] at ([xshift=-7cm,yshift=-4cm]img.west) {\n");
    INRESIZEBOX(fprintf(fp, "f\'\'(x) = ");
                diff_func = optimize_and_differentiate(fp, diff_func, "x", 2, false);
                DumpNodesToLatexRec(diff_func, fp, false);)
    DUMPNODE(diff_func, YES_DIVISION, "dumped functions 2 derivative");
    fprintf(fp, "};\n\n");

    fprintf(fp, "\\end{tikzpicture}\n"
                "\n\n");

    NodeDtorDiff(&diff_func);
    fclose(fp);
}

void WriteLatexDifferential(Node_t * func, const char * diff_var) {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\section[Нахождение производной $f(x) = ");
    DumpNodesToLatexRec(func, fp, false);
    fprintf(fp, "$]{Нужно найти производную по %s функции}\n\n", diff_var);
    fprintf(fp, "\\[\nf(%s) = ", diff_var);
    DumpNodesToLatexRec(func, fp, false);
    fprintf(fp, "\n\\]\n\n");
    
    fprintf(fp, "\\section[Находим]{Заметим очевидное:}\n\n");
    fclose(fp);
}

void WriteLatexToes() {
    FILE *fp = fopen(DumpLatexFileName, "a");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "\\section{Спасибо за внимание, пожалуйста не вызывайте меня больше}\n");
    fprintf(fp, "\\end{document}\n");
    fclose(fp);
}

TreeErr MakePdfFromLatex() {
    char EncodeInUTF8[MAX_STR_SIZE] = {};
    snprintf(EncodeInUTF8, MAX_STR_SIZE - 1, "iconv -f cp1251 -t utf-8 %s > %s", DumpLatexFileName, DumpLatexFileNameEncoded);
    if (system(EncodeInUTF8) != 0) {
        ADD_ERROR_AND_RETURN(ERR_CMD_INVALID, "%s", EncodeInUTF8);
    }

    char command[MAX_STR_SIZE];
    snprintf(command, MAX_STR_SIZE - 1, "pdflatex -interaction=nonstopmode -output-directory=latexdump %s > /dev/null", DumpLatexFileNameEncoded);
    
    if (system(command) != 0) {
        ADD_ERROR_AND_RETURN(ERR_CMD_INVALID, "%s", command);
    }
    return OK;
}

int GetPriorityOfOper(TypeOp_t oper) {
    switch(oper) {
        case OP_MUL:
        case OP_DIV:
        case OP_POW:
            return 2;
        case OP_ADD:
        case OP_SUB:
            return 3;
        case OP_SIN:
        case OP_COS:
        case OP_TG:
        case OP_CTG:
        case OP_LN:
        case OP_LOG:
        case OP_SH:
        case OP_CH:
        case OP_CTH:
        case OP_TH:
        case OP_ARCSIN:
        case OP_ARCCOS:
        case OP_ARCTG:
        case OP_ARCCTG:
            return 3;
    }
    return 4;
}

bool CheckIfTypeWithMorePriority(Node_t *node) {
    Node_t *left        = node->left;
    Node_t *right       = node->right;
    if (left != NULL && left->type == TYPE_OP) {
        TypeOp_t op_left    = left->value.operation;
        if (GetPriorityOfOper(op_left) > GetPriorityOfOper(node->value.operation))
            return true;
        return false;
    }
    if (right != NULL && right->type == TYPE_OP) {
        TypeOp_t op_right   = right->value.operation;
        if (GetPriorityOfOper(op_right) > GetPriorityOfOper(node->value.operation))
            return true;
        return false;
    }
    return false;
}

#define DUMPLEFT(need_parenthises) DumpNodesToLatexRec(cur_node->left, fp, need_parenthises);
#define DUMPRIGHT(need_parenthises) DumpNodesToLatexRec(cur_node->right, fp, need_parenthises);
#define ONEARGCMD_DUMP(oper) \
    fprintf(fp, "\\" #oper "(");\
    DUMPRIGHT(false)\
    fprintf(fp, ")");
#define STDOPERATION_DUMP(oper) \
    if (need_parenthises)\
        fprintf(fp, "(");\
    if (CheckIfTypeWithMorePriority(cur_node))\
        DUMPLEFT(true)\
    else\
        DUMPLEFT(false)\
    fprintf(fp, " " #oper " ");\
    if (CheckIfTypeWithMorePriority(cur_node))\
        DUMPRIGHT(true)\
    else \
        DUMPRIGHT(false)\
    if (need_parenthises)\
        fprintf(fp, ")");
void DumpNodesToLatexRec(Node_t * cur_node, FILE *fp, bool need_parenthises) {
    if (cur_node == NULL) return;

    bool is_const = false;
    switch(cur_node->type) {
        case TYPE_NUM:
            for (size_t i = 0; i < NUM_OF_CONST; i++) {
                if (is_same(cur_node->value.lf, AllConstantsValues[i])) {
                    fprintf(fp, "%s", AllConstantStr[i]);
                    is_const = true;
                    break;
                }
            }
            if (!is_const) {
                if (isnan(cur_node->value.lf))
                    fprintf(fp, "NAN");
                else if ((long long) cur_node->value.lf == cur_node->value.lf)
                    fprintf(fp, "%lld", (long long) cur_node->value.lf);
                else
                    fprintf(fp, "%.2f", cur_node->value.lf);
            }

            break;
        case TYPE_VAR:
            fprintf(fp, "%s", cur_node->value.var.name);
            break;
        case TYPE_OP:
            switch(cur_node->value.operation) {
                case OP_ADD:
                    STDOPERATION_DUMP(+);
                    break; 
                case OP_SUB:
                    STDOPERATION_DUMP(-);
                    break;   
                case OP_MUL:
                    STDOPERATION_DUMP(\\cdot);
                    break;
                case OP_DIV:
                    fprintf(fp, "\\frac{");
                    DUMPLEFT(false)
                    fprintf(fp, "}{");
                    DUMPRIGHT(false)
                    fprintf(fp, "}");
                    break;
                case OP_SIN:
                    ONEARGCMD_DUMP(sin);
                    break;
                case OP_COS:
                    ONEARGCMD_DUMP(cos);
                    break;
                case OP_POW:
                    fprintf(fp, "{(");
                    DUMPLEFT(false)
                    fprintf(fp, ")}^{");
                    DUMPRIGHT(false)
                    fprintf(fp, "}");
                    break;
                case OP_LN:
                    ONEARGCMD_DUMP(ln);
                    break;
                case OP_TG:
                    ONEARGCMD_DUMP(tg);
                    break;
                case OP_CTG:
                    ONEARGCMD_DUMP(ctg);
                    break;
                case OP_LOG:
                    fprintf(fp, "(\\log_{");
                    DUMPLEFT(false)
                    fprintf(fp, "} {");
                    DUMPRIGHT(false)
                    fprintf(fp, "})");
                    break;
                case OP_SH:
                    ONEARGCMD_DUMP(sh);
                    break;
                case OP_CH:
                    ONEARGCMD_DUMP(ch);
                    break;
                case OP_CTH:
                    ONEARGCMD_DUMP(cth);
                    break;
                case OP_TH:
                    ONEARGCMD_DUMP(th);
                    break;
                case OP_ARCSIN:
                    ONEARGCMD_DUMP(arcsin);
                    break;
                case OP_ARCCOS:
                    ONEARGCMD_DUMP(arccos);
                    break;
                case OP_ARCTG:
                    ONEARGCMD_DUMP(arctg);
                    break;
                case OP_ARCCTG:
                    ONEARGCMD_DUMP(arcctg);
                    break;
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

    DumpNodesToLatexRec(node, fp, false);
    fclose(fp);
}