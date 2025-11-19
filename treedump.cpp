#include "treedump.h"
#include "tree.h"
#include "treeverifier.h"
#include "../my_libs/error_manage.h"
#include "time.h"

extern error_t error;
static int count_graph_files = 0;

#ifndef NDEBUG
error_t open_live_server() {
    char command[MAX_STR_SIZE] = {};
    snprintf(command, MAX_STR_SIZE - 1, "open %s", dump_site_name);
    if (system(command) != 0) {
        add_error(ERR_CMD_INVALID, "%s", command);
    }
    return error;
}
#else
error_t open_live_server() {return error;}
#endif

void print_site_header() {
    if (count_graph_files != 0)
        return;
    FILE * fp = fopen(dump_site_name, "w");
    sassert(fp, ERR_PTR_NULL);
    fprintf(fp, "<!DOCTYPE html>\n"
                "<html lang=\"ru\">\n"
                "<head>\n"
                "<style>\n"
                "p, h1, h3, ul{\n"
                "margin: 0;\n"
                "}\n"
                "h2 {\n"
                "color: rgb(30, 0, 255);\n"
                "font-weight: bold;\n"
                "margin-bottom: 0px;\n"
                "}\n"
                "h3 {\n"
                "color: #5c525794\n;"
                "padding-left: 130px;\n"
                "}\n"
                "h1 {\n"
                "color: #e43383ff;\n"
                "font-weight: bold;\n"
                "}\n"
                ".images {\n"
                    "position: relative;\n"
                    "display: inline-block;\n"
                    "height: 200px;\n"
                "}\n"
                "</style>\n"
                "<title>my tree dump</title>\n"
                "</head>\n"
                "<body width=\"device-width\">\n"
            );
    fclose(fp);
}

void print_site_toes() {
    FILE * fp = fopen(dump_site_name, "a");\
    sassert(fp, ERR_PTR_NULL);\

    print_divider(fp);
    fprintf(fp, "</body>\n"
                "</html");
    fclose(fp);
}

void print_divider(FILE * fp) {
    fprintf(fp, "<h1 style=\"color: #a30f7eff\">||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||</h1>\n");
}

void print_to_html(DiffTree_t *tree, TreeCmd command, TreeElem_t value) {
    sassert(tree,   ERR_PTR_NULL);

    FILE * fp = fopen(dump_site_name, "a");
    sassert(fp, ERR_PTR_NULL);

    print_divider(fp);

    TreeErr err = TreeVerify(tree);
    if (err != OK) {
        fprintf(fp, "<h1>WARNING: this operation is incorrect because of err:</h1><ul>");
        fprintf(fp, "<h3><li>%s: %s</li></h3>", errors_text[err], error.error_info[err]);
    }
    fprintf(fp, "num_of_nodes: %zu\n", tree->num_of_nodes);
    fprintf(fp, "<h2> %s (value: %s)</h2>", command_desc[command], value);
    fprintf(fp, "<div class=\"images\">\n"
                "<img src=\"graph/graph%d.png\" class=\"img1\">\n</div>\n",  count_graph_files);
    count_graph_files++;
    fclose(fp);
}

void GraphDumpPrintNode(FILE *fp, Node_t *node, TreeErr is_err) {
    sassert(node, ERR_PTR_NULL);

    fprintf(fp, "{\n"
                "rank=0\n"
                "tree_node_info%p[style=\"rounded\", label=<\n"
                "<TABLE BORDER=\"0\" CELLSPACING=\"0\" CELLBORDER=\"0\" BGCOLOR=\"%s\">\n"
                "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"16\" COLOR=\"#4d545eff\"> type = </FONT> <FONT POINT-SIZE=\"18\">%s</FONT> </TD></TR>\n", node, (is_err != OK) ? "#d65c5ca9" : "#ffffff", AllValueTypesTxt[node->type]);
    switch(node->type) {
        case TYPE_NUM:
            fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"16\" COLOR=\"#4d545eff\"> val = </FONT> <FONT POINT-SIZE=\"18\">%lf</FONT></TD></TR>\n", node->value.lf);
            break;
        case TYPE_OP:
            fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"16\" COLOR=\"#4d545eff\"> val = </FONT> <FONT POINT-SIZE=\"18\">%s</FONT></TD></TR>\n", AllOperationsDumpTxt[node->value.operation]);
            break;
        case TYPE_VAR:
            fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"16\" COLOR=\"#4d545eff\"> val = </FONT> <FONT POINT-SIZE=\"18\">%s</FONT></TD></TR>\n", node->value.var.name);
            break;
    }
    fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"14\" COLOR=\"#64748B\">address: %p</FONT> </TD></TR>\n", node);
    if (node->parent == NULL)
        fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"14\" COLOR=\"#64748B\">parent: (nil)</FONT> </TD></TR>\n");
    else
        fprintf(fp, "<TR><TD COLSPAN=\"2\"> <FONT POINT-SIZE=\"14\" COLOR=\"#64748B\">parent: %p</FONT> </TD></TR>\n", node->parent);

    fprintf(fp, "<TR>\n");
    if (node->left == NULL)
        fprintf(fp, "<TD PORT=\"left\" ><FONT COLOR=\"#2563eb\">L</FONT> <FONT COLOR=\"#94a3b8\">(nil)</FONT></TD>\n");
    else
        fprintf(fp, "<TD PORT=\"left\" ><FONT COLOR=\"#2563eb\">L</FONT> <FONT COLOR=\"#94a3b8\">%p</FONT></TD>\n", node->left);
    if (node->right == NULL)
        fprintf(fp, "<TD PORT=\"right\"><FONT COLOR=\"#94a3b8\">(nil)</FONT> <FONT COLOR=\"#db2777\">R</FONT></TD>\n");
    else
        fprintf(fp, "<TD PORT=\"right\"><FONT COLOR=\"#94a3b8\">%p</FONT> <FONT COLOR=\"#db2777\">R</FONT></TD>\n", node->right);
    fprintf(fp, "</TR>\n"
                "</TABLE>>]\n"
                "}\n");
    
}

