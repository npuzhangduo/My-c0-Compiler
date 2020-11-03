#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <string.h>
#include "grammer.cpp"

using namespace std;

ofstream rstfile;

int stringCnt = 0;

typedef struct consttype {
    char name[512];
    char value[512];
} GlbConst;

GlbConst glbconst[512];
int cntOfGlobal = 0;

typedef struct localvar {
    char name[512];
    int addr;
} localVar;

localVar convar[512];
int localnum;

int paranum = 0;
int calparanum = 0;
int donenum = 0;
int offset = 0;


// 在convar数组中寻找名字为name的变量地址，找到则返回该地址，否则返回-1.
int findaddr(char * name) {
    int i = 0;
    while (i < localnum) {
        if (strcmp(name,convar[i].name) == 0) {
            return convar[i].addr;
        }
        i++;
    }
    return -1;
}

void insertaddr(char * name) {
    strcpy(convar[localnum].name,name);
    convar[localnum].addr = offset;
    rstfile << "\t\tsubi\t$sp\t$sp\t4" << endl;
    offset += 4;
    localnum++;
}

void globalcondef() {
    int i = 0;
    while (strcmp(mid_code[i].op,consts) == 0) {
        strcpy(glbconst[cntOfGlobal].name,mid_code[i].rst);
        strcpy(glbconst[cntOfGlobal].value,mid_code[i].num_b);
        rstfile << mid_code[i].rst << ":\t.word\t" << mid_code[i].num_b << endl ;
        cntOfGlobal++;
        i++;
    }
}

void globalvardef() {
    int i = cntOfGlobal;
    while (strcmp(mid_code[i].op,ints) == 0 || strcmp(mid_code[i].op,chars) == 0
            || strcmp(mid_code[i].op,inta) == 0 || strcmp(mid_code[i].op,chara) == 0) 
    {
        if (strcmp(mid_code[i].op,inta) == 0 || strcmp(mid_code[i].op,chara) == 0) {
            strcpy(glbconst[cntOfGlobal].name,mid_code[i].rst);
            strcpy(glbconst[cntOfGlobal].value,space);
            int step = atoi(mid_code[i].num_b);
            rstfile << mid_code[i].rst << ":\t.space\t" << step * 4 << endl;
            cntOfGlobal++;
            i++; 
        }
        else {
            strcpy(glbconst[cntOfGlobal].name,mid_code[i].rst);
            strcpy(glbconst[cntOfGlobal].value,mid_code[i].num_b);
            rstfile << mid_code[i].rst << ":\t.space\t" << "4" << endl ;
            cntOfGlobal++ ;
            i++;
        }
    }
    donenum = cntOfGlobal;
}

void initasm() {
    int i = 0;
    
    rstfile << ".data" << endl;
    globalcondef();
    globalvardef();
    i = cntOfGlobal;
    while (i < codeNum) {
        if (strcmp(mid_code[i].op,prtf) == 0) {
            if (strcmp(mid_code[i].num_a,space) != 0) {
                char temp[] = {'$','s','t','r','i','n','g','\0'} ;
                char * p = int2array(stringCnt);
                strcat(temp,p);  // p追加到temp的串尾

                strcpy(glbconst[cntOfGlobal].name,temp);
                strcpy(glbconst[cntOfGlobal].value,mid_code[i].num_a);
                strcpy(mid_code[i].num_a,temp);
                cout << temp << endl;
                rstfile << temp << ":\t.asciiz\t" << "\"" << glbconst[cntOfGlobal].value << "\"" << endl;
                cntOfGlobal++;
                stringCnt++;
            }
        }
        i++;
    }
    rstfile << ".text" << endl;
    rstfile << ".globl main" << endl;
    rstfile << "\t\tmove\t$fp\t$sp" << endl;
    rstfile << "\t\tj\tmain" << endl;
}

int isNum(char a) {
    if (a >= '0' && a <= '9') {
        return 1;
    }
    return 0;
}

