// C语言词法分析器
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <set>
using namespace std;


vector<vector<string>> reduce;

vector<string> errorInfo;

vector<string> products[30];

pair<char, int> ACTION[60][25];
int GOTO[60][15];

string terminals[22] = {"{", "}", "if", "(", ")", "then", "else", "while", "ID", "=", ";",
                        "<", ">", "<=", ">=", "==", "+", "-", "*", "/", "NUM", "$"};
string nonTerminals[14] = {"program", "stmt", "compoundstmt", "stmts", "ifstmt", "whilestmt",
                           "assgstmt", "boolexpr", "boolop", "arithexpr", "arithexprprime",
                           "multexpr", "multexprprime", "simpleexpr"};

map<string, int> terminalMap;
map<string, int> nonTerminalMap;

vector<int> stack;

int lineNumber;
int cur;
string buffer;

void init() {
    lineNumber = 0;
    cur = -1;
    for (int i = 0; i < 22; i++) {
        terminalMap[terminals[i]] = i;
    }
    for (int i = 0; i < 14; i++) {
        nonTerminalMap[nonTerminals[i]] = i;
    }
    // init products
    products[1] = vector<string>({"program", "compoundstmt"});
    products[2] = vector<string>({"stmt", "ifstmt"});
    products[3] = vector<string>({"stmt", "whilestmt"});
    products[4] = vector<string>({"stmt", "assgstmt"});
    products[5] = vector<string>({"stmt", "compoundstmt"});
    products[6] = vector<string>({"compoundstmt", "{", "stmts", "}"});
    products[7] = vector<string>({"stmts", "stmt", "stmts"});
    products[8] = vector<string>({"stmts"});
    products[9] = vector<string>({"ifstmt", "if", "(", "boolexpr", ")", "then", "stmt", "else", "stmt"});
    products[10] = vector<string>({"whilestmt", "while", "(", "boolexpr", ")", "stmt"});
    products[11] = vector<string>({"assgstmt", "ID", "=", "arithexpr", ";"});
    products[12] = vector<string>({"boolexpr", "arithexpr", "boolop", "arithexpr"});
    products[13] = vector<string>({"boolop", "<"});
    products[14] = vector<string>({"boolop", ">"});
    products[15] = vector<string>({"boolop", "<="});
    products[16] = vector<string>({"boolop", ">="});
    products[17] = vector<string>({"boolop", "=="});
    products[18] = vector<string>({"arithexpr", "multexpr", "arithexprprime"});
    products[19] = vector<string>({"arithexprprime", "+", "multexpr", "arithexprprime"});
    products[20] = vector<string>({"arithexprprime", "-", "multexpr", "arithexprprime"});
    products[21] = vector<string>({"arithexprprime"});
    products[22] = vector<string>({"multexpr", "simpleexpr", "multexprprime"});
    products[23] = vector<string>({"multexprprime", "*", "simpleexpr", "multexprprime"});
    products[24] = vector<string>({"multexprprime", "/", "simpleexpr", "multexprprime"});
    products[25] = vector<string>({"multexprprime"});
    products[26] = vector<string>({"simpleexpr", "ID"});
    products[27] = vector<string>({"simpleexpr", "NUM"});
    products[28] = vector<string>({"simpleexpr", "(", "arithexpr", ")"});

    // init GOTO
    GOTO[0][0] = 1; GOTO[0][2] = 2;
    GOTO[3][1] = 5; GOTO[3][2] = 9; GOTO[3][3] = 4; GOTO[3][4] = 6; GOTO[3][5] = 7; GOTO[3][6] = 8;
    GOTO[5][1] = 5; GOTO[5][2] = 9; GOTO[5][3] = 14; GOTO[5][4] = 6; GOTO[5][5] = 7; GOTO[5][6] = 8;
    GOTO[15][7] = 18; GOTO[15][9] = 19; GOTO[15][11] = 20; GOTO[15][13] = 21;
    GOTO[16][7] = 25; GOTO[16][9] = 19; GOTO[16][11] = 20; GOTO[16][13] = 21;
    GOTO[17][9] = 26; GOTO[17][11] = 20; GOTO[17][13] = 21;
    GOTO[19][8] = 28;
    GOTO[20][10] = 34;
    GOTO[21][12] = 37;
    GOTO[24][9] = 40; GOTO[24][11] = 20; GOTO[24][13] = 21;
    GOTO[28][9] = 44; GOTO[28][11] = 20; GOTO[28][13] = 21;
    GOTO[35][11] = 45; GOTO[35][13] = 21;
    GOTO[36][11] = 46; GOTO[36][13] = 21;
    GOTO[38][13] = 47;
    GOTO[39][13] = 48;
    GOTO[41][1] = 50; GOTO[41][2] = 9; GOTO[41][4] = 6; GOTO[41][5] = 7; GOTO[41][6] = 8;
    GOTO[43][1] = 51; GOTO[43][2] = 9; GOTO[43][4] = 6; GOTO[43][5] = 7; GOTO[43][6] = 8;
    GOTO[45][10] = 52;
    GOTO[46][10] = 53;
    GOTO[47][12] = 54;
    GOTO[48][12] = 55;
    GOTO[56][1] = 57; GOTO[56][2] = 9; GOTO[56][4] = 6; GOTO[56][5] = 7; GOTO[56][6] = 8;

    // init ACTION
    ACTION[0][0].first = 's'; ACTION[0][0].second = 3;
    ACTION[1][21].first = 'a';
    ACTION[2][21].first = 'r'; ACTION[2][21].second = 1;
    ACTION[3][0].first = 's'; ACTION[3][0].second = 3; ACTION[3][1].first = 'r'; ACTION[3][1].second = 8;
    ACTION[3][2].first = 's'; ACTION[3][2].second = 10; ACTION[3][7].first = 's'; ACTION[3][7].second = 11;
    ACTION[3][8].first = 's'; ACTION[3][8].second = 12;
    ACTION[4][1].first = 's'; ACTION[4][1].second = 13;
    ACTION[5][0].first = 's'; ACTION[5][0].second = 3; ACTION[5][1].first = 'r'; ACTION[5][1].second = 8;
    ACTION[5][2].first = 's'; ACTION[5][2].second = 10; ACTION[5][7].first = 's'; ACTION[5][7].second = 11;
    ACTION[5][8].first = 's'; ACTION[5][8].second = 12;
    ACTION[6][0].first = 'r'; ACTION[6][0].second = 2; ACTION[6][1].first = 'r'; ACTION[6][1].second = 2;
    ACTION[6][2].first = 'r'; ACTION[6][2].second = 2; ACTION[6][6].first = 'r'; ACTION[6][6].second = 2;
    ACTION[6][7].first = 'r'; ACTION[6][7].second = 2; ACTION[6][8].first = 'r'; ACTION[6][8].second = 2;
    ACTION[7][0].first = 'r'; ACTION[7][0].second = 3; ACTION[7][1].first = 'r'; ACTION[7][1].second = 3;
    ACTION[7][2].first = 'r'; ACTION[7][2].second = 3; ACTION[7][6].first = 'r'; ACTION[7][6].second = 3;
    ACTION[7][7].first = 'r'; ACTION[7][7].second = 3; ACTION[7][8].first = 'r'; ACTION[7][8].second = 3;
    ACTION[8][0].first = 'r'; ACTION[8][0].second = 4; ACTION[8][1].first = 'r'; ACTION[8][1].second = 4;
    ACTION[8][2].first = 'r'; ACTION[8][2].second = 4; ACTION[8][6].first = 'r'; ACTION[8][6].second = 4;
    ACTION[8][7].first = 'r'; ACTION[8][7].second = 4; ACTION[8][8].first = 'r'; ACTION[8][8].second = 4;
    ACTION[9][0].first = 'r'; ACTION[9][0].second = 5; ACTION[9][1].first = 'r'; ACTION[9][1].second = 5;
    ACTION[9][2].first = 'r'; ACTION[9][2].second = 5; ACTION[9][6].first = 'r'; ACTION[9][6].second = 5;
    ACTION[9][7].first = 'r'; ACTION[9][7].second = 5; ACTION[9][8].first = 'r'; ACTION[9][8].second = 5;
    ACTION[10][3].first = 's'; ACTION[10][3].second = 15;
    ACTION[11][3].first = 's'; ACTION[11][3].second = 16;
    ACTION[12][9].first = 's'; ACTION[12][9].second = 17;
    ACTION[13][0].first = 'r'; ACTION[13][0].second = 6; ACTION[13][1].first = 'r'; ACTION[13][1].second = 6;
    ACTION[13][2].first = 'r'; ACTION[13][2].second = 6; ACTION[13][6].first = 'r'; ACTION[13][6].second = 6;
    ACTION[13][7].first = 'r'; ACTION[13][7].second = 6; ACTION[13][8].first = 'r'; ACTION[13][8].second = 6;
    ACTION[13][21].first = 'r'; ACTION[13][21].second = 6;
    ACTION[14][1].first = 'r'; ACTION[14][1].second = 7;
    ACTION[15][3].first = 's'; ACTION[15][3].second = 24; ACTION[15][8].first = 's'; ACTION[15][8].second = 22;
    ACTION[15][20].first = 's'; ACTION[15][20].second = 23;
    ACTION[16][3].first = 's'; ACTION[16][3].second = 24; ACTION[16][8].first = 's'; ACTION[16][8].second = 22;
    ACTION[16][20].first = 's'; ACTION[16][20].second = 23;
    ACTION[17][3].first = 's'; ACTION[17][3].second = 24; ACTION[17][8].first = 's'; ACTION[17][8].second = 22;
    ACTION[17][20].first = 's'; ACTION[17][20].second = 23;
    ACTION[18][4].first = 's'; ACTION[18][4].second = 27;
    ACTION[19][11].first = 's'; ACTION[19][11].second = 29; ACTION[19][12].first = 's'; ACTION[19][12].second = 30;
    ACTION[19][13].first = 's'; ACTION[19][13].second = 31; ACTION[19][14].first = 's'; ACTION[19][14].second = 32;
    ACTION[19][15].first = 's'; ACTION[19][15].second = 33;
    ACTION[20][4].first = 'r'; ACTION[20][4].second = 21; ACTION[20][10].first = 'r'; ACTION[20][10].second = 21;
    ACTION[20][11].first = 'r'; ACTION[20][11].second = 21; ACTION[20][12].first = 'r'; ACTION[20][12].second = 21;
    ACTION[20][13].first = 'r'; ACTION[20][13].second = 21; ACTION[20][14].first = 'r'; ACTION[20][14].second = 21;
    ACTION[20][15].first = 'r'; ACTION[20][15].second = 21; ACTION[20][16].first = 's'; ACTION[20][16].second = 35;
    ACTION[20][17].first = 's'; ACTION[20][17].second = 36;
    ACTION[21][4].first = 'r'; ACTION[21][4].second = 25;
    for (int i = 10; i <= 17; i++) {
        ACTION[21][i].first = 'r'; ACTION[21][i].second = 25;
    }
    ACTION[21][18].first = 's'; ACTION[21][18].second = 38; ACTION[21][19].first = 's'; ACTION[21][19].second = 39;
    ACTION[22][4].first = 'r'; ACTION[22][4].second = 26;
    for (int i = 10; i <= 19; i++) {
        ACTION[22][i].first = 'r'; ACTION[22][i].second = 26;
    }
    ACTION[23][4].first = 'r'; ACTION[23][4].second = 27;
    for (int i = 10; i <= 19; i++) {
        ACTION[23][i].first = 'r'; ACTION[23][i].second = 27;
    }
    ACTION[24][3].first = 's'; ACTION[24][3].second = 24; ACTION[24][8].first = 's'; ACTION[24][8].second = 22;
    ACTION[24][20].first = 's'; ACTION[24][20].second = 23;
    ACTION[25][4].first = 's'; ACTION[25][4].second = 41;
    ACTION[26][10].first = 's'; ACTION[26][10].second = 42;
    ACTION[27][5].first = 's'; ACTION[27][5].second = 43;
    ACTION[28][3].first = 's'; ACTION[28][3].second = 24; ACTION[28][8].first = 's'; ACTION[28][8].second = 22;
    ACTION[28][20].first = 's'; ACTION[28][20].second = 23;
    ACTION[29][3].first = 'r'; ACTION[29][3].second = 13; ACTION[29][8].first = 'r'; ACTION[29][8].second = 13;
    ACTION[29][20].first = 'r'; ACTION[29][20].second = 13;
    ACTION[30][3].first = 'r'; ACTION[30][3].second = 14; ACTION[30][8].first = 'r'; ACTION[30][8].second = 14;
    ACTION[30][20].first = 'r'; ACTION[30][20].second = 14;
    ACTION[31][3].first = 'r'; ACTION[31][3].second = 15; ACTION[31][8].first = 'r'; ACTION[31][8].second = 15;
    ACTION[31][20].first = 'r'; ACTION[31][20].second = 15;
    ACTION[32][3].first = 'r'; ACTION[32][3].second = 16; ACTION[32][8].first = 'r'; ACTION[32][8].second = 16;
    ACTION[32][20].first = 'r'; ACTION[32][20].second = 16;
    ACTION[33][3].first = 'r'; ACTION[33][3].second = 17; ACTION[33][8].first = 'r'; ACTION[33][8].second = 17;
    ACTION[33][20].first = 'r'; ACTION[33][20].second = 17;
    ACTION[34][4].first = 'r'; ACTION[34][4].second = 18;
    for (int i = 10; i <= 15; i++) {
        ACTION[34][i].first = 'r'; ACTION[34][i].second = 18;
    }
    for (int i = 35; i <= 36; i++) {
        ACTION[i][3].first = 's'; ACTION[i][3].second = 24;
        ACTION[i][8].first = 's'; ACTION[i][8].second = 22;
        ACTION[i][20].first = 's'; ACTION[i][20].second = 23;
    }
    ACTION[37][4].first = 'r'; ACTION[37][4].second = 22;
    for (int i = 10; i <= 17; i++) {
        ACTION[37][i].first = 'r'; ACTION[37][i].second = 22;
    }
    for (int i = 38; i <= 39; i++) {
        ACTION[i][3].first = 's'; ACTION[i][3].second = 24;
        ACTION[i][8].first = 's'; ACTION[i][8].second = 22;
        ACTION[i][20].first = 's'; ACTION[i][20].second = 23;
    }
    ACTION[40][4].first = 's'; ACTION[40][4].second = 49;
    ACTION[41][0].first = 's'; ACTION[41][0].second = 3; ACTION[41][2].first = 's'; ACTION[41][2].second = 10;
    ACTION[41][7].first = 's'; ACTION[41][7].second = 11; ACTION[41][8].first = 's'; ACTION[41][8].second = 12;
    for (int i = 0; i <= 2; i++) {
        ACTION[42][i].first = 'r'; ACTION[42][i].second = 11;
    }
    for (int i = 6; i <= 8; i++) {
        ACTION[42][i].first = 'r'; ACTION[42][i].second = 11;
    }
    ACTION[43][0].first = 's'; ACTION[43][0].second = 3; ACTION[43][2].first = 's'; ACTION[43][2].second = 10;
    ACTION[43][7].first = 's'; ACTION[43][7].second = 11; ACTION[43][8].first = 's'; ACTION[43][8].second = 12;
    ACTION[44][4].first = 'r'; ACTION[44][4].second = 12;
    for (int i = 45; i <= 46; i++) {
        ACTION[i][4].first = 'r'; ACTION[i][4].second = 21;
        for (int j = 10; j <= 15; j++) {
            ACTION[i][j].first = 'r'; ACTION[i][j].second = 21;
        }
        ACTION[i][16].first = 's'; ACTION[i][16].second = 35; ACTION[i][17].first = 's'; ACTION[i][17].second = 36;
    }
    for (int i = 47; i <= 48; i++) {
        ACTION[i][4].first = 'r'; ACTION[i][4].second = 25;
        for (int j = 10; j <= 17; j++) {
            ACTION[i][j].first = 'r'; ACTION[i][j].second = 25;
        }
        ACTION[i][18].first = 's'; ACTION[i][18].second = 38;
        ACTION[i][19].first = 's'; ACTION[i][19].second = 39;
    }
    ACTION[49][4].first = 'r'; ACTION[49][4].second = 28;
    for (int i = 10; i <= 19; i++) {
        ACTION[49][i].first = 'r'; ACTION[49][i].second = 28;
    }
    for (int i = 0; i <= 2; i++) {
        ACTION[50][i].first = 'r'; ACTION[50][i].second = 10;
    }
    for (int i = 6; i <= 8; i++) {
        ACTION[50][i].first = 'r'; ACTION[50][i].second = 10;
    }
    ACTION[51][6].first = 's'; ACTION[51][6].second = 56;
    ACTION[52][4].first = 'r'; ACTION[52][4].second = 19;
    for (int i = 10; i <= 15; i++) {
        ACTION[52][i].first = 'r'; ACTION[52][i].second = 19;
    }
    ACTION[53][4].first = 'r'; ACTION[53][4].second = 20;
    for (int i = 10; i <= 15; i++) {
        ACTION[53][i].first = 'r'; ACTION[53][i].second = 20;
    }
    ACTION[54][4].first = 'r'; ACTION[54][4].second = 23;
    for (int i = 10; i <= 17; i++) {
        ACTION[54][i].first = 'r'; ACTION[54][i].second = 23;
    }
    ACTION[55][4].first = 'r'; ACTION[55][4].second = 24;
    for (int i = 10; i <= 17; i++) {
        ACTION[55][i].first = 'r'; ACTION[55][i].second = 24;
    }
    ACTION[56][0].first = 's'; ACTION[56][0].second = 3; ACTION[56][2].first = 's'; ACTION[56][2].second = 10;
    ACTION[56][7].first = 's'; ACTION[56][7].second = 11; ACTION[56][8].first = 's'; ACTION[56][8].second = 12;
    for (int i = 0; i <= 2; i++) {
        ACTION[57][i].first = 'r'; ACTION[57][i].second = 9;
    }
    for (int i = 6; i <= 8; i++) {
        ACTION[57][i].first = 'r'; ACTION[57][i].second = 9;
    }
}

