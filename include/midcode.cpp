#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
#include <stdlib.h>

using namespace std;

const int maxmidcode = 1024;

typedef struct midcode {
    char op[8];
    char num_a[128];
    char num_b[128];
    char rst[256];
} FOURVARCODE;

FOURVARCODE mid_code[maxmidcode];

ofstream outputfile;
ofstream afteroutputfile;


char consts[] = {'c','o','n','s','t','\0'} ;
char ints[] = {'i','n','t',' ',' ','\0'} ;
char chars[] = {'c','h','a','r',' ','\0'} ;
char inta[] = {'i','n','t','a',' ','\0'} ;
char chara[] = {'c','h','a','r','a','\0'} ;
char myend[] = {'e','n','d',' ',' ','\0'} ;
char add[] = {'+',' ',' ',' ',' ','\0'} ;
char sub[] = {'-',' ',' ',' ',' ','\0'} ;
char mul[] = {'*',' ',' ',' ',' ','\0'} ;
char divs[] = {'/',' ',' ',' ',' ','\0'} ;
char func[] = {'f','u','n','c',' ','\0'} ;
char prtf[] = {'p','r','t','f',' ','\0'} ;
char scf[] = {'s','c','f',' ',' ','\0'} ;
char ret[] = {'r','e','t',' ',' ','\0'} ;
char lab[] = {'l','a','b',':',' ','\0'} ;
char paraop[] = {'p','a','r','a',' ','\0'} ;
char calpara[] = {'c','p','a','r','a','\0'} ;
char call[] = {'c','a','l','l',' ','\0'} ;
char jne[] = {'j','n','e',' ',' ','\0'} ;
char jmp[] = {'j','m','p',' ',' ','\0'} ;
char ass[] = {'=',' ',' ',' ',' ','\0'} ;
char bt[] = {'>',' ',' ',' ',' ','\0'} ;
char st[] = {'<',' ',' ',' ',' ','\0'} ;
char eql[] = {'=','=',' ',' ',' ','\0'} ;
char neq[] = {'!','=',' ',' ',' ','\0'} ;
char nbt[] = {'<','=',' ',' ',' ','\0'} ;
char nst[] = {'>','=',' ',' ',' ','\0'} ;
char assignA[] = {'[',']','=',' ',' ','\0'} ;
char aAssign[] = {'a','A','s','s',' ','\0'} ;
char exits[] = {'e','x','i','t','\0'} ;

char vod[] = {'v','o','i','d',' ','\0'} ;
char val[] = {'v','a','l','u','e','\0'} ;

char space[] = {' ',' ',' ',' ',' ','\0'} ; 
char zero[] = {'0',' ',' ',' ',' ','\0'} ;

int codeNum = 0;
int labelNum = 0;
int varNum = 0;

vector<FOURVARCODE> midCodeList;

char * int2array(int a);

void replace(char * result,char * a);
char * genLab();
char * genVar();

/*
将数字a转化为字符串
*/
char * int2array(int a) {
    char * temp = (char*)malloc(sizeof(char)*128);
    sprintf(temp,"%d",a);
    return temp;
}

void genMidCode(char * op,char * a,char * b,char * result) {
    if (strcmp(op,func) == 0) {
        outputfile << endl << endl;
    }
    if (strcmp(op,lab) == 0) {
        outputfile << "\t\t" << result << " :" << endl ;
    }
    else {
        outputfile << "\t\t" << op << ", " << a << ", " << b << ", " << result << " " << endl ;
    }
    strcpy(mid_code[codeNum].op,op);
    strcpy(mid_code[codeNum].num_a,a);
    strcpy(mid_code[codeNum].num_b,b);
    strcpy(mid_code[codeNum].rst,result);
    codeNum++;
}

char * genLab() {
    char * label = (char*)malloc(sizeof(char)*16);
    sprintf(label,"_LABEL_%d",labelNum);
    labelNum++;
    return label;
}