void func_mips() {
    rstfile << endl << endl;
    paranum = 0;
    //rstfile << "\t\t# func fourvarcode" << endl;
    //rstfile << "\t\t# Save Register" <<endl;
    rstfile << mid_code[donenum].rst << ":" << endl;

    rstfile << "\t\tsubi\t$sp\t$sp\t4" << endl; // $sp -= 4
    // rstfile << "\t\tsubi\t$sp\t$sp\t" << calparanum * 4 << endl ;
    // rstfile << "\t\t#func fourvarcode finished" << endl;//

    offset = 8; // fp ra
}

void myend_mips() {
    localnum = 0;
}

void ret_mips() {
    // rstfile << "\t\t# ret fourvarcode" << endl ;
    if (strcmp(mid_code[donenum].rst,space) != 0) {
        if (isNum(mid_code[donenum].rst[0])) {
            rstfile << "\t\tli\t$v1\t" << mid_code[donenum].rst << endl; // $v0 $v1放返回值
        }
        else {
            int addr1 = -1 * findaddr(mid_code[donenum].rst);
            if (addr1 == 1) { // global
                rstfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                rstfile << "\t\tlw\t$v1\t($t0)" << endl;
            }
            else {
                rstfile << "\t\tlw\t$v1\t" << addr1 << "($fp)" << endl;
            }
        }
    }
    rstfile << "\t\tmove\t$t0\t$ra" << endl; // save $ra
    rstfile << "\t\tlw\t$ra\t-4($fp)" << endl;  // rstore pre $ra
    rstfile << "\t\tadd\t$sp\t$fp\t$zero" << endl; // sp = fp
    rstfile << "\t\tlw\t$fp\t($fp)" << endl; // fp = pre fp
    rstfile << "\t\tjr\t$t0" << endl;
}

void ints_mips() {
    // local num
    strcpy(convar[localnum].name,mid_code[donenum].rst);
    convar[localnum].addr = offset;
    rstfile << "\t\tsubi\t$sp\t$sp\t4" << endl;

    localnum ++;
    offset += 4;
}

void chars_mips() {
    strcpy(convar[localnum].name,mid_code[donenum].rst);
    convar[localnum].addr = offset;
    rstfile << "\t\tsubi\t$sp\t$sp\t4" << endl;
    localnum++;
    offset += 4;
}

void inta_mips() {
    strcpy(convar[localnum].name,mid_code[donenum].rst);
    convar[localnum].addr = offset;
    int step;
    step = atoi(mid_code[donenum].num_b);
    rstfile << "\t\tsubi\t$sp\t$sp\t" << step * 4 << endl;
    offset += step * 4;
    localnum ++;
}

void chara_mips() {
    strcpy(convar[localnum].name,mid_code[donenum].rst);
    convar[localnum].addr = offset;
    int step;
    step = atoi(mid_code[donenum].num_b);
    rstfile << "\t\tsubi\t$sp\t$sp\t" << step * 4 << endl;
    offset += step * 4;
    localnum ++;
}


void add_mips() {
    int addr1;
    int addr2;
    int addr3;
    if (isNum(mid_code[donenum].num_a[0]) || mid_code[donenum].num_a[0] == '-') {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0]) || mid_code[donenum].num_b[0] == '-') {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1\t($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        }
    }

    rstfile << "\t\tadd\t$t0\t$t0\t$t1" << endl;

    if (mid_code[donenum].rst[0] == '$' ) {
        insertaddr(mid_code[donenum].rst);
        addr3 = -1 * findaddr(mid_code[donenum].rst);
        rstfile << "\t\tsw\t$t0\t" << addr3 << "($fp)" << endl;
    }
    else {
        addr3 = -1 * findaddr(mid_code[donenum].rst);
        if (addr3 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].rst << endl;
            rstfile << "\t\tsw\t$t0\t($t1)" << endl;
        }
        else {
            rstfile << "\t\tsw\t$t0\t" << addr3 << "($fp)" << endl;
        }
    }
}


