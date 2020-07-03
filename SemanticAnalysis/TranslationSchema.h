// C语言词法分析器
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

struct ID {
    int intVal;
    double doubleVal;
    bool isInt;
    bool valid;
    ID() {
        intVal = 0;
        doubleVal = 0;
        isInt = valid = false;
    }
};

ID id[26];
vector<string> errorInfo;
vector<string> stack;

string nonTerminals[15] = {"program", "decls", "decl", "stmt", "compoundstmt", "stmts", "ifstmt", "assgstmt",
                           "boolexpr", "boolop", "arithexpr", "arithexprprime", "multexpr", "multexprprime",
                           "simpleexpr"};

string terminals[24] = {";", "int", "ID", "=", "INTNUM", "real", "REALNUM", "{", "}", "if", "(", ")",
                        "then", "else", "<", ">", "<=", ">=", "==", "+", "-", "*", "/", "$"};

map<string, int> nonTerminalMap;
map<string, int> terminalMap;

string analysisTable[15][24];

int cur;
string buffer;
int lineNumber;

bool leftExpr = true;
bool isInt = false;
int idIndex;
ID current;

bool add, sub, multi, divide;
bool gt, ge, lt, le, eq;

int cnt = 1;
bool jump;

void init() {
    cur = -1;
    lineNumber = 1;
    idIndex = 0;
    cnt = 1;
    leftExpr = true;
    isInt = false;
    jump = true;
    add = sub = multi = divide = false;
    gt = ge = lt = le = eq = false;
    for (int i = 0; i < 15; i++) {
        nonTerminalMap[nonTerminals[i]] = i;
    }
    for (int i = 0; i < 24; i++) {
        terminalMap[terminals[i]] = i;
    }
    analysisTable[0][1] = "decls compoundstmt"; analysisTable[0][5] = "decls compoundstmt";
    analysisTable[0][7] = "decls compoundstmt"; analysisTable[0][23] = "decls compoundstmt";
    analysisTable[1][1] = "decl ; decls"; analysisTable[1][5] = "decl ; decls"; analysisTable[1][7] = "E";
    analysisTable[2][1] = "int ID = INTNUM"; analysisTable[2][5] = "real ID = REALNUM";
    analysisTable[3][2] = "assgstmt"; analysisTable[3][7] = "compoundstmt";
    analysisTable[3][9] = "ifstmt";
    analysisTable[4][7] = "{ stmts }";
    analysisTable[5][2] = "stmt stmts"; analysisTable[5][7] = "stmt stmts"; analysisTable[5][8] = "E";
    analysisTable[5][9] = "stmt stmts";
    analysisTable[6][9] = "if ( boolexpr ) then stmt else stmt";
    analysisTable[7][2] = "ID = arithexpr ;";
    analysisTable[8][2] = "arithexpr boolop arithexpr"; analysisTable[8][4] = "arithexpr boolop arithexpr";
    analysisTable[8][6] = "arithexpr boolop arithexpr"; analysisTable[8][10] = "arithexpr boolop arithexpr";
    analysisTable[9][14] = "<"; analysisTable[9][15] = ">"; analysisTable[9][16] = "<="; analysisTable[9][17] = ">=";
    analysisTable[9][18] = "==";
    analysisTable[10][2] = "multexpr arithexprprime"; analysisTable[10][4] = "multexpr arithexprprime";
    analysisTable[10][6] = "multexpr arithexprprime"; analysisTable[10][10] = "multexpr arithexprprime";
    analysisTable[11][0] = "E"; analysisTable[11][11] = "E"; analysisTable[11][14] = "E";
    analysisTable[11][15] = "E"; analysisTable[11][16] = "E"; analysisTable[11][17] = "E";
    analysisTable[11][18] = "E"; analysisTable[11][19] = "+ multexpr arithexprprime";
    analysisTable[11][20] = "- multexpr arithexprprime";
    analysisTable[12][2] = "simpleexpr multexprprime"; analysisTable[12][4] = "simpleexpr multexprprime";
    analysisTable[12][6] = "simpleexpr multexprprime"; analysisTable[12][10] = "simpleexpr multexprprime";
    analysisTable[13][0] = "E"; analysisTable[13][11] = "E"; analysisTable[13][14] = "E";
    analysisTable[13][15] = "E"; analysisTable[13][16] = "E"; analysisTable[13][17] = "E";
    analysisTable[13][18] = "E"; analysisTable[13][19] = "E"; analysisTable[13][20] = "E";
    analysisTable[13][21] = "* simpleexpr multexprprime"; analysisTable[13][22] = "/ simpleexpr multexprprime";
    analysisTable[14][2] = "ID"; analysisTable[14][4] = "INTNUM"; analysisTable[14][6] = "REALNUM";
    analysisTable[14][10] = "( arithexpr )";
}

