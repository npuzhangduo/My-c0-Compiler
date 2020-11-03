#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string.h>
#include <lex.cpp>
#include <midcode.cpp>

using namespace std;

const int Max = 512;
const int Max_tab = 512;
const int Max_pre = 512;

enum Type_ {
    CONST,
    VAR,
    FUNC,
    PARA
};

// 符号表相关
typedef struct {
    char name[Max];  // name of identifier
    int type; // 0-const 1-var 2-function 3-para
    int value; // 常量的值，特别地，如果标识符是一个函数名，则用1表示函数类型int，0表示void
    int address; // 标识符存储地址或地址位移
    int para; // 表示参数个数或者数组大小
} symbol;

typedef struct  {
    symbol element[Max_tab];
    int index; // 符号表栈顶指针, number of symbol table
    int toltalPre; // 当前符号表拥有地分程序总数
    int indexOfPre[Max_pre]; // 分程序索引 find index in table 
} symbolTab;

symbolTab symTable;
char idName[Max];
int value;
int para;
int address;

int is_array;

int pre_symId;
string preToken;
int preIndex;
char preCh;

int isFor = 0;

int isVoid = 0;

int isArr = 0;

int hasRet = 0;

// 四元式相关
// outputfile
char temp[128];
char nowitem[Max];
int factorType;

char procname[Max];
int tempSym;
int isconst;
int isMain = 0;

// 方法声明
// grammer analyse
void states();
void item();
void factor();
void valueOfpara();
void condition();

void loopSta();
void writeSta();
void switchSta();
void readSta();
void returnSta();
void conditionSta();


void pushSymTab(char * name,int type,int value,int address,int para);

void insertSymTab(char * name,int type,int value,int adress,int para);

int searchSymTab(char * name);

void insertPara(int para);

void delSymTab();

void insertSymTab(char * name,int type,int value,int address,int para) {
    if (symTable.index > Max_tab) {
        error(OUTOFTABINDEXX_ERROR,line_num);
        return;
    }
    pushSymTab(name,type,value,address,para);
}

void insertPara(int para) {
    int i;
    i = symTable.indexOfPre[symTable.toltalPre - 1];
    symTable.element[i].para = para;
}

int searchSymTab(char * name,int flag) { // use flah to seperate id or func
    isArr = 0;
    if (flag == 1) { // func
        int i;
        for (i = 1;i < symTable.toltalPre;i++) {
            if (strcmp(symTable.element[symTable.indexOfPre[i]].name,name) == 0) {
                break;
            }
        }
        if (i >= symTable.toltalPre) { // can not find
            return 0;
        }
        if (symTable.element[symTable.indexOfPre[i]].para != para) {
            error(PARANUM_ERROR,line_num);
            return -1;
        }
        if (symTable.element[symTable.indexOfPre[i]].value == 0) {
            isVoid = 1;
        }
        return 1;
    }
    else {  // id
        int i;
        // 首先在所在函数对应的符号表区域寻找，即局部变量
        for (i = symTable.indexOfPre[symTable.toltalPre-1];i < symTable.index;i++) {
            if (strcmp(symTable.element[i].name,name) == 0) {
                break;
            }
        }
        // 全局变量
        if (i == symTable.index) {
            int i = 0;
            int n = symTable.indexOfPre[1];
            for (;i < n;i++) {
                if (strcmp(symTable.element[i].name,name) == 0) {
                    break;
                }
            }
            if (i == n) {
                cout << line_num <<" : " << name << ":" << "undefined identifier!" << endl;
                return 0;
            }
            if (symTable.element[i].type == 1) { // var
                factorType = symTable.element[i].value;
                if (symTable.element[i].para != -1) {
                    isArr = 1;
                }
                return symTable.element[i].address;
            }
            else if (symTable.element[i].type == 0) { // const
                isconst = 1;
                return symTable.element[i].value;
            }
            else if (symTable.element[i].type == 3) { // paramenter
                return -1;
            }
        }
        else { // 局部变量
            if (symTable.element[i].type == 1) { // var
                factorType = symTable.element[i].value;
                if (symTable.element[i].para != -1) {
                    isArr = 1;
                }
                return symTable.element[i].address;
            }
            else if (symTable.element[i].type == 0) { // const
                isconst = 1;
                return symTable.element[i].value;
            }
            else if (symTable.element[i].type == 3) {
                return -1;
            }
        }
        return 1;
    }
}

/*
查找符号表获得标识符为name的数组的长度
*/
int arrLength(char * name) { // once a array was used,search 
    int i;
    i = symTable.indexOfPre[symTable.toltalPre-1];
    for (;i < symTable.index;i++) {  // 如果该数组是局部变量
        if (strcmp(symTable.element[i].name,name) == 0) {
            return symTable.element[i].para;
        }
    }
    if (i == symTable.index) { // 不是局部变量则是全局变量
        i= 0;
        for (;i < symTable.indexOfPre[1];i++) {
            if (strcmp(symTable.element[i].name,name) == 0) {
                return symTable.element[i].para;
            }
        }
    }
}

void pushSymTab(char * name,int type,int value,int address,int para) {
    if (type == 2) { // func
        int i;
        for (int i = 1;i < symTable.toltalPre;i++) {
            if (strcmp(symTable.element[symTable.indexOfPre[i]].name,name) == 0) {
                error(FUNCNAMECOMPLICT_ERROR,line_num);
                return;
            }
        }
        if (i >= symTable.toltalPre) {
            symTable.indexOfPre[symTable.toltalPre++] = symTable.index;
        }
    }
    else {
        int i = symTable.indexOfPre[symTable.toltalPre-1]; // 查找当前函数的符号表
        for (;i < symTable.index;i++) {
            if (strcmp(symTable.element[i].name,name) == 0) {
                error(VARNAMECOMPLICT_ERROR,line_num);
                return;
            }
        }
    }
    strcpy(symTable.element[symTable.index].name,name);
    symTable.element[symTable.index].type = type;
    symTable.element[symTable.index].value = value;
    symTable.element[symTable.index].address = address;
    symTable.element[symTable.index].para = para;
    symTable.index++;
}


void delSymTab() {

}

bool IsArray() {
    if (isArr == 1) {
        isArr = 0;
        return true;
    }
    else {
        return false;
    }
}

bool IsConst() {
    if (isconst == 1) {
        isconst = 0;
        return true;
    }
    else {
        return false;
    }
}