void sub_mips() {
    int addr1;
    int addr2;
    int addr3;
    if (isNum(mid_code[donenum].num_a[0]) || mid_code[donenum].num_a[0] == '-') {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0]) || mid_code[donenum].num_b[0] == '-') {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1\t($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        }
    }

    rstfile << "\t\tsub\t$t0\t$t0\t$t1" << endl;

    if (mid_code[donenum].rst[0] == '$' ) {
        insertaddr(mid_code[donenum].rst);
        addr3 = -1 * findaddr(mid_code[donenum].rst);
        rstfile << "\t\tsw\t$t0\t" << addr3 << "($fp)" << endl;
    }
    else {
        addr3 = -1 * findaddr(mid_code[donenum].rst);
        if (addr3 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].rst << endl;
            rstfile << "\t\tsw\t$t0\t($t1)" << endl;
        }
        else {
            rstfile << "\t\tsw\t$t0\t" << addr3 << "($fp)" << endl;
        }
    }
}

void mul_mips() {
    int addr1;
    int addr2;
    int addr3;
    if (isNum(mid_code[donenum].num_a[0]) || mid_code[donenum].num_a[0] == '-') {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0]) || mid_code[donenum].num_b[0] == '-') {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1\t($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        }
    }
    rstfile << "\t\tmult\t$t0\t$t1" << endl;
    rstfile << "\t\tmfhi\t$t1" << endl;
    rstfile << "\t\tmflo\t$t0" << endl;
    
    if (mid_code[donenum].rst[0] == '$' ) {
        insertaddr(mid_code[donenum].rst);
        addr3 = -1 * findaddr(mid_code[donenum].rst);
        rstfile << "\t\tsw\t$t0\t" << addr3 << "($fp)" << endl;
    }
    else {
        addr3 = -1 * findaddr(mid_code[donenum].rst);
        if (addr3 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].rst << endl;
            rstfile << "\t\tsw\t$t0\t($t1)" << endl;
        }
        else {
            rstfile << "\t\tsw\t$t0\t" << addr3 << "($fp)" << endl;
        }
    }
}

void div_mips() {
    int addr1;
    int addr2;
    int addr3;
    if (isNum(mid_code[donenum].num_a[0]) || mid_code[donenum].num_a[0] == '-') {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0]) || mid_code[donenum].num_b[0] == '-') {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1\t($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        }
    }
    rstfile <<"\t\tdiv\t$t0\t$t1" << endl;
    rstfile << "\t\tmfhi\t$t1" << endl;
    rstfile << "\t\tmflo\t$t0" << endl;
    
    if (mid_code[donenum].rst[0] == '$' ) {
        insertaddr(mid_code[donenum].rst);
        addr3 = -1 * findaddr(mid_code[donenum].rst);
        rstfile << "\t\tsw\t$t0\t" << addr3 << "($fp)" << endl;
    }
    else {
        addr3 = -1 * findaddr(mid_code[donenum].rst);
        if (addr3 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].rst << endl;
            rstfile << "\t\tsw\t$t0\t($t1)" << endl;
        }
        else {
            rstfile << "\t\tsw\t$t0\t" << addr3 << "($fp)" << endl;
        }
    }
}

void prtf_mips() {
    if (strcmp(mid_code[donenum].num_a,space) != 0) {
        rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
        rstfile << "\t\tmove\t$a0\t$t0" << endl;  // 字符串地址给$a0
        rstfile << "\t\tli\t$v0\t4" << endl; // 设置操作模式 4 打印字符串
        rstfile << "\t\tsyscall" << endl;
        rstfile << "\t\tli\t$a0\t\'\\n\'" <<endl;
        rstfile << "\t\tli\t$v0\t11" << endl;  // ?
        rstfile << "\t\tsyscall" << endl;
    }
    if (strcmp(mid_code[donenum].num_b,space) == 0) {
        return;
    }
    int addr1 = -1 * findaddr(mid_code[donenum].num_b);
    if (addr1 == 1) { // global
        if (isNum(mid_code[donenum].num_b[0]) || mid_code[donenum].num_b[0] == '-') {
            rstfile << "\t\tli\t$a0\t" << mid_code[donenum].num_b << endl;
        }
        else {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$a0\t($t0)" << endl;
        }
    }
    else {
        rstfile << "\t\tlw\t$a0\t" << addr1 << "($fp)" << endl;
    }
    if (strcmp(mid_code[donenum].rst,ints) == 0) {
        rstfile << "\t\tli\t$v0\t1" << endl;
    }
    else if (strcmp(mid_code[donenum].rst,chars) == 0) {
        rstfile << "\t\tli\t$v0\t11" << endl;
    }
    rstfile << "\t\tsyscall" << endl;
    rstfile << "\t\tli\t$a0\t\'\\n\'" <<endl;
    rstfile << "\t\tli\t$v0\t11" << endl;  // ?
    rstfile << "\t\tsyscall" << endl;
}