char * genVar() {
    char * var = (char*)malloc(sizeof(char)*16);
    sprintf(var,"$_%d",varNum);
    varNum++;
    return var;
}

int isnumber(char * a,int len) {
    for (int i = 0;i < len;++i) {
        if (a[i] < '0' || a[i] > '9') {
            return 0;
        }
    }
    return 1;
}

void replace(char * result,char * a) {
    for (int i = 0;i < codeNum;++i) {
        if (strcmp(mid_code[i].num_a,result) == 0) {
            strcpy(mid_code[i].num_a,a);
        }
        if (strcmp(mid_code[i].num_b,result) == 0) {
            strcpy(mid_code[i].num_b,a);
        }
        if (strcmp(mid_code[i].rst,result) == 0) {
            strcpy(mid_code[i].rst,a);
        }
    }
}

void combine() {
    for (int i = 0;i < codeNum;++i) {
        if (strcmp(mid_code[i].op,sub) == 0 &&
                isnumber(mid_code[i].num_a,strlen(mid_code[i].num_a)) &&
                isnumber(mid_code[i].num_b,strlen(mid_code[i].num_b)))
        {
            int a = atoi(mid_code[i].num_a) - atoi(mid_code[i].num_b);
            char b[512];
            strcpy(b,mid_code[i].rst);
            strcpy(mid_code[i].op,space);
            strcpy(mid_code[i].num_a,space);
            strcpy(mid_code[i].num_b,space);
            strcpy(mid_code[i].rst,space);
            replace(b,int2array(a));
        }
        else if (strcmp(mid_code[i].op,add) == 0 &&
                isnumber(mid_code[i].num_a,strlen(mid_code[i].num_a)) &&
                isnumber(mid_code[i].num_b,strlen(mid_code[i].num_b)))
        {
            int a = atoi(mid_code[i].num_a) + atoi(mid_code[i].num_b);
            char b[512];
            strcpy(b,mid_code[i].rst);
            strcpy(mid_code[i].op,space);
            strcpy(mid_code[i].num_a,space);
            strcpy(mid_code[i].num_b,space);
            strcpy(mid_code[i].rst,space);
            replace(b,int2array(a));
        }
        else if (strcmp(mid_code[i].op,mul) == 0 &&
                isnumber(mid_code[i].num_a,strlen(mid_code[i].num_a)) &&
                isnumber(mid_code[i].num_b,strlen(mid_code[i].num_b)))
        {
            int a = atoi(mid_code[i].num_a) * atoi(mid_code[i].num_b);
            char b[512];
            strcpy(b,mid_code[i].rst);
            strcpy(mid_code[i].op,space);
            strcpy(mid_code[i].num_a,space);
            strcpy(mid_code[i].num_b,space);
            strcpy(mid_code[i].rst,space);
            replace(b,int2array(a));
        }
        else if (strcmp(mid_code[i].op,divs) == 0 &&
                isnumber(mid_code[i].num_a,strlen(mid_code[i].num_a)) &&
                isnumber(mid_code[i].num_b,strlen(mid_code[i].num_b)))
        {
            int a = atoi(mid_code[i].num_a) / atoi(mid_code[i].num_b);
            char b[512];
            strcpy(b,mid_code[i].rst);
            strcpy(mid_code[i].op,space);
            strcpy(mid_code[i].num_a,space);
            strcpy(mid_code[i].num_b,space);
            strcpy(mid_code[i].rst,space);
            replace(b,int2array(a));
        }

        if (strcmp(mid_code[i].op,func) == 0) {
            afteroutputfile << endl << endl;
        }
        if (strcmp(mid_code[i].op,lab) == 0) {
            afteroutputfile << "\t\t" << mid_code[i].rst << ":"<<endl;
        }
        if(strcmp(mid_code[i].op, space) != 0){
            afteroutputfile << "\t\t" << mid_code[i].op << ", " << mid_code[i].num_a << ", " << mid_code[i].num_b << ", " << mid_code[i].rst << " " << endl ;
        }

    }
}