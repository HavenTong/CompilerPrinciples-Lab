// C语言词法分析器
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
using namespace std;

struct Node {
    string token;
    int level;
    Node() {

    }
    Node(string token, int level): token(token), level(level) {}
};
vector<Node> tree;
vector<Node> stack;
vector<string> errorInfo;
string nonTerminals[14] = {"program", "stmt", "compoundstmt", "stmts", "ifstmt", "whilestmt", "assgstmt",
                           "boolexpr", "boolop", "arithexpr", "arithexprprime", "multexpr",
                           "multexprprime", "simpleexpr"};
string terminals[22] = {"{", "}", "(", ")", "i", "t", "e", "w", ";", "<", ">", "<=", ">=", "==", "+",
                        "-", "*", "/", "I", "N", "=", "$"};
map<string, int> nonTerminalMap;
map<string, int> terminalMap;
string analysisTable[14][22];
int cur;
string buffer;
int lineNumber;


/* 不要修改这个标准输入函数 */
void read_prog(string& prog)
{
    char c;
    while(scanf("%c",&c)!=EOF){
        prog += c;
    }
}
/* 你可以添加其他函数 */

void init() {
    cur = 0;
    lineNumber = 0;
    for (int i = 0; i < 14; i++) {
        nonTerminalMap[nonTerminals[i]] = i;
    }
    for (int i = 0; i < 22; i++) {
        terminalMap[terminals[i]] = i;
    }
    analysisTable[0][0] = "compoundstmt";
    analysisTable[1][0] = "compundstmt"; analysisTable[1][4] = "ifstmt";
    analysisTable[1][7] = "whilestmt"; analysisTable[1][18] = "assgstmt";
    analysisTable[2][0] = "{stmts}";
    analysisTable[3][0] = "stmt stmts"; analysisTable[3][1] = "E";
    analysisTable[3][4] =  analysisTable[3][7] = analysisTable[3][18] = "stmt stmts";
    analysisTable[4][4] = "if (boolexpr) then stmt else stmt";
    analysisTable[5][7] = "while (boolexpr) stmt";
    analysisTable[6][18]  = "ID = arithexpr;";
    analysisTable[7][2] = analysisTable[7][18] = analysisTable[7][19] = "arithexpr boolop arithexpr";
    analysisTable[8][9] = "<"; analysisTable[8][10] = ">"; analysisTable[8][11] = "<=";
    analysisTable[8][12] = ">="; analysisTable[8][13] = "==";
    analysisTable[9][2] = analysisTable[9][18] = analysisTable[9][19] = "multexpr arithexprprime";
    analysisTable[10][3] = analysisTable[10][8] = analysisTable[10][9] = analysisTable[10][10] = "E";
    analysisTable[10][11] = analysisTable[10][12] = analysisTable[10][13] = "E";
    analysisTable[10][14] = "+ multexpr arithexprprime"; analysisTable[10][15] = "- multexpr arithexprprime";
    analysisTable[11][2] = analysisTable[11][18] = analysisTable[11][19] = "simpleexpr multexprprime";
    analysisTable[12][3] = analysisTable[12][8] = analysisTable[12][9] = analysisTable[12][10] = "E";
    analysisTable[12][11] = analysisTable[12][12] = analysisTable[12][13] = analysisTable[12][14] = analysisTable[12][15] = "E";
    analysisTable[12][16] = "* simpleexpr multexprprime";
    analysisTable[12][17] = "/ simpleexpr multexprprime";
    analysisTable[13][2] = "(arithexpr)";
    analysisTable[13][18] = "ID"; analysisTable[13][19] = "NUM";

}

bool isNonTerminal(string& token) {
    return nonTerminalMap.find(token) != nonTerminalMap.end();
}

void moveNext(string& prog) {
    cur++;
    while (cur < prog.size() && (prog[cur] == ' ' || prog[cur] == '\t' || prog[cur] == '\n')) {
        if (prog[cur] == '\n') {
            lineNumber++;
        }
        cur++;
    }
//    if (prog[cur] == '=' && prog[cur + 1] == '=') {
//        cur++;
//        buffer = "==";
//    } else if (prog[cur]) {
//        buffer = string(1, prog[cur]);
//    }
    if ((prog[cur] == '=' || prog[cur] == '>' || prog[cur] == '<') && (prog[cur + 1] == '=')) {
        buffer = string(1, prog[cur]);
        buffer += prog[cur + 1];
        cur++;
    } else {
        buffer = string(1, prog[cur]);
    }
}