void scf_mips() {
    int addr1 = -1 * findaddr(mid_code[donenum].rst);
    if (strcmp(mid_code[donenum].num_b,ints) == 0) {
        rstfile << "\t\tli\t$v0\t5" << endl;
    }
    else if (strcmp (mid_code[donenum].num_b,chars) == 0) {
        rstfile << "\t\tli\t$v0\t12" << endl;
    }
    rstfile << "\t\tsyscall" << endl;
    if (addr1 == 1) { // global
        rstfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
        rstfile << "\t\tsw\t$v0\t($t0)" << endl;
    }
    else {
        rstfile << "\t\tsw\t$v0\t" << addr1 << "($fp)" << endl;
    }
}

void lab_mips() {
    rstfile << mid_code[donenum].rst << ":" <<endl;
}

void paraop_mips() {
    // $a0 ~ $a3用于存储程序的前四个参数
    if (paranum < 4) {
        switch (paranum) {
        case 0:
            rstfile << "\t\tmove\t$t0\t$a0" << endl;
            rstfile << "\t\tsw\t$t0\t" << -8 << "($fp)" << endl;
            break;
        case 1 :
            rstfile << "\t\tmove\t$t0\t$a1" << endl;
            rstfile << "\t\tsw\t$t0\t" << -12 << "($fp)" << endl;
            break;
        case 2:
            rstfile << "\t\tmove\t$t0\t$a2" << endl;
            rstfile << "\t\tsw\t$t0\t" << -16 << "($fp)" << endl;
            break;
        case 3:
            rstfile << "\t\tmove\t$t0\t$a3" << endl;
            rstfile << "\t\tsw\t$t0\t" << -20 << "($fp)" << endl;
            break;
        default:
            break;
        }
    }
    insertaddr(mid_code[donenum].rst);
    paranum++;
}

void calpara_mips() {
    int addr1 = -1 * findaddr(mid_code[donenum].rst); // -1 表示地址从高地址向地址生长
    if (calparanum < 4) {
        switch (calparanum) {
        case 0:
            if (addr1 == 1) { // not find,is global
                if (isNum(mid_code[donenum].rst[0]) || mid_code[donenum].rst[0] == '-') {
                    rstfile << "\t\tli\t$a0\t" << mid_code[donenum].rst << endl; 
                }
                else {
                    rstfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                    rstfile << "\t\tlw\t$a0\t($t0)" << endl;
                }
            }
            else {
                rstfile << "\t\tlw\t$a0\t" << addr1 << "($fp)" << endl;
            }
            break;
        case 1:
            if (addr1 == 1) { // not find,is global
                if (isNum(mid_code[donenum].rst[0]) || mid_code[donenum].rst[0] == '-') {
                    rstfile << "\t\tli\t$a1\t" << mid_code[donenum].rst << endl; 
                }
                else {
                    rstfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                    rstfile << "\t\tlw\t$a1\t($t0)" << endl;
                }
            }
            else {
                rstfile << "\t\tlw\t$a1\t" << addr1 << "($fp)" << endl;
            }
            break;
        case 2:
            if (addr1 == 1) { // not find,is global
                if (isNum(mid_code[donenum].rst[0]) || mid_code[donenum].rst[0] == '-') {
                    rstfile << "\t\tli\t$a2\t" << mid_code[donenum].rst << endl; 
                }
                else {
                    rstfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                    rstfile << "\t\tlw\t$a2\t($t0)" << endl;
                }
            }
            else {
                rstfile << "\t\tlw\t$a2\t" << addr1 << "($fp)" << endl;
            }
            break;
        case 3:
            if (addr1 == 1) { // not find,is global
                if (isNum(mid_code[donenum].rst[0]) || mid_code[donenum].rst[0] == '-') {
                    rstfile << "\t\tli\t$a3\t" << mid_code[donenum].rst << endl; 
                }
                else {
                    rstfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                    rstfile << "\t\tlw\t$a3\t($t0)" << endl;
                }
            }
            else {
                rstfile << "\t\tlw\t$a3\t" << addr1 << "($fp)" << endl;
            }
            break;
        }
    }
    else { // not only 4 paraments
        if (addr1 == 1) { // global
            if (isNum(mid_code[donenum].rst[0]) || mid_code[donenum].rst[0] == '-') {
                rstfile << "\t\tli\t$t8\t" << mid_code[donenum].rst << endl;
            }
            else {
                rstfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                rstfile << "\t\tlw\t$t8\t($t0)" << endl;
            }
        }
        else {
            rstfile << "\t\tlw\t$t8\t" << addr1 << "($fp)" << endl;
        }
        rstfile << "\t\tsw\t$t8\t" << -1 * ((calparanum - 4)*4 + 24) << "($sp)" << endl ;
        // + 24的意义为留出 fp ，ra ，以及前四个参数的 地址空间
    }
    calparanum++;
}