bool IsVoid() {
    if (isVoid == 1) {
        isVoid = 0;
        return true;
    }
    else {
        return false;
    }
}
void skip(char c) {
    if (token.c_str()[0] != c) {
        cout << "Error: Line:" << line_num << " expect :" << c <<endl; 
        // exit(0);
        return;
    }
    getsym();
}

void expect(string item) {
    cout << "Line: " << line_num << item << "." <<endl;
    // exit(0);
}

void readNext() {
    do {
        getsym();
    } while (symId != TK_SEMICOLON);
}
/*
<声明头部> ::= int<标识符>|char<标识符>
用于有返回值的函数
*/
void declHead() {
    
    if (symId == KW_INT || symId == KW_CHAR) {
        getsym();
        if (symId != IDSYM) {
            error(DELHEAD_IDMIS_ERROR,line_num);
            return;
        }
        strcpy(idName,token.c_str());
        value = 1;
        address = 0; // 暂时设为0
        para = 0;
        insertSymTab(idName,2,value,address,para);
        strcpy(procname,idName);
    
        getsym();

    }
}

/*
常量定义子程序
<常量定义> ::= int<标识符> = <整数> {,<标识符> = <整数>} 
| char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}

暂时只考虑 <标识符> = <整数（字符）>
*/
void constDef(int type) {
    getsym();
    if (symId == IDSYM) {
        strcpy(idName,token.c_str());
        getsym();
        skip('=');
        // <整数> ::= [+|-]<无符号整数> | 0
        
        if (symId == TK_PLUS || symId == TK_MINUS) {
            int tempSymId = symId; // for the record of +/-;
            getsym();
            if (type == KW_INT && symId == TK_NUMBER) {
                if (tempSymId == TK_PLUS) {
                    value = num;
                }
                else {
                    value = -num;
                }
                // 插入符号表
                address++;
                para = -1;
                insertSymTab(idName,0,value,address,para);

                // 生成四元式
                sprintf(temp,"%d",value);
                genMidCode(consts,ints,temp,idName);
            }
            else {
                expect("number");
            }
        }
        else if (type == KW_CHAR && symId == TK_CHAR) {
            value = token[0];
            
            // 插入符号表
            address++;
            para = -1;
            insertSymTab(idName,0,value,address,para);
            // 生成四元式
            sprintf(temp,"%d",value);
            genMidCode(consts,chars,temp,idName);
        }
        else if (type == KW_INT && symId == TK_NUMBER) {
            value = num;
            // 插入符号表
            address++;
            para = -1;
            insertSymTab(idName,0,value,address,para);
            // 生成四元式
            sprintf(temp,"%d",value);
            genMidCode(consts,ints,temp,idName);
        }
        else {
            error(AFTERASS_NOTIDEN_ERROR,line_num);
            exit(0);
        }
    }
    else {
        expect("an identifier");
    }
    getsym();
}

/*
常量声明子程序
<常量说明> ::= const<常量定义>;{const<常量定义>;}
*/

void constDecl() {
    getsym();
    int type;
    if (symId == KW_INT) {
        type = KW_INT;
        
    }
    else if (symId == KW_CHAR) {
        type = KW_CHAR;
    }
    else {
        type = -1;
        error(CONSTDEF_TYPE_ERROR,line_num);
        return;
    }
    constDef(type);
    while (symId == TK_COMMA) {
        constDef(type);
    }
    if (symId == TK_SEMICOLON) {
        getsym();
        if (symId == KW_CONST) {
            constDecl();
        }
        else {
            cout << "the analysis of const part finished. " << endl;
            _gramma << "the analysis of const part finished. " << endl;
        }
    }
    else {
        error(SEMICSYMMIS_ERROR,line_num);
        return;
    }
}

/*
变量定义子程序
<变量定义> ::= <类型标识符>(<标识符>|<标识符>'[' <无符号整数> ']')
              {(<标识符>|<标识符>'[' <无符号整数> ']')}
*/
void varDef() {
    if (symId == KW_INT || symId == KW_CHAR) {
        int tempType;
        tempType = symId;
        do {
            para = 0;
            getsym();
            if (symId == IDSYM) {
                strcpy(idName,token.c_str());
                getsym();
                
                if (symId == TK_OPENBR) { // array
                    getsym();
                    if (symId != TK_NUMBER) {
                        error(VARDEF_ARRAYINDEX_ERROR,line_num);
                        do {
                            getsym();
                        } while (symId != TK_SEMICOLON);
                        return ;
                    }
                    para = num;
                    getsym();
                    if (symId == TK_CLOSEBR) {
                        value = tempType;
                        address += para;
                        insertSymTab(idName,VAR,value,address,para);
                        cout <<"Line:"<<line_num<<"There is a array var defination" << endl;
                        _gramma <<"Line:"<<line_num<<"There is a array var defination" << endl;
                        sprintf(temp,"%d",para);
                        if (tempType == KW_INT) {
                            genMidCode(inta,space,temp,idName);
                        }
                        else {
                            genMidCode(chara,space,temp,idName);
                        }
                    }
                    else {
                        error(RMPARENSYMMIS_ERROR,line_num);
                        do {
                            getsym();
                        }
                        while (symId != TK_SEMICOLON);
                        return;
                    }
                    getsym();
                } // if (symId == TK_OPENBR)
                else {  // id
                    // cout << "here" << endl;
                    value = tempType;
                    address++;
                    para = -1;
                    insertSymTab(idName,VAR,value,address,para);
                    cout <<"Line:"<<line_num << "There is a var defination" << endl ;
                    _gramma <<"Line:"<<line_num << "There is a var defination" << endl ;
                    if (tempType == KW_INT) {
                        genMidCode(ints,space,space,idName);
                    }                
                    else if (tempType == KW_CHAR) {
                        genMidCode(chars,space,space,idName);
                    }
                }
            } // if (symId == IDSYM)
            else {
                error(VARDEF_TYPE_ERROR,line_num);
                do {
                    getsym();
                }
                while (symId != TK_SEMICOLON);
                return;
            }
    
        }
        while (symId == TK_COMMA);
        
    } //if (symId == KW_INT || symId == KW_CHAR) 
    else {
        error(UNDEFINED_TYPE_ERROR,line_num);
    }
    
}