void error(Node& lastToken, bool isTerminal) {
    if (isTerminal) {
        string info = "语法错误,第" + to_string(lineNumber) + "行,缺少" + "\"" + lastToken.token + "\"";
        errorInfo.push_back(info);
        stack.pop_back();
        tree.push_back(lastToken);
    } else {
        int row = nonTerminalMap[lastToken.token];
        bool defaultEmpty = false;
        for (int j = 0; j < 22; j++) {
            if (analysisTable[row][j] == "E") {
                defaultEmpty = true;
                break;
            }
        }
        if (defaultEmpty) {
            tree.push_back(lastToken);
            tree.push_back(Node("E", lastToken.level + 1));
            stack.pop_back();
        }
    }
//    string info = "语法错误,第" + to_string(lineNumber) + "行,缺少" + lastToken.token;
//    errorInfo.push_back(info);
//    stack.push_back(Node(buffer, lastToken.level));
//    tree.push_back(lastToken);
}

int getRow(string& nonTerminal) {
    return nonTerminalMap[nonTerminal];
}

int getCol(string& terminal) {
    return terminalMap[terminal];
}

void pushStack(Node& top, int row, int col) {
    tree.push_back(top);
    int level = top.level;
    level++;
    stack.pop_back();
    switch (row) {
        case 0:
            stack.push_back(Node("compoundstmt", level));
            break;
        case 1:
            if (col == 0) {
                stack.push_back(Node("compoundstmt", level));
            } else if (col == 4) {
                stack.push_back(Node("ifstmt", level));
            } else if (col == 7) {
                stack.push_back(Node("whilestmt", level));
            } else {
                stack.push_back(Node("assgstmt", level));
            }
            break;
        case 2:
            stack.push_back(Node("}", level));
            stack.push_back(Node("stmts", level));
            stack.push_back(Node("{", level));
            break;
        case 3:
            if (col == 0 || col == 4 || col == 7 || col == 18) {
                stack.push_back(Node("stmts", level));
                stack.push_back(Node("stmt", level));
            } else if (col == 1) {
                tree.push_back(Node("E", level));
            }
            break;
        case 4:
            stack.push_back(Node("stmt", level));
            stack.push_back(Node("e", level));
            stack.push_back(Node("s", level));
            stack.push_back(Node("l", level));
            stack.push_back(Node("e", level));
            stack.push_back(Node("stmt", level));
            stack.push_back(Node("n", level));
            stack.push_back(Node("e", level));
            stack.push_back(Node("h", level));
            stack.push_back(Node("t", level));
            stack.push_back(Node(")", level));
            stack.push_back(Node("boolexpr", level));
            stack.push_back(Node("(", level));
            stack.push_back(Node("f", level));
            stack.push_back(Node("i", level));
            break;
        case 5:
            stack.push_back(Node("stmt", level));
            stack.push_back(Node(")", level));
            stack.push_back(Node("boolexpr", level));
            stack.push_back(Node("(", level));
            stack.push_back(Node("e", level));
            stack.push_back(Node("l", level));
            stack.push_back(Node("i", level));
            stack.push_back(Node("h", level));
            stack.push_back(Node("w", level));
            break;
        case 6:
            stack.push_back(Node(";", level));
            stack.push_back(Node("arithexpr", level));
            stack.push_back(Node("=", level));
            stack.push_back(Node("D", level));
            stack.push_back(Node("I", level));
            break;
        case 7:
            stack.push_back(Node("arithexpr", level));
            stack.push_back(Node("boolop", level));
            stack.push_back(Node("arithexpr", level));
            break;
        case 8:
            if (col == 9) {
                stack.push_back(Node("<", level));
            } else if (col == 10) {
                stack.push_back(Node(">", level));
            } else if (col == 11) {
//                stack.push_back(Node("=", level));
//                stack.push_back(Node("<", level));
                stack.push_back(Node("<=", level));
            } else if (col == 12) {
//                stack.push_back(Node("=", level));
//                stack.push_back(Node(">", level));
                stack.push_back(Node(">=", level));
            } else {
                stack.push_back(Node("==", level));
//                stack.push_back(Node("=", level));
            }
            break;
        case 9:
            stack.push_back(Node("arithexprprime", level));
            stack.push_back(Node("multexpr", level));
            break;
        case 10:
            if (col == 14) {
                stack.push_back(Node("arithexprprime", level));
                stack.push_back(Node("multexpr", level));
                stack.push_back(Node("+", level));
            } else if (col == 15) {
                stack.push_back(Node("arithexprprime", level));
                stack.push_back(Node("multexpr", level));
                stack.push_back(Node("-", level));
            } else {
                tree.push_back(Node("E", level));
            }
            break;
        case 11:
            stack.push_back(Node("multexprprime", level));
            stack.push_back(Node("simpleexpr", level));
            break;
        case 12:
            if (col == 16) {
                stack.push_back(Node("multexprprime", level));
                stack.push_back(Node("simpleexpr", level));
                stack.push_back(Node("*", level));
            } else if (col == 17) {
                stack.push_back(Node("multexprprime", level));
                stack.push_back(Node("simpleexpr", level));
                stack.push_back(Node("/", level));
            } else {
                tree.push_back(Node("E", level));
            }
            break;
        case 13:
            if (col == 2) {
                stack.push_back(Node(")", level));
                stack.push_back(Node("arithexpr", level));
                stack.push_back(Node("(", level));
            } else if (col == 18) {
                stack.push_back(Node("D", level));
                stack.push_back(Node("I", level));
            } else {
                stack.push_back(Node("M", level));
                stack.push_back(Node("U", level));
                stack.push_back(Node("N", level));
            }
            break;
        default:
            break;
    }
}