void call_mips() {
    calparanum = 0;

    rstfile << "\t\tsw\t$fp\t($sp)" << endl; // 保存上一个函数的$fp
    rstfile << "\t\tadd\t$fp\t$sp\t$0" << endl; // 设置本函数的$fp = $sp,

    rstfile << "\t\tsubi\t$sp\t$sp\t4" << endl; // $sp -= 4
    rstfile << "\t\tsw\t$ra\t($sp)" << endl; // 保存$ra
    rstfile << "\t\tjal\t" << mid_code[donenum].num_a << endl;
    rstfile << "\t\tnop\n";
    // 处理返回值
    if (strcmp(mid_code[donenum].rst,space) != 0) {
        insertaddr(mid_code[donenum].rst);
        int addr1 = -1 * findaddr(mid_code[donenum].rst);

        rstfile << "\t\tsw\t$v1\t" << addr1 << "($fp)" << endl;
    }
}

void jmp_mips() {
    rstfile << "\t\tj\t" << mid_code[donenum].rst << endl;
}

void jne_mips() {
    rstfile << mid_code[donenum].rst << endl;
}


void bt_mips() { // >
    int addr1;
    int addr2;
    if (isNum(mid_code[donenum].num_a[0])) {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0])) {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "(fp)" << endl;
        }
    }
    rstfile << "\t\tble\t$t0\t$t1\t";
    // ble $t0,$t1,target
    // branch to target if $t0 <= $t1
}

void st_mips() { // <,a,b,   a < b
    int addr1;
    int addr2;
    if (isNum(mid_code[donenum].num_a[0])) {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0])) {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        }
    }
    rstfile << "\t\tbge\t$t0\t$t1\t";
    // bge $t0,$t1,target
    // branch to target if $t0 >= $t1
}

void eql_mips() { // ==
    int addr1;
    int addr2;
    if (isNum(mid_code[donenum].num_a[0])) {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0])) {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        }
    }
    rstfile << "\t\tbne\t$t0\t$t1\t";
    // bne $t0,$t1,target
    // branch to target if $t0 != $t1
}

void neq_mips() { // !=
    int addr1;
    int addr2;
    if (isNum(mid_code[donenum].num_a[0])) {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0])) {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        }
    }
    rstfile << "\t\tbeq\t$t0\t$t1\t";
    // beq $t0,$t1,target
    // branch to target if $t0 == $t1
}

void nbt_mips() { // <=
    int addr1;
    int addr2;
    if (isNum(mid_code[donenum].num_a[0])) {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0])) {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        }
    }
    rstfile << "\t\tbgt\t$t0\t$t1\t";
    // bgt $t0,$t1,target
    // branch to target if $t0 > $t1
}