void moveNext(string& prog) {
    cur++;
    while (cur < prog.size() && (prog[cur] == ' ' || prog[cur] == '\t' || prog[cur] == '\n')) {
        if (prog[cur] == '\n') {
            lineNumber++;
        }
        cur++;
    }
    if ((prog[cur] == '=' || prog[cur] == '<' || prog[cur] == '>') && prog[cur + 1] == '=') {
        buffer = string(1, prog[cur]);
        buffer += prog[cur + 1];
        cur++;
    } else if (prog[cur] == 'i') {
        buffer = string(1, prog[cur]);
        buffer += prog[cur + 1];
        cur++;
    } else if (prog[cur] == 't') {
        buffer = prog.substr(cur, 4);
        cur += 3;
    } else if (prog[cur] == 'e') {
        buffer = prog.substr(cur, 4);
        cur += 3;
    } else if (prog[cur] == 'w') {
        buffer = prog.substr(cur, 5);
        cur += 4;
    } else if (prog[cur] == 'I') {
        buffer = prog.substr(cur, 2);
        cur += 1;
    } else if (prog[cur] == 'N') {
        buffer = prog.substr(cur, 3);
        cur += 2;
    } else {
        buffer = string(1, prog[cur]);
    }
}

int getActionCol(string& token) {
    return terminalMap[token];
}