/*
变量声明子程序
<变量声明> ::= <变量定义>;{<变量定义>}

并没有用到，因为和函数冲突的原因
*/
void varDecl() {
    do {
        varDef();
        
        if (symId != TK_SEMICOLON) {
            error(SEMICSYMMIS_ERROR,line_num);
            do{
                getsym() ;
            }
            while(symId != KW_INT && symId != KW_CHAR && symId != KW_VOID);
            return;
        }
        getsym();
    }
    while (symId == KW_INT || symId == KW_CHAR);
}

/*
复合语句子程序
<符合语句> ::= [<常量说明>][<变量说明>]<语句列>
*/
void compoundSta() {
    if (symId == KW_CONST) {
        constDecl();
    }
    if (symId == KW_INT || symId == KW_CHAR) {
        varDecl();
    }
    states();
}

/*
主函数子程序
<主函数> ::= void main '(' ')' '{' <复合语句> '}'
*/
void mainFunc() {
    
    if (symId != KW_MAIN) {
        error(MAINSYM_ERROR,line_num);
        return;
    }
    strcpy(idName,token.c_str());
    insertSymTab(idName,FUNC,-1,0,0);
    strcpy(procname,idName);
    genMidCode(func,space,space,procname);
    isMain = 1;
    getsym();
    if (symId != TK_OPENPA) {
        error(LPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_OPENPA);
        return;
    }
    getsym();
    if (symId != TK_CLOSEPA) {
        error(LPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_OPENPA);
        return;
    }
    getsym();
    if (symId != TK_BEGIN) {
        error(LBPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != KW_CONST && symId != KW_INT && symId != KW_CHAR 
                && symId != KW_IF && symId != KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN);
        return;
    }
    getsym();
    compoundSta();
    if (symId != TK_END) {
        error(RBPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != KW_CONST && symId != KW_INT && symId != KW_CHAR 
                && symId != KW_IF && symId != KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN);
        return;
    }
    genMidCode(myend,space,space,procname);
    _gramma << "it is a main function." << endl;
    cout << "it is a main function." << endl;
    getsym();

}

/*
表达式子程序
<表达式> ::= [+|-] <项>{<加法运算符> <项>}
*/
void expr() {
    factorType = 0;
    char place1[256];
    char place2[256];
    char place3[256];
    if (symId == TK_PLUS || symId == TK_MINUS) {
        factorType = KW_INT;
        if (symId == TK_PLUS) {
            getsym();
            item();
            strcpy(place3,nowitem);
        }
        else if (symId == TK_MINUS) {
            getsym();
            item();
            strcpy(place1,nowitem);
            strcpy(place3,genVar());
            char myzero[] = {'0','\0'};
            genMidCode(sub,myzero,place1,place3); //place3 = 0 - place1 
        }
        do {
            if (symId == TK_PLUS || symId == TK_MINUS) {
                factorType = KW_INT;
                strcpy(place1,place3);
                if (symId == TK_PLUS) {
                    getsym();
                    item();
                    strcpy(place2,nowitem);
                    strcpy(place3,genVar());
                    genMidCode(add,place1,place2,place3); // place3 = place1 + place2
                }
                else if (symId == TK_MINUS) {
                    getsym();
                    item();
                    strcpy(place2,nowitem);
                    strcpy(place3,genVar());
                    genMidCode(sub,place1,place2,place3); // place3 = place1 - place2
                }
            }
        }
        while (symId == TK_PLUS || symId == TK_MINUS);
        _gramma << "it is a expr." << endl;
        cout << "it is a expr" << endl;
        strcpy(nowitem,place3);
    }
    else {
        item();
        strcpy(place3,nowitem);
        do {
            if (symId == TK_PLUS || symId == TK_MINUS) {
                factorType = KW_INT;
                strcpy(place1,place3);
                if (symId == TK_PLUS) {
                    getsym();
                    item();
                    strcpy(place2,nowitem);
                    strcpy(place3,genVar());
                    genMidCode(add,place1,place2,place3); // place3 = place1 + place2
                }
                else if (symId == TK_MINUS) {
                    getsym();
                    item();
                    strcpy(place2,nowitem);
                    strcpy(place3,genVar());
                    genMidCode(sub,place1,place2,place3); // place3 = place1 - place2
                }
            }
        }
        while (symId == TK_PLUS || symId == TK_MINUS);
        _gramma << "it is a expr." << endl;
        cout << "it is a expr" << endl;
        strcpy(nowitem,place3);
    }
}

/*
项子程序
<项> :: <因子> {<乘法运算符><因子>}
*/
void item() {
    char place1[128],place2[128],place3[256];
    factor();
    strcpy(place3,nowitem);
    do {
        if (symId == TK_MUL || symId == TK_DIV) {
            strcpy(place1,place3);
            if (symId == TK_MUL) {
                getsym();
                factor();
                strcpy(place2,nowitem);
                strcpy(place3,genVar());
                genMidCode(mul,place1,place2,place3);
            }
            else {
                getsym();
                factor();
                strcpy(place2,nowitem);
                strcpy(place3,genVar());
                genMidCode(divs,place1,place2,place3);
            }
        }
    }
    while (symId == TK_MUL || symId == TK_DIV);
    strcpy(nowitem,place3);
}