bool isDigit(char ch) {
    return ('0' <= ch) && (ch <= '9');
}

bool isLowerCase(char ch) {
    return ('a' <= ch) && (ch <= 'z');
}

bool isNonTerminal(string& token) {
    return nonTerminalMap.find(token) != nonTerminalMap.end();
}

void error(int type) {
    string info;
    switch (type) {
        case 0:
            info = "error message:line " + to_string(lineNumber) + ",division by zero";
            errorInfo.push_back(info);
            break;
        case 1:
            info = "error message:line " + to_string(lineNumber) + ",realnum can not be translated into int type";
            errorInfo.push_back(info);
            break;
        default:
            info = "error message:line " + to_string(lineNumber);
            errorInfo.push_back(info);
            break;
    }
//    printf("error %s\n", buffer.c_str());
}

void moveNext(string& prog) {
    cur++;
    while (prog[cur] == ' ' || prog[cur] == '\t' || prog[cur] == '\n') {
        if (prog[cur] == '\n') {
            lineNumber++;
        }
        cur++;
    }
    if (prog[cur] == '>') {
        buffer = string(1, prog[cur]);
        if (prog[cur + 1] == '=') {
            buffer += prog[cur + 1];
            cur++;
            ge = true;
        } else {
            gt = true;
        }
        leftExpr = false;
    } else if (prog[cur] == '<') {
        buffer = string(1, prog[cur]);
        if (prog[cur + 1] == '=') {
            buffer += prog[cur + 1];
            cur++;
            le = true;
        } else {
            lt = true;
        }
        leftExpr = false;
    } else if (prog[cur] == '=') {
        buffer = string(1, prog[cur]);
        if (prog[cur + 1] == '=') {
            buffer += prog[cur + 1];
            cur++;
            eq = true;
        }
        leftExpr = false;
    } else if (prog[cur] == 'i') {
        buffer = string(1, prog[cur]);
        if (prog[cur + 1] == 'f') {
            buffer += prog[cur + 1];
            cur++;
        } else {
            buffer += prog[cur + 1];
            buffer += prog[cur + 2];
            cur += 2;
            isInt = true;
        }
    } else if (prog[cur] == 'r' || prog[cur] == 't' || prog[cur] == 'e') {
        if (prog[cur + 1] == 'e') {
            isInt = false;
        }
        buffer = prog.substr(cur, 4);
        cur += 3;
    } else if (prog[cur] == '+') {
        add = true;
        buffer = string(1, prog[cur]);
    } else if (prog[cur] == '-') {
        sub = true;
        buffer = string(1, prog[cur]);
    } else if (prog[cur] == '*') {
        multi = true;
        buffer = string(1, prog[cur]);
    } else if (prog[cur] == '/') {
        divide = true;
        buffer = string(1, prog[cur]);
    } else if (prog[cur] == ')') {
        leftExpr = true;
        buffer = string(1, prog[cur]);
    } else if (isLowerCase(prog[cur])) {
        buffer = "ID";
        if (leftExpr) {
            idIndex = prog[cur] - 'a';
            if (isInt) {
                id[idIndex].isInt = isInt;
            }
            id[idIndex].valid = true;
        } else {
            int pos = prog[cur] - 'a';
            if (sub) {
                if (id[idIndex].isInt) {
                    current.intVal -= id[pos].intVal;
                } else {
                    current.doubleVal -= id[pos].doubleVal;
                }
                sub = false;
            } else if (multi) {
                if (id[idIndex].isInt) {
                    current.intVal *=  id[pos].intVal;
                } else {
                    current.doubleVal *= (id[pos].doubleVal == 0) ? id[pos].intVal : id[pos].doubleVal;
                }
                multi = false;
            } else if (divide) {
                if (id[idIndex].isInt && id[pos].intVal != 0) {
                    current.intVal /= id[pos].intVal;
                } else if (!id[idIndex].isInt && id[pos].doubleVal != 0) {
                    current.doubleVal /= id[pos].doubleVal;
                }
                divide = false;
            } else if (ge) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal < id[pos].intVal;
                } else {
                    jump = id[idIndex].doubleVal < id[pos].doubleVal;
                }
                if (jump) {
                    cnt = 0;
                }
                ge = false;
            } else if (gt) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal <= id[pos].intVal;
                } else {
                    jump = id[idIndex].doubleVal <= id[pos].doubleVal;
                }
                if (jump) {
                    cnt = 0;
                }
                gt = false;
            } else if (le) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal > id[pos].intVal;
                } else {
                    jump = id[idIndex].doubleVal > id[pos].doubleVal;
                }
                if (jump) {
                    cnt = 0;
                }
                le = false;
            } else if (lt) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal >= id[pos].intVal;
                } else {
                    jump = id[idIndex].doubleVal >= id[pos].doubleVal;
                }
                if (jump) {
                    cnt = 0;
                }
                lt  = false;
            } else if (eq) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal != id[pos].intVal;
                } else {
                    jump = id[idIndex].doubleVal != id[pos].doubleVal;
                }
                if (jump) {
                    cnt = 0;
                }
                eq = false;
            } else {
                if (add) {
                    add = false;
                }
                if (id[idIndex].isInt) {
                    current.intVal += id[pos].intVal;
                } else {
//                    printf("pos: %lf\n", id[pos].doubleVal);
                    current.doubleVal += id[pos].doubleVal;
                }
            }
        }
    } else if (prog[cur] == ';') {
        buffer = string(1, prog[cur]);
        leftExpr = true;
        if (cnt != 0) {
            if (id[idIndex].isInt) {
                id[idIndex].intVal = current.intVal;
            } else {
                id[idIndex].doubleVal = current.doubleVal;
//                printf("%.2f %s\n", id[idIndex].doubleVal, id[idIndex].isInt ? "YES" : "NO");
            }
            if (!jump) {
                cnt--;
                jump = true;
            }
        } else {
            cnt++;
        }
        current.intVal = 0;
        current.doubleVal = 0;
    } else if (isDigit(prog[cur])) {
        int integerPart = 0;
        while (isDigit(prog[cur])) {
            integerPart = 10 * integerPart + (prog[cur] - '0');
            cur++;
        }
        if (prog[cur] != '.') {
            buffer = "INTNUM";
            cur--;
            if (sub) {
                if (id[idIndex].isInt) {
                    current.intVal -= integerPart;
                } else {
                    current.doubleVal -= integerPart;
                }
                sub = false;
            } else if (multi) {
                if (id[idIndex].isInt) {
                    current.intVal *= integerPart;
                } else {
                    current.doubleVal *= integerPart;
                }
                multi = false;
            } else if (divide) {
                if (integerPart == 0) {
                    error(0);
                    integerPart = 1;
                }
                if (id[idIndex].isInt) {
                    current.intVal /= integerPart;
                } else {
                    current.doubleVal /= integerPart;
//                    printf("current: %lf, integerPart: %d\n", current.doubleVal, integerPart);
                }
                divide = false;
            } else if (gt) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal <= integerPart;
                } else {
                    jump = id[idIndex].doubleVal <= integerPart;
                }
                if (jump) {
                    cnt = 0;
                }
                gt = false;
            } else if (ge) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal < integerPart;
                } else {
                    jump = id[idIndex].doubleVal < integerPart;
                }
                if (jump) {
                    cnt = 0;
                }
                ge = false;
            } else if (lt) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal >= integerPart;
                } else {
                    jump = id[idIndex].doubleVal >= integerPart;
                }
                if (jump) {
                    cnt = 0;
                }
                lt = false;
            } else if (le) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal > integerPart;
                } else {
                    jump = id[idIndex].doubleVal > integerPart;
                }
                if (jump) {
                    cnt = 0;
                }
                le = false;
            } else if (eq) {
                if (id[idIndex].isInt) {
                    jump = id[idIndex].intVal != integerPart;
                } else {
                    jump = id[idIndex].doubleVal != integerPart;
                }
                if (jump) {
                    cnt = 0;
                }
                eq = false;
            } else {
                if (add) {
                    add = false;
                }
                if (id[idIndex].isInt) {
                    current.intVal += integerPart;
                } else {
                    current.doubleVal += integerPart;
                }
            }
        } else {
            if (id[idIndex].isInt) {
                error(1);
                id[idIndex].isInt = false;
                id[idIndex].doubleVal = id[idIndex].intVal;
                id[idIndex].intVal = 0;
                current.doubleVal = current.intVal;
                current.intVal = 0;
                buffer = "INTNUM";
            } else {
                buffer = "REALNUM";
            }
            double power = 10;
            double decimalPart = 0;
            cur++;
            while (isDigit(prog[cur])) {
                decimalPart += (prog[cur] - '0') / power;
                power *= 10;
                cur++;
            }
            cur--;
            double val = integerPart + decimalPart;
            if (sub) {
                current.doubleVal -= val;
                sub = false;
            } else if (multi) {
                current.doubleVal *= val;
                multi = false;
            } else if (divide) {
                if (val == 0) {
                    error(0);
                    val = 1;
                }
                current.doubleVal /= val;
                divide = false;
            } else if (ge) {
                jump = id[idIndex].doubleVal < val;
                if (jump) {
                    cnt = 0;
                }
                ge = false;
            } else if (gt) {
                jump = id[idIndex].doubleVal <= val;
                if (jump) {
                    cnt = 0;
                }
                gt = false;
            } else if (le) {
                jump = id[idIndex].doubleVal > val;
                if (jump) {
                    cnt = 0;
                }
                le = false;
            } else if (lt) {
                jump = id[idIndex].doubleVal >= val;
                if (jump) {
                    cnt = 0;
                }
                lt = false;
            } else if (eq) {
                jump = id[idIndex].doubleVal != val;
                if (jump) {
                    cnt = 0;
                }
                eq = false;
            } else {
                current.doubleVal += val;
                if (add) {
                    add = false;
                }
            }
        }
    } else {
        buffer = string(1, prog[cur]);
    }
}