void LL(string& prog) {
    stack.push_back(Node("$", 0));
    stack.push_back(Node("program", 0));
    buffer = string(1, prog[cur]);
    Node top = stack.back();
    int cnt = 0;
    while (stack.size() > 1) {
        if (top.token == buffer) {
            cnt++;
            if (top.token == "e") {
                if (cnt == 4) {
                    tree.push_back(Node("else", top.level));
                    cnt = 0;
                } else if (cnt == 5) {
                    tree.push_back(Node("while", top.level));
                    cnt = 0;
                }
            } else if (top.token == "f") {
                tree.push_back(Node("if", top.level));
                cnt = 0;
            } else if (top.token == "n") {
                tree.push_back(Node("then", top.level));
                cnt = 0;
            } else if (top.token == "D") {
                tree.push_back(Node("ID", top.level));
                cnt = 0;
            } else if (top.token == "M") {
                tree.push_back(Node("NUM", top.level));
                cnt = 0;
            } else if (top.token == "=") {
//                Node lastNode = tree.back();
//                if (lastNode.token == "<" || lastNode.token == ">") {
//                    lastNode.token += '=';
//                } else {
//                    tree.push_back(Node("=", top.level));
//                }
                tree.push_back(Node("=", top.level));
            } else if (top.token == "{" || top.token == "}" || top.token == "(" || top.token == ")"
            || top.token == ";" || top.token == "<" || top.token == ">" || top.token == "+" || top.token == "-"
            || top.token == "*" || top.token == "/" || top.token == "==" || top.token == ">=" || top.token == "<=") {
                tree.push_back(Node(top.token, top.level));
                cnt = 0;
            }
            moveNext(prog);
            stack.pop_back();
        } else if (!isNonTerminal(top.token)) {
            error(top, true);
        } else {
            int row = getRow(top.token);
            int col = getCol(buffer);
            if (row == -1 || col == -1 || analysisTable[row][col].empty()) {
//                printf("error: row: %d col: %d line: %d token: %s buffer: %s\n", row, col, lineNumber, top.token.c_str(), buffer.c_str());
                error(top, false);
            } else {
                pushStack(top, row, col);
            }
        }
        top = stack.back();
    }
}

void Analysis()
{

    string prog;
    read_prog(prog);
    /* 骚年们 请开始你们的表演 */
    /********* Begin *********/
    prog += '$';
    init();
    LL(prog);
    for (int i = 0; i < errorInfo.size(); i++) {
        printf("%s\n", errorInfo[i].c_str());
    }
    for (int i = 0; i < tree.size(); i++) {
        for (int j = 0; j < tree[i].level; j++) {
            printf("\t");
        }
        printf("%s%s", tree[i].token.c_str(), (i == tree.size() - 1) ? "" : "\n");
    }
    /********* End *********/

}