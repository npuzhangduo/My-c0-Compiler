
#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <error.cpp>
using namespace std;

// 关键词编码
enum e_TokenCode {
    TK_PLUS,        // + 加号
    TK_MINUS,       // - 减号
    TK_MUL,         // * 乘号
    TK_DIV,         //  / 除号
    TK_LT,          // < 小于号
    TK_LEQ,         // <= 小于等于
    TK_GT,          // > 大于
    TK_GEQ,         // >= 大于等于
    TK_NEQ,         // != 不等于
    TK_EQ,          // == 等TK_于
    TK_COMMA,       // , 逗号
    TK_BEGIN,       // { 左大括号
    TK_END,         // } 右大括号
    TK_OPENBR,      // [ 左中括号
    TK_CLOSEBR,     // ] 右中括号
    TK_OPENPA,      // ( 左小括号
    TK_CLOSEPA,     // ) 右下括号
    TK_SQUOTE,      // ' 单引号
    TK_DQUOTE,      // "  双引号
    TK_ASSIGN,      // = 赋值
    TK_COLON,       // : 冒号
    TK_SEMICOLON,   // ; 分号
    TK_EOF,         // EOF 文件结束

    TK_STRING,      // 字符串常量
    TK_NUMBER,      // 常数
    TK_CHAR,        // 字符常量

    // 关键字
    KW_INT,         // int
    KW_CHAR,        // char
    KW_CONST,       // const
    KW_MAIN,        // main
    KW_IF,          // if
    KW_ELSE,        // else
    KW_WHILE,       // while
    KW_DO,          // do
    KW_VOID,        // void
    KW_SCANF,       // scanf
    KW_PRINT,       // print
    KW_RETURN,      // return
    KW_SWITH,       // switch
    KW_CASE,        // case
    KW_FOR,         // for

    // 标识符
    IDSYM
};

int num_of_kw = KW_FOR - KW_INT + 1;

string word[] = {"int","char","const","main","if","else",
                    "while","do","void","scanf","printf","return","switch","case","for"} ;
string wSymbol[] = {"KW_INT","KW_CHAR","KW_CONST","KW_MAIN","KW_IF","KW_ELSE",
                    "KW_WHILE","KW_DO","KW_VOID","KW_SCANF","KW_PRINT","KW_RETURN","KW_SWITCH","KW_CASE","KW_FOR"} ;


FILE * fin;
FILE * fin_t;
int line_num = 0;
int symId;
string sym;
int num;
char ch;
string token;

string source;
ofstream result;
ofstream _gramma;

void getch() {
    ch = getc(fin);
}

// 忽略空格、tab和回车
void skip_white_space () {
    //空格 tab 回车
    while (ch == ' ' || ch == '\t' || ch == '\r') { 
        if (ch == '\r') {
            getch();
            if (ch != '\n')
                return;
            line_num++;
        }
        // printf("%c",ch);
        getch();
    }
}

// 解析注释
void parse_comment () {
    getch();
    do {
        do {
            if (ch == '\n' || ch == '*' || ch == EOF) 
                break;
            else 
                getch();
        } while(1);

        if (ch == '\n') {
            line_num++;
            getch();
        }
        else if (ch == '*') {
            getch();
            if (ch == '/') {
                getch();
                return;
            }
        }
        else {
            printf("一直到结尾未看到配对的注释结束符");
            return;
        }
    } while(1);
}

// 预处理
void preprocess () {
    while (1) {
        if (ch == ' ' || ch == '\t' || ch == '\r')
            skip_white_space();
        else if (ch == '/') {
            getch();
            if (ch == '*') {
                parse_comment();
            }
            else {
                ungetc(ch,fin);
                ch = '/';
                break;
            }
        }
        else
            break;
    }
}

