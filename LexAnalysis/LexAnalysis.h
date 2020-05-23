// C语言词法分析器
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
using namespace std;

vector<pair<string , int> > table;
const int COMMENT = 79;
const int CONSTANT = 80;
const int IDENTIFIER = 81;
string keyword[32] = {"auto", "break", "case", "char", "const", "continue", "default", "do",
                      "double", "else", "enum", "extern", "float", "for", "goto", "if", "int", "long",
                      "register", "return", "short", "signed", "sizeof", "static", "struct",  "switch",
                      "typedef",  "union",  "unsigned", "void", "volatile",  "while" };
map<string, int> keywords;
map<string, int> delimiters;
map<string, int> ops;


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
    // keywords 初始化
    for (int i = 0; i < 32; i++) {
        keywords[keyword[i]] = i + 1;
    }
    // ops 初始化
    ops["-"] = 33; ops["--"] = 34; ops["-="] = 35; ops["->"] = 36; ops["!"] = 37; ops["!="] = 38;
    ops["%"] = 39; ops["%="] = 40; ops["&"] = 41; ops["&&"] = 42; ops["&="] = 43; ops["*"] = 46;
    ops["*="] = 47; ops["."] = 49; ops["/"] = 50; ops["/="] = 51; ops[":"] = 52; ops["?"] = 54;
    ops["^"] = 57; ops["^="] = 58; ops["|"] = 60; ops["||"] = 61; ops["|="] = 62; ops["~"] = 64; ops["+"] = 65;
    ops["++"] = 66; ops["+="] = 67; ops["<"] = 68; ops["<<"] = 69; ops["<<="] = 70; ops["<="] = 71;
    ops["="] = 72; ops["=="] = 73; ops[">"] = 74; ops[">="] = 75; ops[">>"] = 76; ops[">>="] = 77;

    // delimiter 初始化
    delimiters["("] = 44; delimiters[")"] = 45; delimiters[","] = 48; delimiters[";"] = 53;
    delimiters["["] = 55; delimiters["]"] = 56; delimiters["{"] = 59; delimiters["}"] = 63;
    delimiters["\""] = 78;
}

