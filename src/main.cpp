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
#include "treehelpers.h"


int main(int argc, char * argv[]) {
    srand(time(NULL));
    if (argc != 2 || strncmp(argv[1], "--", 2) != 0) {
        print_help();
        return OK;
    }
    char * file_name = argv[1] + 2;
    const char *argument = "x";

    STARTTXTDUMPS();
    TreeCtorDiff(tree);
    MakeTreeFromFile(tree, file_name);

    double xfrom    = 0.0;
    double xto      = 0.0;
    double TailorPoint = 0.0;
    size_t TailorAccuracy = 0.0;
    size_t NthDerivative = 0;
    double TangentPoint = 0;
    ylimits limits = {};
    GetLimitsAndPointsFromFile(file_name, &xfrom, &xto, &limits
                            , &TailorPoint, &TailorAccuracy, &NthDerivative, &TangentPoint);

    if (error.is_error == true) {
        TreeDtorDiff(tree);
        print_error(errors_text);
        return error.recently_added_error_int;
    }
        
    PrintLatexVarsInTxt(tree, TailorPoint, NthDerivative, TailorAccuracy);
    DiffTree_t *DerivativeTree1 = GetNthDerivative(tree, argument, NthDerivative, false);
    DiffTree_t *DerivativeTree2 = GetNthDerivative(tree, argument, NthDerivative, true);
    DiffTree_t *TailorTree      = GetLatexTailorSeries(tree, argument, TailorPoint, TailorAccuracy, true);
    DiffTree_t *FirstDerivative = GetNthDerivative(tree, argument, 1, false);
    DiffTree_t *TangentTree     = GetTangentTree(tree, FirstDerivative, TangentPoint, argument);
    LatexPrintCringe(argument);

    double FuncRoot     = TangentPoint;
    char *FileNameGraph = PrintGnuplotGraphOfFunc(tree, TailorTree, TangentTree, argument, FuncRoot, xfrom, xto, limits);

    WriteLatexFuncPic(tree->root, TailorTree->root, TangentTree->root, FileNameGraph, FuncRoot, TangentPoint, TailorPoint);
    WriteLatexMeme(tree->root, argument, "memes/memediff1.png");
    WriteLatexMeme(tree->root, argument, "memes/memediff2.jpg");

    PrintLatexFunctionResults(tree, TailorTree, NthDerivative, TailorPoint, TailorAccuracy);
    FINISHTXTDUMPS();
    MakePdfFromLatex();

    free(FileNameGraph);
    TreeDtorDiff(tree);
    TreeDtorDiff(FirstDerivative);
    TreeDtorDiff(DerivativeTree1);
    TreeDtorDiff(DerivativeTree2);
    TreeDtorDiff(TailorTree);
    TreeDtorDiff(TangentTree);

    if (error.is_error == true) {
        print_error(errors_text);
        return error.recently_added_error_int;
    }
    return 0;
}