int is_letter (char c) {
    return (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') || c == '_'; 
}

// 判断 c 是否为数字
int is_digit (char c) {
    return c >= '0' && c <= '9';
}

void parse_identifier() {
    token.clear();
    token.push_back(ch);
    getch();
    while (is_letter(ch) || is_digit(ch)) {
        token += ch;
        getch();
    }
    
}

void parse_string(char sep) {
    getch();
    char c;
    source.clear();
    source.push_back(sep);
    while (true) {
        if (ch == sep)
            break;
        else if (ch == '\\') {
            source.push_back(ch);
            getch();
            if (ch == '0')
                c = '\0';
            else if (ch == 'a')
                c = '\a';
            else if (ch == 'b')
                c = '\b';
            else if (ch == 't')
                c = '\t';
            else if (ch == 'n')
                c = '\n';
            else if (ch == 'v')
                c = '\v';
            else if (ch == 'f')
                c = '\f';
            else if (ch == 'r')
                c = '\r';
            else if (ch == '\"')
                c = '\"';
            else if (ch == '\'')
                c = '\'';
            else if (ch == '\\')
                c = '\\';
            else {
                error(ESCAPE_CHAR_ERROR,line_num);
            }
            token.push_back(c);
            source.push_back(ch);
            getch();
        }
        else {
            token.push_back(ch);
            source.push_back(ch);
            getch();
        }
    }
    source.push_back(sep);
    getch();
}

void getsym() {
    token.clear();
    preprocess();
    if (is_letter(ch)) {
        parse_identifier();
        int is_kw = 0;
        for (int i = 0;i < num_of_kw;i++) {
            if (token == word[i]) {
                symId = KW_INT + i;
                sym = wSymbol[i];
                is_kw = 1;
                break;
            }
        }
        if (!is_kw) {
            sym = "IDSYM";
            symId = IDSYM;
        }
    }
    else if (is_digit(ch)) {
        sym = "TK_NUMBER";
        symId = TK_NUMBER;
        num = 0;
        int flag = 0;
        if (ch == '0') {
            flag = 1;
        }
        
        while (is_digit(ch)) {
            token.push_back(ch);
            num = num * 10 + (int)(ch - '0');
            getch();
        }
        if (flag == 1 && num != 0) {
            error(ZEROSTART_ERROR,line_num);
        }
    }
    else if (ch == '=') {
        token.push_back(ch);
        symId = TK_ASSIGN;
        sym = "TK_ASSIGN";
        getch();
        if (ch == '=') {
            token.push_back(ch);
            symId = TK_EQ;
            sym = "TK_EQ";
            getch();
        }
    }
    else if (ch == '>') {
        token.push_back(ch);
        symId = TK_GT;
        sym = "TK_GT";
        getch();
        if (ch == '=') {
            token.push_back(ch);
            symId = TK_GEQ;
            sym = "TK_GEQ";
            getch();
        }
    }
    else if (ch == '<') {
        token.push_back(ch);
        symId = TK_LT;
        sym = "TK_LT";
        getch();
        if (ch == '=') {
            token.push_back(ch);
            symId = TK_LEQ;
            sym = "TK_LEQ";
            getch();
        }
    }
    else if (ch == '!') {
        token.push_back(ch);
        getch();
        if (ch == '=') {
            token.push_back(ch);
            symId = TK_NEQ;
            sym = "TK_NEQ";
            getch();
        }
        else {
            error(NOT_ERROR,line_num);
        }
    }
    else if (ch == '\'') {
        parse_string('\'');
        symId = TK_CHAR;
        sym = "TK_CHAR";
       // getch();
    }
    else if (ch == '\"') {
        parse_string('\"');
        symId = TK_STRING;
        sym = "TK_STRING";
    }
    else if (ch == ',') {
        symId = TK_COMMA;
        sym = "TK_COMMA";
        token.push_back(ch);
        getch();
    }
    else if (ch == ':') {
        symId = TK_COLON;
        sym = "TK_COLON";
        token.push_back(ch);
        getch();
    }
    else if (ch == ';') {
        symId = TK_SEMICOLON;
        sym = "TK_SEMICOLON";
        token.push_back(ch);
        getch();
    }
    else if (ch == '{') {
        symId = TK_BEGIN;
        sym = "TK_BEGIN";
        token.push_back(ch);
        getch();
    }
    else if (ch == '}') {
        symId = TK_END;
        sym = "TK_END";
        token.push_back(ch);
        getch();
    }
    else if (ch == '[') {
        symId = TK_OPENBR;
        sym = "TK_OPENBR";
        token.push_back(ch);
        getch();
    }
    else if (ch == ']') {
        symId = TK_CLOSEBR;
        sym = "TK_CLOSEBR";
        token.push_back(ch);
        getch();
    }
    else if (ch == '(') {
        symId = TK_OPENPA;
        sym = "TK_OPENPA";
        token.push_back(ch);
        getch();
    }
    else if (ch == ')') {
        symId = TK_CLOSEPA;
        sym = "TK_CLOSEPA";
        token.push_back(ch);
        getch();
    }
    else if (ch == '+') {
        symId = TK_PLUS;
        sym = "TK_PLUS";
        token.push_back(ch);
        getch();
    }
    else if (ch == '-') {
        symId = TK_MINUS;
        sym = "TK_MINUS";
        token.push_back(ch);
        getch();
    }
    else if (ch == '*') {
        symId = TK_MUL;
        sym = "TK_MUL";
        token.push_back(ch);
        getch();
    }
    else if (ch == '/') {
        symId = TK_DIV;
        sym = "TK_DIV";
        token.push_back(ch);
        getch();
    }
    else if (ch == EOF) {
        symId = TK_EOF;
        sym = "TK_EOF";
    }
    // if (!sym.compare("TK_NUMBER")) {
    //     cout << sym << " : " << symId << " " << token <<" "<< num <<endl;
    // }
    // else {
    //     cout << sym << " : " << symId << " " << token <<endl;
    // }
}