int getCol(string& terminal) {
    if (terminalMap.find(terminal) == terminalMap.end()) {
        return -1;
    }
    return terminalMap[terminal];
}

int getRow(string& nonTerminal) {
    if (nonTerminalMap.find(nonTerminal) == nonTerminalMap.end()) {
        return -1;
    }
    return nonTerminalMap[nonTerminal];
}

void pushStack(int row, int col) {
    stack.pop_back();
    switch (row) {
        case 0:
            stack.push_back("compoundstmt");
            stack.push_back("decls");
            break;
        case 1:
            if (col == 1 || col == 5) {
                stack.push_back("decls");
                stack.push_back(";");
                stack.push_back("decl");
            }
            break;
        case 2:
            if (col == 1) {
                stack.push_back("INTNUM");
                stack.push_back("=");
                stack.push_back("ID");
                stack.push_back("int");
            } else {
                stack.push_back("REALNUM");
                stack.push_back("=");
                stack.push_back("ID");
                stack.push_back("real");
            }
            break;
        case 3:
            if (col == 2) {
                stack.push_back("assgstmt");
            } else if (col == 7) {
                stack.push_back("compoundstmt");
            } else {
                stack.push_back("ifstmt");
            }
            break;
        case 4:
            stack.push_back("}");
            stack.push_back("stmts");
            stack.push_back("{");
            break;
        case 5:
            if (col == 2 || col == 7 || col == 9) {
                stack.push_back("stmts");
                stack.push_back("stmt");
            }
            break;
        case 6:
            stack.push_back("stmt");
            stack.push_back("else");
            stack.push_back("stmt");
            stack.push_back("then");
            stack.push_back(")");
            stack.push_back("boolexpr");
            stack.push_back("(");
            stack.push_back("if");
            break;
        case 7:
            stack.push_back(";");
            stack.push_back("arithexpr");
            stack.push_back("=");
            stack.push_back("ID");
            break;
        case 8:
            stack.push_back("arithexpr");
            stack.push_back("boolop");
            stack.push_back("arithexpr");
            break;
        case 9:
            if (col == 14) {
                stack.push_back("<");
            } else if (col == 15) {
                stack.push_back(">");
            } else if (col == 16) {
                stack.push_back("<=");
            } else if (col == 17) {
                stack.push_back(">=");
            } else {
                stack.push_back("==");
            }
            break;
        case 10:
            stack.push_back("arithexprprime");
            stack.push_back("multexpr");
            break;
        case 11:
            if (col == 19) {
                stack.push_back("arithexprprime");
                stack.push_back("multexpr");
                stack.push_back("+");
            } else if (col == 20) {
                stack.push_back("arithexprprime");
                stack.push_back("multexpr");
                stack.push_back("-");
            }
            break;
        case 12:
            stack.push_back("multexprprime");
            stack.push_back("simpleexpr");
            break;
        case 13:
            if (col == 21) {
                stack.push_back("multexprprime");
                stack.push_back("simpleexpr");
                stack.push_back("*");
            } else if (col == 22) {
                stack.push_back("multexprprime");
                stack.push_back("simpleexpr");
                stack.push_back("/");
            }
            break;
        case 14:
            if (col == 2) {
                stack.push_back("ID");
            } else if (col == 4) {
                stack.push_back("INTNUM");
            } else if (col == 6) {
                stack.push_back("REALNUM");
            } else {
                stack.push_back(")");
                stack.push_back("arithexpr");
                stack.push_back("(");
            }
        default:
            break;
    }
}