int getGotoCol(string& token) {
    return nonTerminalMap[token];
}

bool isNonTerminal(string& token) {
    return nonTerminalMap.find(token) != nonTerminalMap.end();
}

void error(int state){
    string lost = ";";
    string info = "语法错误，第" + to_string(lineNumber) + "行，缺少" + "\"" + lost + "\"";
//    printf("error %d\n", state);
    if (errorInfo.empty() || info != errorInfo.back()) {
        errorInfo.push_back(info);
    }
    int col = getActionCol(lost);
    if (ACTION[state][col].first == 's') {
        stack.push_back(ACTION[state][col].second);
    } else if (ACTION[state][col].first == 'r') {
        vector<string> produce = products[ACTION[state][col].second];
        int size = produce.size() - 1;
        for (int i = 0; i < size; i++) {
            stack.pop_back();
        }
        int t = stack.back();
        int gotoCol = getGotoCol(produce[0]);
        stack.push_back(GOTO[t][gotoCol]);
        reduce.push_back(produce);
    }
}

void LR(string& prog) {
    prog += '$';
    moveNext(prog);
    stack.push_back(0);
    int state;
    while (1) {
        state = stack.back();
//        for (int i = 0; i < stack.size(); i++) {
//            printf("%d ", stack[i]);
//        }
//        printf("%s\n", buffer.c_str());
        int col = getActionCol(buffer);
        if (ACTION[state][col].first == 's') {
            stack.push_back(ACTION[state][col].second);
            moveNext(prog);
        } else if (ACTION[state][col].first == 'r') {
            vector<string> produce = products[ACTION[state][col].second];
            int size = produce.size() - 1;
            for (int i = 0; i < size; i++) {
                stack.pop_back();
            }
            int t = stack.back();
            int gotoCol = getGotoCol(produce[0]);
            stack.push_back(GOTO[t][gotoCol]);
            reduce.push_back(produce);
        } else if (ACTION[state][col].first == 'a') {
            break;
        } else {
            error(state);
        }
    }
}

void printResult() {
    vector<vector<string>> ans;
    ans.push_back(vector<string>({"program"}));
    for (int i = 0; i < errorInfo.size(); i++) {
        printf("%s\n", errorInfo[i].c_str());
    }
    for (int i = reduce.size() - 1; i >= 0; i--) {
        vector<string> result;
        vector<string> pre = ans.back();
        int pos = 0;
        for (int j = pre.size() - 1; j >= 0; j--) {
            if (isNonTerminal(pre[j])) {
                pos = j;
                break;
            }
        }
        for (int j = 0; j < pre.size(); j++) {
            if (j != pos) {
                result.push_back(pre[j]);
            } else {
                for (int k = 1; k < reduce[i].size(); k++) {
                    result.push_back(reduce[i][k]);
                }
            }
        }
        ans.push_back(result);
    }
    for (int i = 0; i < ans.size(); i++) {
        if (!ans[i].empty()) {
            printf("%s", ans[i][0].c_str());
            for (int j = 1; j < ans[i].size(); j++) {
                printf(" %s", ans[i][j].c_str());
            }
        }
        if (i != ans.size() - 1) {
            printf(" =>\n");
        }
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

    LR(prog);

    printResult();
    
    /********* End *********/
    
}