bool isLetter(char ch) {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

bool isDigit(char ch) {
    return (ch >= '0' && ch <= '9');
}

bool isKeyword(string& s) {
    return (keywords[s] != 0);
}

bool isIdentifier(char ch) {
    return isLetter(ch) || isDigit(ch) || ch == '_';
}

void transfer(string& prog) {
    int state = 0;
    int p = 0;
    string buffer;
    while (p < prog.size()) {
        switch (state) {
            case 0:
                if (isLetter(prog[p])) state = 1;
                else if (isDigit(prog[p])) state = 4;
                else if (prog[p] == '-') state = 6;
                else if (prog[p] == '!') state = 11;
                else if (prog[p] == '%') state = 14;
                else if (prog[p] == '&') state = 17;
                else if (prog[p] == '*') state = 21;
                else if (prog[p] == '.') state = 24;
                else if (prog[p] == '/') state = 25;
                else if (prog[p] == ':') state = 33;
                else if (prog[p] == '?') state = 34;
                else if (prog[p] == '^') state = 35;
                else if (prog[p] == '|') state = 38;
                else if (prog[p] == '~') state = 42;
                else if (prog[p] == '+') state = 43;
                else if (prog[p] == '<') state = 47;
                else if (prog[p] == '=') state = 53;
                else if (prog[p] == '>') state = 56;
                else if (prog[p] == '(' || prog[p] == ')' || prog[p] == ','
                || prog[p] == ';' || prog[p] == '[' || prog[p] == ']'
                || prog[p] == '{' || prog[p] == '}') state = 62;
                else if (prog[p] == '"') state = 63;
                else if (prog[p] == ' ' || prog[p] == '\t' || prog[p] == '\n') state = 65;
                buffer += prog[p];
                break;
            case 1: // Identifier
                if (isIdentifier(prog[p])) {
                    state = 1;
                    buffer += prog[p];
                } else if (isKeyword(buffer)) {
                    p--;
                    state = 2;
                } else {
                    p--;
                    state = 3;
                }
                break;
            case 2: // keyword
                table.push_back(make_pair(buffer, keywords[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 3: // Identifier accept
                table.push_back(make_pair(buffer, IDENTIFIER));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 4: // digit
                if (isDigit(prog[p])) {
                    state = 4;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 5;
                }
                break;
            case 5: // digit accept
                table.push_back(make_pair(buffer, CONSTANT));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 6: // -
                if (prog[p] == '-') {
                    state = 7;
                    buffer += prog[p];
                } else if (prog[p] == '=') {
                    state = 8;
                    buffer += prog[p];
                } else if (prog[p] == '>') {
                    state = 9;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 10;
                }
                break;
            case 7: // --
            case 8: // -=
            case 9: // ->
            case 10: // -
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 11: // !
                if (prog[p] == '=') {
                    state = 12;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 13;
                }
                break;
            case 12: // !=
            case 13: // !
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 14: // %
                if (prog[p] == '=') {
                    state = 15;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 16;
                }
                break;
            case 15: // %=
            case 16: // %
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 17: // &
                if (prog[p] == '&') {
                    state = 18;
                    buffer += prog[p];
                } else if (prog[p] == '=') {
                    state = 19;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 20;
                }
                break;
            case 18: // &&
            case 19: // &=
            case 20: // &
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 21: // *
                if (prog[p] == '=') {
                    state = 22;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 23;
                }
                break;
            case 22: // *=
            case 23: // *
            case 24: // .
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 25: // /
                if (prog[p] == '=') {
                    state = 26;
                    buffer += prog[p];
                } else if (prog[p] == '*') {
                    state = 27;
                    buffer += prog[p];
                } else if (prog[p] == '/') {
                    state = 30;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 32;
                }
                break;
            case 26: // /=
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 27: // /*
                if (prog[p] == '*') {
                    state = 28;
                    buffer += prog[p];
                } else {
                    state = 27;
                    buffer += prog[p];
                }
                break;
            case 28: // /*...*
                if (prog[p] == '/') {
                    state = 29;
                    buffer += prog[p];
                } else {
                    state = 27;
                    buffer += prog[p];
                }
                break;
            case 29: // /*...*/
                table.push_back(make_pair(buffer, COMMENT));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 30: // //
                if (prog[p] == '\n') {
                    state = 31;
                } else {
                    state = 30;
                    buffer += prog[p];
                }
                break;
            case 31: // //...\n
                table.push_back(make_pair(buffer, COMMENT));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 32: // /
            case 33: // :
            case 34: // ?
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 35: // ^
                if (prog[p] == '=') {
                    state = 36;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 37;
                }
                break;
            case 36: // ^=
            case 37: // ^
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 38: // |
                if (prog[p] == '|') {
                    state = 39;
                    buffer += prog[p];
                } else if (prog[p] == '=') {
                    state = 40;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 41;
                }
                break;
            case 39: // ||
            case 40: // |=
            case 41: // |
            case 42: // ~
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 43: // +
                if (prog[p] == '+') {
                    state = 44;
                    buffer += prog[p];
                } else if (prog[p] == '=') {
                    state = 45;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 46;
                }
                break;
            case 44: // ++
            case 45: // +=
            case 46: // +
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 47: // <
                if (prog[p] == '<') {
                    state = 48;
                    buffer += prog[p];
                } else if (prog[p] == '=') {
                    state = 51;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 52;
                }
                break;
            case 48: // <<
                if (prog[p] == '=') {
                    state = 49;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 50;
                }
                break;
            case 49: // <<=
            case 50: // <<
            case 51: // <=
            case 52: // <
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 53: // =
                if (prog[p] == '=') {
                    state = 54;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 55;
                }
                break;
            case 54: // ==
            case 55: // =
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 56: // >
                if (prog[p] == '>') {
                    state = 57;
                    buffer += prog[p];
                } else if (prog[p] == '=') {
                    state = 60;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 61;
                }
                break;
            case 57: // >>
                if (prog[p] == '=') {
                    state = 58;
                    buffer += prog[p];
                } else {
                    p--;
                    state = 59;
                }
                break;
            case 58: // >>=
            case 59: // >>
            case 60: // >=
            case 61: // >
                table.push_back(make_pair(buffer, ops[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 62: //
                table.push_back(make_pair(buffer, delimiters[buffer]));
                buffer.clear();
                p--;
                state = 0;
                break;
            case 63:
                if (prog[p] == '"') {
                    state = 64;
                    buffer += prog[p];
                } else {
                    state = 63;
                    buffer += prog[p];
                }
                break;
            case 64:
                buffer = buffer.substr(1, buffer.size() - 2);
                table.push_back(make_pair("\"", delimiters["\""]));
                table.push_back(make_pair(buffer, IDENTIFIER));
                table.push_back(make_pair("\"", delimiters["\""]));
                p--;
                buffer.clear();
                state = 0;
                break;
            default:
                buffer.clear();
                p--;
                state = 0;
                break;
        }
        p++;
    }
}

void print() {
    int i;
    for (i = 0; i < table.size(); i++) {
        printf("%d: <%s,%d>\n", i + 1, table[i].first.c_str(), table[i].second);
    }
    printf("%d: <%s,%d>", i + 1, "}", delimiters["}"]);
}


void Analysis()
{
    init();
    string prog;
    read_prog(prog);
    /* 骚年们 请开始你们的表演 */
    /********* Begin *********/
//    printf("%s", prog.c_str());
    init();
    transfer(prog);
    print();
    /********* End *********/

}