/*
因子子程序
<因子> ::= <标识符> | <标识符> '[' <表达式> ']' | <整数> | <字符> 
         | <有返回值函数调用语句> | '(' <表达式> ')'
*/
void factor() {
    int t; // record if id is leagal
    char place3[256];
    strcpy(place3,space);
    // cout <<"!!!!!!!!!!!!!!!!" << endl;
    // cout << sym << endl;
    // cout << symId << endl;
    // cout << token <<endl;
    if (symId == IDSYM) {
        char tempIdname[Max];
        strcpy(idName,token.c_str());
        strcpy(tempIdname,token.c_str());

       
        getsym();
        
        if (symId == TK_OPENPA) { // func
            getsym();
            valueOfpara();
            if (symId != TK_CLOSEPA) {
                error(RPARENSYMMIS_ERROR,line_num);
                do {
                    getsym();
                }
                while (symId != TK_SEMICOLON);
                return;
            }
            t = searchSymTab(tempIdname,1);
            if (t == 0) {
                error(UNDEFFUNC_ERROR,line_num);
            }
            strcpy(place3,genVar());
            genMidCode(call,tempIdname,space,place3);
            strcpy(nowitem,place3);
            getsym();
        }
        else if (symId == TK_OPENBR) { // array
            getsym();
            int tempFactType = factorType;
            expr();
            factorType = tempFactType;
            char ttt[256];
            strcpy(ttt,nowitem);

            if (nowitem[0] >= '0' && nowitem[0] <= '9') {
                int arrLen = 0;
                arrLen = arrLength(idName);
                // 越界检查
                if (arrLen != -1 && atoi(ttt) >= arrLen) {
                    cout <<"Line: " <<line_num <<" Warning : Array out of bounds "<< line_num << endl;
                }
            }
            if (symId != TK_CLOSEBR) {
                error(RMPARENSYMMIS_ERROR,line_num);
                do {
                    getsym();
                }
                while (symId != TK_SEMICOLON && symId != TK_ASSIGN && symId != TK_PLUS
                    && symId != TK_MINUS && symId != TK_MUL && symId != TK_DIV 
                    && symId != TK_CLOSEPA && symId != TK_COMMA && symId != TK_GT
                    && symId != TK_LT && symId != TK_LEQ && symId != TK_GEQ
                    && symId != TK_EQ && symId != TK_NEQ);
                return;
            }
            t = searchSymTab(tempIdname,0);
            strcpy(nowitem,genVar());
            genMidCode(aAssign,tempIdname,ttt,nowitem);
            getsym();
        }
        else { // identifier is variable
            t = searchSymTab(idName,0);
            if (isconst) {  // isconst 在 searchSymTab(idName,0) 赋值
                strcpy(nowitem,int2array(t));
                factorType = TK_NUMBER;
                isconst = 0;
            }
            else { // var
                strcpy(nowitem,idName);
                // cout <<"&&&&&" << endl;
                // cout << nowitem << endl;
                // cout << sym << endl;
                // cout << symId << endl << endl;
            }
        }
    } // if (symId == IDSYM)
    else if (symId == TK_OPENPA) {  // 括号
        getsym();
        expr();
        if (symId != TK_CLOSEPA) {
            error(RPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != TK_SEMICOLON && symId != TK_ASSIGN && symId != TK_PLUS
                && symId != TK_MINUS && symId != TK_MUL && symId != TK_DIV 
                && symId != TK_CLOSEPA && symId != TK_COMMA && symId != TK_GT
                && symId != TK_LT && symId != TK_LEQ && symId != TK_GEQ
                && symId != TK_EQ && symId != TK_NEQ);
            return;
        }
        getsym();
    }
    // <整数> ::= [+|-]<无符号整数> | 0
    else if (symId == TK_NUMBER || symId == TK_PLUS || symId == TK_MINUS) {
        if (symId == TK_PLUS || symId == TK_MINUS) {
            int sign;
            if (symId == TK_PLUS) {
                sign = 1;
            }
            else {
                sign = -1;
            }
            getsym();
            if (symId != TK_NUMBER) {
                error(AFTEROP_NUMMIS_ERROR,line_num);
                do {
                    getsym();
                }
                while (symId != TK_SEMICOLON && symId != TK_ASSIGN && symId != TK_PLUS
                    && symId != TK_MINUS && symId != TK_MUL && symId != TK_DIV 
                    && symId != TK_CLOSEPA && symId != TK_COMMA && symId != TK_GT
                    && symId != TK_LT && symId != TK_LEQ && symId != TK_GEQ
                    && symId != TK_EQ && symId != TK_NEQ);
                return;
            }
            num = num * sign;
            sprintf(nowitem,"%d",num);
            factorType = TK_NUMBER; // ? what is the intention of the var?
            getsym();
        }
        else {
            sprintf(nowitem,"%d",num);
            factorType = TK_NUMBER;
            getsym();
        }
    }
    else if (symId == TK_CHAR) {
        factorType = TK_CHAR;
        sprintf(nowitem,"%d",token[0]);
        getsym();
    }
    else {
        error(FACTOR_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_SEMICOLON && symId != TK_ASSIGN && symId != TK_PLUS
                && symId != TK_MINUS && symId != TK_MUL && symId != TK_DIV 
                && symId != TK_CLOSEPA && symId != TK_COMMA && symId != TK_GT
                && symId != TK_LT && symId != TK_LEQ && symId != TK_GEQ
                && symId != TK_EQ && symId != TK_NEQ);
        return;
    }
}

/*
<有返回值函数调用语句> ::= <标识符> '(' <值参数表> ')'
<无返回值函数调用语句> ::= <标识符> '(' <值参数表> ')'
*/