error_t create_tree_graph(DiffTree_t *tree) {
    sassert(tree, ERR_PTR_NULL);

    int counter = 0;

    FILE * fp = fopen(dump_graph_txt_file_name, "w");
    sassert(fp, ERR_PTR_NULL);

    fprintf(fp, "digraph {\n"
                "rankdir=TB\n"
                "ranksep=0.5\n"
                "node[shape=box, style=\"rounded,filled\",\n"
                "color=\"#b8caf2ff\",\n"
                "fontname=\"Inter,Helvetica,Arial\",\n"
                "fontcolor=\"#111827\",\n"
                "penwidth=1.2,\n"
                "margin=\"0.06,0.04\"]\n");
    GraphDumpPrintNode(fp, tree->root, OK);

    int count = 0;
    print_nodes_to_dump_file(tree->root, tree, fp, NO_DIRECTION, &count);
    fprintf(fp, "}");
    fclose(fp);

    char EncodeInUTF8[MAX_STR_SIZE] = {};
    snprintf(EncodeInUTF8, MAX_STR_SIZE - 1, "iconv -f cp1251 -t utf-8 graph/graph.txt > graph/graphEncoded.txt");
    if (system(EncodeInUTF8) != 0) {
        add_error(ERR_CMD_INVALID, "%s", EncodeInUTF8);
    }
    char command[MAX_STR_SIZE] = {};
    snprintf(command, MAX_STR_SIZE - 1, "dot graph/graphEncoded.txt -Gdpi=80 -Tpng -o graph/graph%d.png", count_graph_files);
    if (system(command) != 0) {
        add_error(ERR_CMD_INVALID, "%s", command);
    }

    return error;
}

void get_timed_file_name(char log_file_name[]) {
    time_t cur_time = time(NULL);
    tm *tm_info = localtime(&cur_time);

    char formatted_time[MAX_STR_SIZE] = {};
    strftime(formatted_time, MAX_STR_SIZE, "%H,%M,%S, %d.%m.%Y", tm_info);

    snprintf(log_file_name, MAX_STR_SIZE - 1, "%s/last_dump/treetxtdump_%s", tree_txt_dump_dir_name, formatted_time);
}

TreeErr print_nodes_to_dump_file(Node_t * node, DiffTree_t * tree, FILE *fp, dirType type_of_direction, int *counter) {
    sassert(node,   ERR_PTR_NULL);
    sassert(fp,     ERR_PTR_NULL);

    if (*counter > tree->num_of_nodes) {
        ADD_ERROR_AND_RETURN(ERR_INVALID_SIZE, "size: %d, expected: %zu", *counter, tree->num_of_nodes);
    }
    TreeErr err = NodeVerify(node, tree->root);
    if (err == ERR_PTR_NULL)
        return err;

    GraphDumpPrintNode(fp, node, err);

    if (err != OK) {
        fprintf(fp, "{rank=0 error%p[fillcolor=\"#d65c5ca9\", label=\"%s: \n%s\"]}\n", node, errors_text[err], error.error_info[err]);
        fprintf(fp, "tree_node_info%p->error%p\n", node, node);
    }
    (*counter)++;
    if (node->left != NULL) {
        fprintf(fp, "tree_node_info%p:left->tree_node_info%p[color=\"#2563eb\", label=\"L\", fontcolor=\"#0000ff\" , minlen=2]\n", node, node->left);
        if (err != OK)
            return err;
        print_nodes_to_dump_file(node->left, tree, fp, LEFT_DIRECTION, counter);
    }
    if (node->right != NULL) {
        fprintf(fp, "tree_node_info%p:right->tree_node_info%p[color=\"#db2777\", label=\"R\", fontcolor=\"#ff0000\" , minlen=2]\n", node, node->right);
        if (err != OK)
            return err;
        print_nodes_to_dump_file(node->right, tree, fp, RIGHT_DIRECTION, counter);
    }

    return OK;
}