void nst_mips() { // >=
    int addr1;
    int addr2;
    if (isNum(mid_code[donenum].num_a[0])) {  // 立即数
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {   // 全局变量
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {  // 局部变量
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }

    if (isNum(mid_code[donenum].num_b[0])) {
        rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
    }
    else {
        addr2 = -1 * findaddr(mid_code[donenum].num_b);
        if (addr2 == 1) {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1($t1)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        }
    }
    rstfile << "\t\tblt\t$t0\t$t1\t";
    // blt $t0,$t1,target
    // branch to target if $t0 < $t1
}

// a = b
// =,b, , a
void ass_mips() {
    int addr1;
    int addr2;
    if (isNum(mid_code[donenum].num_a[0]) || mid_code[donenum].num_a[0] == '-') {
        rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        addr1 = -1 * findaddr(mid_code[donenum].num_a);
        if (addr1 == 1) {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
        else {
            rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
        }
    }
    addr2 = -1 * findaddr(mid_code[donenum].rst);
    if (addr2 == 1) {
        rstfile << "\t\tla\t$t1\t" << mid_code[donenum].rst << endl;
        rstfile << "\t\tsw\t$t0\t($t1)" << endl;
    }
    else {
        rstfile << "\t\tsw\t$t0\t" << addr2 << "($fp)" << endl;
    }
}

// []= , b , i , a
// a[i] = b
void assignA_mips() { // [] = 
    int addr1 = -1 * findaddr(mid_code[donenum].num_a); // b的值
    int addr2 = -1 * findaddr(mid_code[donenum].num_b); // 偏移量
    int addr3 = -1 * findaddr(mid_code[donenum].rst); // 数组初地址

    if (addr1 == 1) {
        if (isNum(mid_code[donenum].num_a[0])) {
            rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
        }
        else {
            rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
            rstfile << "\t\tlw\t$t0\t($t0)" << endl;
        }
    }
    else {
        rstfile << "\t\tlw\t$t0\t" << addr1 << "($fp)" << endl;
    }

    if (addr2 == 1) {
        if (isNum(mid_code[donenum].num_b[0])) {
            rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
        }
        else {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1\t($t1)" << endl;
        }
        rstfile << "\t\tli\t$t2\t-4" << endl;
        rstfile << "\t\tmult\t$t1\t$t2" << endl;
        rstfile << "\t\tmflo\t$t1" << endl;
    }
    else {
        rstfile << "\t\tlw\t$t1\t" << addr1 << "($fp)" << endl;
        rstfile << "\t\tmult\t$t1\t$t2" << endl;
        rstfile << "\t\tmflo\t$t1" << endl;
    }

    if (addr3 == 1) {
        rstfile << "\t\tla\t$t2\t" << mid_code[donenum].rst << endl;
        rstfile << "\t\tadd\t$t1\t$t1\t$t2" << endl;
        rstfile << "\t\tsw\t$t0\t($t1)" << endl;
    }
    else {
        rstfile << "\t\tli\t$t2\t" << addr3 << endl;
        rstfile << "\t\tadd\t$t1\t$t2\t$t1" <<endl;
        rstfile << "\t\tadd\t$t1\t$t1\t$fp" << endl;
        rstfile << "\t\tsw\t$t0\t($t1)" << endl;
    }
}

// a = b[i]
// aassi,b,i,a
void aAssign_mips() {
    int addr1 = -1 * findaddr(mid_code[donenum].num_a); // 数组的初地址
    int addr2 = -1 * findaddr(mid_code[donenum].num_b); // 数组偏移量
    if (addr1 == 1) {
        rstfile << "\t\tla\t$t0\t" << mid_code[donenum].num_a << endl;
    }
    else {
        rstfile << "\t\tli\t$t0\t" << addr1 << endl;
        rstfile << "\t\tadd\t$t0\t$t0\t$fp" << endl;
    }

    if (addr2 == 1) {
        if (isNum(mid_code[donenum].num_b[0])) {
            rstfile << "\t\tli\t$t1\t" << mid_code[donenum].num_b << endl;
        }
        else {
            rstfile << "\t\tla\t$t1\t" << mid_code[donenum].num_b << endl;
            rstfile << "\t\tlw\t$t1\t($t1)" << endl;
        }
        rstfile <<"\t\tli\t$t2\t-4" << endl;
        rstfile << "\t\tmult\t$t1\t$t2" << endl;
        rstfile << "\t\tmflo\t$t1" << endl;
    }
    else {
        rstfile << "\t\tlw\t$t1\t" << addr2 << "($fp)" << endl;
        rstfile <<"\t\tli\t$t2\t-4" << endl;
        rstfile << "\t\tmult\t$t1\t$t2" << endl;
        rstfile << "\t\tmflo\t$t1" << endl;
    }

    rstfile << "\t\tadd\t$t0\t$t0\t$t1" << endl;
    rstfile << "\t\tlw\t$t0\t($t0)" << endl;
    insertaddr(mid_code[donenum].rst);                        // ????
    int addr3 = -1 * findaddr(mid_code[donenum].rst);
    rstfile << "\t\tsw\t$t0\t" << addr3 << "($fp)" << endl;
}

void exit_mips() {
    rstfile << "\t\tli\t$v0\t10" << endl;
    rstfile << "\t\tsyscall" << endl;
}

void run() {
    initasm();
    while (donenum < codeNum) {
        if (strcmp(mid_code[donenum].op,func) == 0) {
            // rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            func_mips();
        }
        else if (strcmp(mid_code[donenum].op,ints) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            ints_mips();
        }
        else if (strcmp(mid_code[donenum].op,chars) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            chars_mips();
        }
        else if (strcmp(mid_code[donenum].op,inta) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            inta_mips();
        } 
        else if (strcmp(mid_code[donenum].op,chara) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            chara_mips();
        }
        else if (strcmp(mid_code[donenum].op,add) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            add_mips();
        }
        else if (strcmp(mid_code[donenum].op,sub) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            sub_mips();
        }
        else if (strcmp(mid_code[donenum].op,mul) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            mul_mips();
        }
        else if (strcmp(mid_code[donenum].op,divs) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            div_mips();
        }
        else if (strcmp(mid_code[donenum].op,prtf) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            prtf_mips();
        }
        else if (strcmp(mid_code[donenum].op,scf) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            scf_mips();
        }
        else if (strcmp(mid_code[donenum].op,ret) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            ret_mips();
        }
        else if (strcmp(mid_code[donenum].op,lab) == 0) {
            //rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            lab_mips();
        }
        else if (strcmp(mid_code[donenum].op,paraop) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            paraop_mips();
        }
        else if (strcmp(mid_code[donenum].op,calpara) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            calpara_mips();
        }
        else if (strcmp(mid_code[donenum].op,call) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            call_mips();
        }
        else if (strcmp(mid_code[donenum].op,jne) == 0) {
            //rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            jne_mips();
        }
        else if (strcmp(mid_code[donenum].op,jmp) == 0) {
            //rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            jmp_mips();
        }
        else if (strcmp(mid_code[donenum].op,ass) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            ass_mips();
        }
        else if (strcmp(mid_code[donenum].op,bt) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            bt_mips();
        }
        else if (strcmp(mid_code[donenum].op,st) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            st_mips();
        }
        else if (strcmp(mid_code[donenum].op,eql) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            eql_mips();
        }
        else if (strcmp(mid_code[donenum].op,neq) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            neq_mips();
        }
        else if (strcmp(mid_code[donenum].op,nbt) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            nbt_mips();
        }
        else if (strcmp(mid_code[donenum].op,nst) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            nst_mips();
        }
        else if (strcmp(mid_code[donenum].op,assignA) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            assignA_mips();
        }
        else if (strcmp(mid_code[donenum].op,aAssign) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            aAssign_mips();
        }
        else if (strcmp(mid_code[donenum].op,exits) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            exit_mips();
        }
        else if (strcmp(mid_code[donenum].op,myend) == 0) {
            rstfile << "#\t" << mid_code[donenum].op << " " << mid_code[donenum].num_a << " " << mid_code[donenum].num_b << " " << mid_code[donenum].rst << endl ;
            myend_mips();
        }
        donenum++;
    }
    
}