void callSta(char * name) {
    getsym();

    valueOfpara(); // what is the work done by this func ?
    int t = searchSymTab(name,1);

    if (t == 0) {
        error(UNDEFFUNC_ERROR,line_num);
    }

    if (symId != TK_CLOSEPA) {
        error(RPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_SEMICOLON);

        return;
    } 
    genMidCode(call,name,space,space);
    getsym();
    if (symId != TK_SEMICOLON) {
        error(SEMICSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
        return;
    }
    getsym();
    _gramma << "Line:" << line_num << "it is a function call statement." << endl;
    cout << "Line:" << line_num << "it is a function call statement." << endl;
}

/*
赋值语句子程序
<赋值语句> ::= <标识符> = <表达式> | <标识符> '[' <表达式> ']' = <表达式>
*/

void assignSta() {
    char place1[128];
    char palce2[128];
    char place3[128];
    strcpy(place1,space);
    strcpy(palce2,space);
    strcpy(place3,space);

    char tempIdName[Max];
    strcpy(idName,token.c_str());
    strcpy(tempIdName,token.c_str());
    strcpy(place3,token.c_str());
    int t;

    getsym();

    if (symId == TK_ASSIGN) {
        t = searchSymTab(idName,0);

        getsym();
        expr();
        strcpy(place1,nowitem);
        if (symId != TK_SEMICOLON) {
            error(SEMICSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                  && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                  && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                  && symId != TK_END);
            return;
        }
        genMidCode(ass,place1,space,place3);
        getsym();
        _gramma << "Line:" << line_num << "it is a assign statement" << endl;
        cout << "Line:" << line_num << "it is a assign statement" << endl;
    }
    else if (symId == TK_OPENBR) {
        t = searchSymTab(idName,0);
        getsym();
        expr();
        strcpy(palce2,nowitem);
        if (symId != TK_CLOSEBR) {
            error(RMPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != TK_SEMICOLON);
            return;
        }
        getsym();
        if (symId == TK_ASSIGN) {
            getsym();
            expr();
            strcpy(place1,nowitem);
            if (symId != TK_SEMICOLON) {
                error(SEMICSYMMIS_ERROR,line_num);
                do {
                    getsym();
                }
                while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                    && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                    && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                    && symId != TK_END);
                return;
            }
            genMidCode(assignA,place1,palce2,place3);
        }
        _gramma << "Line:" << line_num << "it is a assign statement" << endl;
        cout << "Line:" << line_num << "it is a assign statement" << endl;
    }
    else if (symId == TK_OPENPA) {
        int t_isVoid = isVoid;
        callSta(tempIdName);
        isVoid = t_isVoid;
    }
    else {
        error(ASSIGNSTATUS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
        return;
    }

}

/*
语句子程序
<语句> ::= <条件语句> | <循环语句> | '{' <语句列> '}'
          | <有返回值函数调用语句>; |  <无返回值函数调用语句>;
          | <赋值语句>; | <读语句>; | <写语句>; | <空>;
          | <返回语句>;
*/
void state() {
    // 条件语句
   
    if (symId == KW_IF) {
        conditionSta();
        return;
    }
    else if (symId == KW_SWITH) {
        switchSta();
        return;
    }
    // 循环语句
    else if (symId == KW_WHILE || symId == KW_DO || symId == KW_FOR) {
        loopSta();
        return;
    }
    // 语句列
    else if (symId == TK_BEGIN) {
        getsym();
        states();
        if (symId != TK_END) {
            error(RBPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                  && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                  && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                  && symId != TK_END);
            return;
        }
        getsym();
        return;
    }
    // 赋值语句 与 函数调用语句
    else if (symId == IDSYM) {
        assignSta();
    }
    else if (symId == KW_PRINT) {
        writeSta();
        if (symId != TK_SEMICOLON) {
            error(SEMICSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                    && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                    && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                    && symId != TK_END);
            return;
        }
        getsym();
    }
    else if (symId == KW_SCANF) {
        readSta();
        if (symId != TK_SEMICOLON) {
            error(SEMICSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                    && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                    && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                    && symId != TK_END);
            return;
        }
        
        getsym();
    }
    else if(symId == KW_RETURN){
        returnSta();
        if (symId != TK_SEMICOLON) {
            error(SEMICSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                    && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                    && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                    && symId != TK_END);
            return;
        }
        getsym();
    }
    else if (symId == TK_SEMICOLON) {
        getsym();
    }
    else {
        return;
    }

}


/*
条件语句子程序
<条件语句> ::= if '(' <条件> ')' <语句> [else <语句>]
*/

void conditionSta() {
    char label1[256];  // else的开始
    char label2[256];  // 整个条件语句的结束
    char conditionValue[512];

    strcpy(label1,genLab());
    strcpy(label2,genLab());

    if (symId == KW_IF) {
        getsym();
        if (symId != TK_OPENPA) {
            error(LPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != TK_SEMICOLON);
            return;
        }
        getsym();
        condition();
        strcpy(conditionValue,nowitem);
        genMidCode(jne,space,space,label1); // condition 为假跳到label1执行else
        if (symId != TK_CLOSEPA) {
            error(RPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != TK_SEMICOLON);
            return;
        }
        // condition时成立时的中间代码生成
        getsym();
        state();
        genMidCode(jmp,space,space,label2); // 不执行else
        // 开始生成else的中间代码
        genMidCode(lab,space,space,label1); // 对label1进行回填 
        if (symId == KW_ELSE) {
            getsym();
            state();
        }
        genMidCode(lab,space,space,label2); // 对lab2进行回填
        _gramma << "Line:" << line_num << "it is a condition statement" << endl;
        cout << "Line:" << line_num << "it is a condition statement" << endl;

    }
}
/*
＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞ ‘}’
＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞}
＜情况子语句＞  ::=  case＜常量＞：＜语句＞

*/
class switchTable {
public:
    char value[16];
    char lable[16];
};

vector<switchTable> table_swith;

void sw_conditionSta(char * next) {
    char label[16];
    strcpy(label,genLab());
    genMidCode(lab,space,space,label);
    switchTable t;
    strcpy(t.lable,label);
    getsym();
    sprintf(t.value,"%d",num);
    
    table_swith.push_back(t);
    if (symId == TK_NUMBER) {
        getsym();
        skip(':');
        state();
    }
    genMidCode(jmp,space,space,next);
}
void conditionTable(char * next) {
    sw_conditionSta(next);
    while (symId == KW_CASE) {
        sw_conditionSta(next);
    }
}
/*
＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞ ‘}’
＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞}
＜情况子语句＞  ::=  case＜常量＞：＜语句＞

*/
void switchSta() {
    char value_of_expr[128];
    char next[16];
    char test[16];
    strcpy(next,genLab());
    strcpy(test,genLab());
    getsym();
    skip('(');
    expr();
    strcpy(value_of_expr,nowitem);
    skip(')');
    genMidCode(jmp,space,space,test);
    skip('{');
    conditionTable(next);
    
    skip('}');

    //getsym();

    genMidCode(lab,space,space,test);

    for (int i = 0;i < table_swith.size();i++) {
        genMidCode(neq,value_of_expr,table_swith[i].value,space);
        genMidCode(jne,space,space,table_swith[i].lable);
    }
    genMidCode(lab,space,space,next);
    table_swith.clear();
}


/*
条件子程序
<条件> ::= <表达式><关系运算符><表达式> | <表达式>
表达式0为假，否则为真
*/
void condition() {
    char place1[128];
    char palce2[128];
    strcpy(place1,space);
    strcpy(palce2,space);

    expr();
    strcpy(place1,nowitem);
    if (symId == TK_GT) {
        getsym();
        expr();
        strcpy(palce2,nowitem);
        genMidCode(bt,place1,palce2,space);
    }
    else if (symId == TK_LT) {
        getsym();
        expr();
        strcpy(palce2,nowitem);
        genMidCode(st,place1,palce2,space);
    }
    else if (symId == TK_EQ) {
        getsym();
        expr();
        strcpy(palce2,nowitem);
        genMidCode(eql,place1,palce2,space);
    }
    else if (symId == TK_LEQ) {
        getsym();
        expr();
        strcpy(palce2,nowitem);
        genMidCode(nbt,place1,palce2,space);
    }
    else if (symId == TK_GEQ) {
        getsym();
        expr();
        strcpy(palce2,nowitem);
        genMidCode(nst,place1,palce2,space);
    }
    else if (symId == TK_NEQ) {
        getsym();
        expr();
        strcpy(palce2,nowitem);
        genMidCode(neq,place1,palce2,space);
    }
    else if (symId == TK_CLOSEPA) { // 直接就一表达式
        genMidCode(neq,place1,zero,space);
    }
    else if (isFor && symId == TK_SEMICOLON) {
        genMidCode(neq,place1,zero,space);
    }
    else {
        error(CONDITIONOP_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
        return;
        
    }
    
}

/*
while循环子程序
<while-do> ::= while '(' <条件> ')' <语句>
*/
void whileSta() {
    
    char place1[128];

    char label1[256]; // while 循环内语句开始的label
    char label2[256]; // while-do结束后的label

    strcpy(label1,genLab());
    strcpy(label2,genLab());

    if (symId == KW_WHILE) {
        getsym();
        if (symId != TK_OPENPA) {
            error(LPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                    && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                    && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                    && symId != TK_END);
            return;
        }
        getsym();
        
        genMidCode(lab,space,space,label1);
        condition();
        if (symId != TK_CLOSEPA) {
            error(RPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                    && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                    && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                    && symId != TK_END);
            return;
        }
        genMidCode(jne,space,space,label2);
        getsym();
        state();
        genMidCode(jmp,space,space,label1);
        genMidCode(lab,space,space,label2);
        _gramma << "Line: " << line_num << " it is a while-do-loop statement" << endl;
        cout << "Line: " << line_num << " it is a do-while-loop statement" << endl;
    }
    else {
        expect("while");
    }
}
/*
do-while-loop 子程序
<do-while> ::= do <语句> while '(' <条件> ')' 
*/
void doWhileSta() {
    char place1[128];
    char place2[128];

    char label1[256]; // 循环内容语句的开始label
    char label2[256]; // do-while结束后的label

    strcpy(label1,genLab());
    strcpy(label2,genLab());

    genMidCode(lab,space,space,label1); // 对label1回填
    getsym();
    state();
    if (symId != KW_WHILE) {
        error(WHILESYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
        return;
    }
    getsym();
    if (symId != TK_OPENPA) {
        error(LPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
        return;
    }
    getsym();
    condition();
    if (symId != TK_CLOSEBR) {
        error(RPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
        return;
    }
    genMidCode(jne,space,space,label2);
    genMidCode(jmp,space,space,label1);
    genMidCode(lab,space,space,label2);
    _gramma << "Line: " << line_num << " it is a do-while-loop statement" << endl;
    cout << "Line: " << line_num << " it is a do-while-loop statement" << endl;
    
}
/*
for-lopp 子程序
<for-loop> ::= for '(' <标识符>=<表达式>;<条件>;<标识符>=<标识符>(+|-)<步长> ')' <语句>
暂时不支持
*/
void forloopSta() {
    // char label1[256]; // 循环内容语句的开始label
    // char label2[256]; // for-loop 结束后的label
    
    // getsym();
    // skip('(');
    // getsym();
    // char name1[Max];
    // char name2[Max];
    // char name3[Max];
    // isFor = 1;
    // int s;
    // if (symId == IDSYM) {
    //     strcpy(name1,token.c_str());
    //     assignSta();

    //     genMidCode(lab,space,space,label1);
    //     condition();
    //     genMidCode(jne,space,space,label2);
    //     if (symId != TK_SEMICOLON,line_num) {
    //         error(SEMICSYMMIS_ERROR,line_num);
    //         do {
    //             getsym();
    //         }
    //         while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
    //                 && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
    //                 && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
    //                 && symId != TK_END);
    //         return;
    //     }
    //     getsym();
    //     if (symId != IDSYM) {
    //         error(IDSYMMIS_ERROR,line_num);
    //         do {
    //             getsym();
    //         }
    //         while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
    //                 && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
    //                 && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
    //                 && symId != TK_END);
    //         return;
    //     }
    //     strcpy(name2,token.c_str());
    //     s = searchSymTab(name2,0);
    //     if (isconst) {
    //         error(VARTYPE_ERROR,line_num);
    //         isconst = 0;
    //     }
    //     getsym();
    //     if (symId != TK_ASSIGN) {
    //         error(FORASSIGNMIS_ERROR,line_num);
    //         do {
    //             getsym();
    //         }
    //         while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
    //                 && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
    //                 && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
    //                 && symId != TK_END);
    //         return;
    //     }
    //     getsym();
    //     if (symId != IDSYM) {
    //         error(IDSYMMIS_ERROR,line_num);
    //         do {
    //             getsym();
    //         }
    //         while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
    //                 && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
    //                 && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
    //                 && symId != TK_END);
    //         return;
    //     }
    //     strcpy(name3,token.c_str());
    //     s = searchSymTab(name3,0);
    //     if (isconst) {
    //         error(VARTYPE_ERROR,line_num);
    //         isconst = 0;
    //     }
    //     getsym();
    //     if (symId == TK_PLUS || symId == TK_MINUS) {
    //         int sign;
    //         if (symId == TK_PLUS) {
    //             getsym();
    //         }
    //     }

    // }

}

/*
<循环语句> ::= <do-while>
  | <for-loop>
  |  <while-do>
*/
void loopSta() {
   
    if (symId == KW_DO) {
        doWhileSta();
    } // if (symId == KW_DO)
    else if (symId == KW_WHILE) {
        whileSta();
    }
    else if (symId == KW_FOR) {
        forloopSta();
    }
    
}

/*

<值参数表> ::= <表达式>{，<表达式>} | <空>
<表达式>   ::= [+|-]<项>>{<加法运算符><项>} 包含终结符 + -

<项>     ::= <因子>{<乘法运算符><因子>}
<因子>   ::= <标识符>|<标识符>'['<表达式>']'|<整数>|<字符>|<有返回值函数调用语句>|'('<表达式>')'
包含终结符 标识符 整数 字符 (

*/

void valueOfpara() {
    para = 0;
    vector<string> v;
    do {
        if (symId == TK_COMMA) {
            getsym();
        }
        if (symId == TK_PLUS || symId == TK_MINUS || symId == IDSYM
           || symId == TK_NUMBER || symId == TK_CHAR || symId == TK_OPENPA) 
        {
            expr();
            v.push_back(nowitem);
            para++;
        }
    }
    while (symId == TK_COMMA);
    char t[32];
    int l = v.size();
    for (int i = 0;i < l;i++) {
        strcpy(t,v[i].c_str());
        genMidCode(calpara,space,space,t);
    }
}

/*
参数子程序(用于函数定义)
<参数> ::= <参数表>
<参数表> ::= <类型标识符><标识符>{,<类型标识符><标识符>} | <空>
*/
void paraTable() {
    para = 0;
    int temp;
    do {
        if (symId == TK_COMMA) {
            getsym();
        }
        if (symId == KW_INT || symId == KW_CHAR) {
            temp = symId;
            getsym();
            if (symId != IDSYM) {
                error(IDSYMMIS_ERROR,line_num);
                do {
                    getsym();
                }
                while (symId != TK_CLOSEPA);
                return;
            }
            strcpy(idName,token.c_str());
            value = -1;
            address++;
            insertSymTab(idName,3,value,address,para + 1);
            if (temp == KW_INT) {
                genMidCode(paraop,ints,space,idName);
            }
            else if (temp == KW_CHAR) {
                genMidCode(paraop,chars,space,idName);
            }
            para++;
            getsym();
        }
    }
    while (symId == TK_COMMA);
    insertPara(para);
}

/*
语句列子程序
<语句列> ::= {<语句>}
*/
void states() {
    do {
        state();
    }
    while (
        symId == KW_IF || symId == KW_DO || symId == KW_WHILE
        || symId == KW_FOR || symId == IDSYM || symId == KW_SCANF
        || symId == KW_RETURN || symId == KW_PRINT || symId == KW_SWITH
        || symId == TK_BEGIN || symId == TK_SEMICOLON
    );
}

/*
读语句子程序
<读语句> :: scanf'(' <标识符>{,<标识符>} ')'
*/

void readSta() {
    char name[32];
    int s;
    if (symId == KW_SCANF) {
        getsym();
        if (symId == TK_OPENPA) {
            do {
                getsym();
                if (symId != IDSYM) {
                    error(IDSYMMIS_ERROR,line_num);
                    do {
                        getsym();
                    }
                    while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                            && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                            && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                            && symId != TK_END);
                    return;
                }
                strcpy(name,token.c_str());
                s = searchSymTab(name,0);
                if (s == -2) {
                    error(VARTYPE_ERROR,line_num);
                }
                if (IsArray()) {
                    error(READARRAY_ERROR,line_num);
                }
                genMidCode(scf,space,factorType == KW_INT ? ints : chars,name);
                getsym();
            }
            while (symId == TK_COMMA);

            if (symId != TK_CLOSEPA) {
                error(RPARENSYMMIS_ERROR,line_num);
                do {
                    getsym();
                }
                while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                        && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                        && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                        && symId != TK_END);
                return;
            }
            _gramma << "Line:" << line_num << "it is a read statement." << endl;
            cout << "Line:" << line_num << "it is a read statement." << endl;
        }
        else {
            error(LPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                    && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                    && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                    && symId != TK_END);
            return;
        }
    }
    getsym();
}

/*
写语句子程序
＜写语句＞    ::= printf ‘(’ ＜字符串＞,＜表达式＞ ‘)’| 
         printf ‘(’＜字符串＞ ‘)’| printf ‘(’＜表达式＞‘)’
*/
void writeSta() {
    char place1[256];
    char place2[256];
    strcpy(place1,space);
    strcpy(place2,space);

    getsym();
    if (symId != TK_OPENPA) {
        error(LPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
        return;
    }
    getsym();
    if (symId == TK_STRING) {
        strcpy(place1,token.c_str());
        getsym();
        if (symId == TK_COMMA) {
            getsym();
            expr();
            if (symId != TK_CLOSEPA) {
                error(RPARENSYMMIS_ERROR,line_num);
                do {
                    getsym();
                }
                while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                    && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                    && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                    && symId != TK_END);
                return;
            }
            strcpy(place2,nowitem);
        }
        else if (symId != TK_CLOSEPA) {
            error(RPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
            return;
        }
    }
    else {
        expr();
        strcpy(place2,nowitem);
        if (symId != TK_CLOSEPA) {
            error(RPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
            return;
        }
    }
    getsym();
    if( factorType == KW_INT || factorType == TK_NUMBER) {
        genMidCode(prtf,place1,place2,ints);
    }
    else {
        genMidCode(prtf,place1,place2,chars);
    }
    _gramma << "Line: " << line_num << "it is a write statement." << endl;
    cout << "Line: " << line_num << "it is a write statement." << endl;
}

/*
返回语句子程序
<返回语句> ::= return ['(' <表达式> ')']
*/

void returnSta() {
    char place[32];
    getsym();
    if (isVoid && symId != TK_SEMICOLON) {
        error(UNEXCEPTRETURNVAL_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_SEMICOLON);
    }
    if (symId == TK_OPENPA) {
        hasRet = 1;
        getsym();
        expr();
        strcpy(place,nowitem);
        if (symId != TK_CLOSEPA) {
            error(RPARENSYMMIS_ERROR,line_num);
            do {
                getsym();
            }
            while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
                && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
                && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
                && symId != TK_END);
            return;
        }
        if (isMain) {
            genMidCode(exits,space,space,space);
        }
        else {
            genMidCode(ret,space,space,place);
        }
        getsym();
    }
    else if (symId == TK_SEMICOLON) {
        if (isMain) {
            genMidCode(exits,space,space,space);
        }
        else {
            genMidCode(ret,space,space,space);
        }
    }
    else {
        error (LPARENSYMMIS_ERROR,line_num);
        while (symId != TK_SEMICOLON) {
            getsym();
        }
        return;
    }
    if (symId != TK_SEMICOLON) {
        error(SEMICSYMMIS_ERROR,line_num);
        
        while (symId != KW_IF && symId == KW_FOR && symId != KW_DO
            && symId != TK_BEGIN && symId != IDSYM && symId != KW_PRINT
            && symId != KW_SCANF && symId != TK_SEMICOLON && symId != KW_RETURN
            && symId != TK_END) 
        {
            getsym();
        }
        return;
    }
    _gramma << "Line:" << line_num << " it is a return statement." << endl;
    cout << "Line:" << line_num << " it is a return statement." << endl;
}

/*
<有返回值函数定义> ::= <声明头部>'(' <参数> ')' '{' <复合语句> '}'
*/
void funcWithRetValDef() {
    hasRet = 0;
    tempSym = symId;
    declHead();
    if (symId != TK_OPENPA) {
        error(LPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_CLOSEPA);
        getsym();
        return;
    }
    if (tempSym == KW_INT) {
        genMidCode(func,ints,space,procname);
    }
    else if (tempSym == KW_CHAR) {
        genMidCode(func,chars,space,procname);
    }
    getsym();
    paraTable();
    if (symId != TK_CLOSEPA) {
        error(RPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_BEGIN);
        getsym();
        return;
    }

    getsym(); // should be '{'
    if (symId != TK_BEGIN) {
        error(LPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_END);
        getsym();
        return;
    }
    getsym();
    compoundSta();
    if (symId != TK_END) {
        error(RPARENSYMMIS_ERROR,line_num);
        while (symId != KW_VOID && symId != KW_INT && symId != KW_CHAR) {
            getsym();
        }
        return;
    }
    if (hasRet == 0) { // hasRet在returnSta()中赋值，标记函数内是否有带表达式的return
        cout << "Line:" << line_num << "expected return value missed" << endl;
    }
    getsym();
    genMidCode(myend,space,space,procname);
    _gramma << "Line:" << line_num <<"it is a valuable-function defination statemnet." <<endl;
    cout << "Line:" << line_num <<"it is a valuable-function defination statemnet." <<endl;
    hasRet = 0;
}

/*
<无返回值的函数定义> ::= void<标识符>'('<参数>')''{'<复合语句>'}'
*/
void funcWithNotRetValDef() {
    int tempValue;
    int tempAddr;
    char tempIdName[Max];

    isVoid = 1;
    if (symId != IDSYM) {
        error(IDSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_END);
        return;
    }
    strcpy(tempIdName,token.c_str());
    strcpy(idName,token.c_str());

    tempValue -10;
    value = 0; // ?
    tempAddr = 0;
    address = 0;
    insertSymTab(idName,2,value,address,para);
    strcpy(procname,idName);
    genMidCode(func,vod,space,procname);

    getsym();
    if (symId != TK_OPENPA) {
        error(LPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_CLOSEPA);
        return;
    }

    getsym();
    paraTable();
    insertPara(para);

    if (symId != TK_CLOSEPA) {
        error(RPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != TK_END);
        return;
    }

    getsym();
    if (symId != TK_BEGIN) {
        error(LBPARENSYMMIS_ERROR,line_num);
        do {
            getsym();
        }
        while (symId != RBPARENSYMMIS_ERROR);
        return;
    }

    getsym();
    if (symId != TK_END) {
        compoundSta();
    }

    if (symId != TK_END) {
        error(RPARENSYMMIS_ERROR,line_num);
        while (symId != KW_VOID && symId != KW_INT && symId != KW_CHAR) {
            getsym();
        }
        return;
    }
    getsym();
    genMidCode(ret,space,space,space);
    genMidCode(myend,space,space,procname);
    isVoid = 0;
    _gramma <<"Line: " << line_num << "it is a void function-defination statement" << endl;
    cout <<"Line: " << line_num << "it is a void function-defination statement" << endl;
}


/*
<程序> ::= [<常量说明>][<变量说明>]
   {<有返回值函数定义> | <无返回值函数定义>} <主函数>
*/
void program() {
    string id_func;
    if (symId == KW_CONST) {
        constDecl();
    }
    while (symId == KW_INT || symId == KW_CHAR) {
        pre_symId = symId;
        preCh = ch;
        preIndex = line_num;
        preToken = token;
        getsym();
        if (symId != IDSYM) {
            error (IDSYMMIS_ERROR,line_num);
            return;
        }
        strcpy(idName,token.c_str());
        id_func = token;
        getsym();

        if (symId == TK_COMMA || symId == TK_OPENBR) {
            
            
            ungetc(ch,fin);
            if (symId == TK_COMMA) {
                ungetc(',',fin);
            }
            else {
                ungetc('[',fin);
            }
            symId = pre_symId;
            token = preToken;
            string s(id_func.rbegin(),id_func.rend());
            for (int i = 0;i < s.length();i++) {
                char t = s[i];
                ungetc(t,fin);
                // cout << t << endl;
            }
            getch();
            varDef();
            getsym();
        }
        else if (symId == TK_SEMICOLON) {
            value = pre_symId;
            address++;
            para = -1;
            insertSymTab(idName,VAR,value,address,para);
            if (pre_symId == KW_INT) {
                genMidCode(ints,space,space,idName);
            }
            else if (pre_symId == KW_CHAR){
                genMidCode(chars,space,space,idName);
            }
            getsym();
        }
        else {
            symId = pre_symId;
            
            token = preToken;

            ungetc(ch,fin);
            ungetc('(',fin);
            string s(id_func.rbegin(),id_func.rend());
            for (int i = 0;i < s.length();i++) {
                char t = s[i];
                ungetc(t,fin);
            }
            getch();
            break;
        }
    }
    // function
    while (symId == KW_INT || symId == KW_CHAR || symId == KW_VOID) {
        if (symId == KW_INT || symId == KW_CHAR) {
            funcWithRetValDef();
        }
        else if (symId == KW_VOID) {
            
            
            getsym();
            if (symId == KW_MAIN) {
                mainFunc();
                break;
            }
            else {
                funcWithNotRetValDef();
            }
        }
    }
}

void init() {
    symTable.index = 0;
    symTable.indexOfPre[0] = 0;
    symTable.toltalPre = 1; // 对应主函数
}


void stt() {
    int i = symTable.index ;
    int k = 0 ;

    ofstream debug;

    debug.open("../src/debug.txt", ios::out) ;
    debug << "------------------------------symbol table-------------------------------------" << endl ;
    debug << "--name-----------type----------value----------address---------para--" << endl ;
    for(int n = 0 ; n < i ; n++){
        debug   << symTable.element[n].name << "\t\t\t\t"
                << symTable.element[n].type << "\t\t\t\t"
                << symTable.element[n].value << "\t\t\t\t"
                << symTable.element[n].address << "\t\t\t\t"
                << symTable.element[n].para << endl ;

        if(n != 0 && n == symTable.indexOfPre[k]){
            debug << "-----------------------------------------------------" << endl ;
            k++ ;
        }
    }

    debug.close() ;

}