void LL(string& prog) {
    moveNext(prog);
    stack.push_back("$");
    stack.push_back("program");
    string top = stack.back();
    while (stack.size() > 1) {
        if (top == buffer) {
            stack.pop_back();
            moveNext(prog);
        } else if (!isNonTerminal(top)) {
            error(2);
        } else {
            int row = getRow(top);
            int col = getCol(buffer);
            if (row == -1 || col == -1 || analysisTable[row][col].empty()) {
                error(2);
            } else {
                pushStack(row, col);
            }
        }
        top = stack.back();
    }
}

/* 不要修改这个标准输入函数 */
void read_prog(string& prog)
{
    char c;
    while(scanf("%c",&c)!=EOF){
        prog += c;
    }
}
/* 你可以添加其他函数 */

void Analysis()
{
    string prog;
    read_prog(prog);
    /* 骚年们 请开始你们的表演 */
    /********* Begin *********/

    init();

    LL(prog);

    if (errorInfo.empty()) {
        for (int i = 0; i < 26; i++) {
            if (id[i].valid) {
                if (id[i].isInt) {
                    printf("%c: %d", 'a' + i, id[i].intVal);
                } else {
                    printf("%c: %g", 'a' + i, id[i].doubleVal);
                }
                if (i != 2) {
                    printf("\n");
                }
            }
        }
    } else {
        for (int i = 0; i < errorInfo.size(); i++) {
            printf("%s", errorInfo[i].c_str());
            if (i != errorInfo.size() - 1) {
                printf("\n");
            }
        }
    }
    
    /********* End *********/
    
}