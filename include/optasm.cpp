#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <string.h>
#include <set>
#include <queue>
#include <list>
#include "grammer.cpp"

using namespace std;

ofstream optfile;
int stringCnt = 0;

typedef struct consttype {
    char name[512];
    char value[512];
    int reg_id;
    bool stack_valid;
    bool reg_valid;
} GlbConst;

GlbConst glbconst[512];
int cntOfGlobal = 0;

typedef struct localvar {
    char name[512];
    int addr;
    int reg_id;
    bool stack_valid;
    bool reg_valid;
} localVar;

vector<localVar> convar;
int localnum;

int paranum = 0;
int calparanum = 0;
int donenum = 0;
int offset = 0;

class regDescriptor {
public:
    int id;
    string name;
    int value;
    set <string> varNameSet;
    bool free;
};
vector <regDescriptor> regDes;

int findaddr(char * name);
void insertLocalReg(char * name,int id);
void insertLocalStack(char * name);
void load(string regName,int address);

void initLocalVar() {
    for (int i = 0;i < convar.size();i++) {
        convar[i].addr = -2;
        convar[i].reg_id = false;
        convar[i].stack_valid = false;
    }
}

void initRegDes() {
    for (int i = 0;i < 8;i++) {
        regDescriptor tmp;
        tmp.id = i;
        tmp.name = "no";
        regDes.push_back(tmp);
    }
    for (int i = 8;i < 15;i++) {
        regDescriptor tmp;
        tmp.id = i;
        tmp.name = "$t";
        char t = i  - 8 + '0';
        tmp.name.push_back(t);
        tmp.value = 0;
        tmp.varNameSet.clear();
        tmp.free = true;
        regDes.push_back(tmp);
    }
    for (int i = 15;i < 24;i++) {
        regDescriptor tmp;
        tmp.id = i;
        tmp.name = "no";
        regDes.push_back(tmp);
    }
    for (int i = 24;i < 26;i++) {
        regDescriptor tmp;
        tmp.id = i;
        tmp.name = "$t";
        char t = i  - 16 + '0';
        tmp.name.push_back(t);
        tmp.value = 0;
        tmp.varNameSet.clear();
        tmp.free = true;
        regDes.push_back(tmp);
    }
    for (int i = 27;i < 32;i++) {
        regDescriptor tmp;
        tmp.id = i;
        tmp.name = "no";
        regDes.push_back(tmp);
    }
}



vector< vector<FOURVARCODE> > blocks;
class RegAssign {
public:
    std::string a;
    std::string b;
    std::string rst;
};

std::string regIdToString(int id) {
    return regDes[id].name;
}

int regStringToid(std::string name) {
    for (int i = 0;i < 32;i++) {
        if (regDes[i].name == name)
            return regDes[i].id;
    }
    return -1;
}

int index[] = {8,9,10,11,12,13,14,15,24,25};


list<int> used_queue;

void move_to_end(int id) {
    for (list<int>::iterator iter = used_queue.begin();iter != used_queue.end();iter++) {
        if (*iter == id) {
            used_queue.erase(iter);
            used_queue.push_back(id);
            return;
        }
    }
}
// 在convar数组中寻找名字为name的变量地址，找到则返回该地址，否则返回-1.
int findaddr(char * name) {
    int i = 0;
    while (i < localnum) {
        if (strcmp(name,convar[i].name) == 0) {
            if (convar[i].reg_valid)
                return convar[i].reg_id;
            else {
                return -convar[i].addr;
            }
        }
        i++;
    }
    return -1;
}


void insertaddr(char * name) {
    strcpy(convar[localnum].name,name);
    convar[localnum].addr = offset;
    convar[localnum].reg_valid = false;
    convar[localnum].stack_valid = false;
    optfile << "\t\tsubi\t$sp\t$sp\t4" << endl;
    offset += 4;
    localnum++;
}


void deleteRegFromLocal(int id) {
    for (int i = 0;i < convar.size();i++) {
        if (convar[i].reg_id == id || convar[i].reg_valid) {
            convar[i].reg_id = -1;
            convar[i].reg_valid = false;
        }
    }
}

int findAddrInStack(char * name) {
    int i = 0;
    while (i < localnum) {
        if (strcmp(name,convar[i].name) == 0) {
            return convar[i].addr;
        }
        i++;
    }
    return -1;
}

int isInReg(char * varName) {
    int addr = findaddr(varName);
    if (addr > 0)
        return addr;
    return -1;
}
string getReg(char * varname) {
    
    int find_addr = findaddr(varname);
    // 就在寄存器中
    if (find_addr > 0) {
        move_to_end(find_addr);
        return regIdToString(find_addr);
    }
    // 找空闲
    for (int i = 0;i < 8;i++) {
        if (regDes[index[i]].free) {
            string reg_name = regIdToString(index[i]);
            used_queue.push_back(index[i]);
            load(reg_name,varname);
            return reg_name;
        }
    }
    // 无空闲，需要换出
    int front = used_queue.front();
    used_queue.pop_front();
    used_queue.push_back(front);
    for (set<string>::iterator iter = regDes[front].varNameSet.begin();iter != regDes[front].varNameSet.end();iter++) {
        string na = *iter;
        char nam[32];
        strcpy(nam,na.c_str());
        int addr = -1 * findAddrInStack(nam);
        if (addr == 1) { // 全局变量
            optfile << "\t\tla\t$t8\t" << na << endl;
            optfile << "\t\tsw\t"<< regIdToString(front) <<"\t($t8)" << endl;
        }
        else {
            if (!convar[-addr / 4].stack_valid) {
                optfile << "\t\tsw\t"<<regIdToString(front)<<"\t" << addr << "($fp)" << endl;
                convar[-addr / 4].stack_valid = true;
            }
            deleteRegFromLocal(front);
        }
    }
    load(regIdToString(front),varname);
    return regIdToString(front);
}

void saveRegToMem() {
    for (int i = 0;i < 8;i++) {
        for (set<std::string>::iterator iter = regDes[index[i]].varNameSet.begin();iter != regDes[index[i]].varNameSet.end();iter++) {
            string na = *iter;
            char nam[32];
            strcpy(nam,na.c_str());
            int addr = -1 * findAddrInStack(nam);
            if (addr == 1) {
                optfile << "\t\tla\t$t8\t" << na << endl;
                optfile << "\t\tsw\t" << regIdToString(index[i]) << "\t($t8)" << endl;
            }
            else {
                if (!convar[-addr / 4].stack_valid) {
                    optfile << "\t\tsw\t"<<regIdToString(index[i])<<"\t" << addr << "($fp)" << endl;
                    convar[-addr / 4].stack_valid = true;
                }
            }
        }
    }
}
void clearReg(int id) {
    deleteRegFromLocal(id); 
    regDes[id].varNameSet.clear();
    regDes[id].free = true;
}
void saveRegToMemAndClear() {
    for (int i = 0;i < 8;i++) {
        for (set<std::string>::iterator iter = regDes[index[i]].varNameSet.begin();iter != regDes[index[i]].varNameSet.end();iter++) {
            string na = *iter;
            char nam[32];
            strcpy(nam,na.c_str());
            int addr = -1 * findAddrInStack(nam);
            if (addr == 1) {
                optfile << "\t\tla\t$t8\t" << na << endl;
                optfile << "\t\tsw\t" << regIdToString(index[i]) << "\t($t8)" << endl;
            }
            else {
                
                if (!convar[-addr / 4].stack_valid) {
                    optfile << "\t\tsw\t"<<regIdToString(index[i])<<"\t" << addr << "($fp)" << endl;
                    convar[-addr / 4].stack_valid = true;
                }
            }
        }
        clearReg(index[i]);
    }
}

char * localAddressToName(int address) {
    return convar[-address / 4].name;
}

void load(string regName,char * varname) {
    int address = findAddrInStack(varname);
    int regId = regStringToid(regName);
    clearReg(regId);
    optfile <<"\t\t"<<"lw\t" << regName << address <<  "($fp)" << endl;
    regDes[regId].free = false;
    regDes[regId].varNameSet.insert(varname);
    convar[address / 4].reg_valid = true;
    convar[address / 4].reg_id = regId;
}

void save(string regName,char * varname) {
    int address = findAddrInStack(varname);
    convar[address / 4].stack_valid = true;
}

void setStackUnvalid(char * name) {
    int address = findAddrInStack(name);
    convar[address / 4].stack_valid = false;
}

bool isFirst(int i) {
    if (strcmp(mid_code[i].op,lab) == 0 || strcmp(mid_code[i].op,func) == 0  || i == 0) {
        return true;
    }
    else if (i > 0 && (strcmp(mid_code[i-1].op,jmp) == 0 || strcmp(mid_code[i-1].op,jne) == 0 
            || strcmp(mid_code[i-1].op,call) == 0 || strcmp(mid_code[i-1].op,ret) == 0)) {
        return true;
    }
    return false;
}

void toBlock() {
    int i = 0;
    while (i < codeNum) {
        //cout << i << endl;
        if (isFirst(i)) {
            vector <FOURVARCODE> Block; 
            Block.push_back(mid_code[i]);
            int j;
            for (j = i + 1;j < codeNum;j++) {
                if (!isFirst(j)) {
                    Block.push_back(mid_code[j]);
                }
                else {
                    blocks.push_back(Block);
                    i = j;
                    break;
                }
            }
            if (j == codeNum) {
                i = j;
                blocks.push_back(Block);
            }
        }
        else {
            error(DAG_ASSIGN_ERROR,0);
        }   
    }
}

void displayBlock() {
    for (int i = 0;i < blocks.size();i++) {
        cout << "Block:" << i << endl;
        for (int j = 0;j < blocks[i].size();j++) {
            if (strcmp(blocks[i][j].op,lab) == 0) {
                cout << "\t\t" << blocks[i][j].rst << " :" << endl ;
            }
            else {
                cout << "\t\t" << blocks[i][j].op << ", " << blocks[i][j].num_a << ", " << blocks[i][j].num_b << ", " << blocks[i][j].rst << " " << endl ;
            }
        }
        cout << endl << endl;
    }
}




void globalcondef() {
    int i = 0;
    while (strcmp(mid_code[i].op,consts) == 0) {
        strcpy(glbconst[cntOfGlobal].name,mid_code[i].rst);
        strcpy(glbconst[cntOfGlobal].value,mid_code[i].num_b);
        optfile << mid_code[i].rst << ":\t.word\t" << mid_code[i].num_b << endl ;
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
            optfile << mid_code[i].rst << ":\t.space\t" << step * 4 << endl;
            cntOfGlobal++;
            i++; 
        }
        else {
            strcpy(glbconst[cntOfGlobal].name,mid_code[i].rst);
            strcpy(glbconst[cntOfGlobal].value,mid_code[i].num_b);
            optfile << mid_code[i].rst << ":\t.space\t" << "4" << endl ;
            cntOfGlobal++ ;
            i++;
        }
    }
    donenum = cntOfGlobal;
}

void initasm() {
    int i = 0;
    
    optfile << ".data" << endl;
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
                optfile << temp << ":\t.asciiz\t" << "\"" << glbconst[cntOfGlobal].value << "\"" << endl;
                cntOfGlobal++;
                stringCnt++;
            }
        }
        i++;
    }
    optfile << ".text" << endl;
    optfile << ".globl main" << endl;
    optfile << "\t\tmove\t$fp\t$sp" << endl;
    optfile << "\t\tj\tmain" << endl;
}


int isNum(char a) {
    if (a >= '0' && a <= '9') {
        return 1;
    }
    return 0;
}

void func_mips() {
    optfile << endl << endl;
    paranum = 0;
    //rstfile << "\t\t# func fourvarcode" << endl;
    //rstfile << "\t\t# Save Register" <<endl;
    optfile << mid_code[donenum].rst << ":" << endl;

    optfile << "\t\tsubi\t$sp\t$sp\t4" << endl; // $sp -= 4
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
            optfile << "\t\tli\t$v1\t" << mid_code[donenum].rst << endl; // $v0 $v1放返回值
        }
        else {
            int addr1 = -1 * findaddr(mid_code[donenum].rst);
            if (addr1 == 1) { // global
                optfile << "\t\tla\t$t8\t" << mid_code[donenum].rst << endl;
                optfile << "\t\tlw\t$v1\t($t8)" << endl;
            }
            else {
                optfile << "\t\tlw\t$v1\t" << addr1 << "($fp)" << endl;
            }
        }
    }
    optfile << "\t\tmove\t$t8\t$ra" << endl; // save $ra
    optfile << "\t\tlw\t$ra\t-4($fp)" << endl;  // rstore pre $ra
    optfile << "\t\tadd\t$sp\t$fp\t$zero" << endl; // sp = fp
    optfile << "\t\tlw\t$fp\t($fp)" << endl; // fp = pre fp
    optfile << "\t\tjr\t$t8" << endl;
}

void ints_mips() {
    // local num
    strcpy(convar[localnum].name,mid_code[donenum].rst);
    convar[localnum].addr = offset;
    convar[localnum].reg_valid = false;
    convar[localnum].stack_valid = false;
    optfile << "\t\tsubi\t$sp\t$sp\t4" << endl;

    localnum ++;
    offset += 4;
}

void chars_mips() {
    strcpy(convar[localnum].name,mid_code[donenum].rst);
    convar[localnum].addr = offset;
    convar[localnum].reg_valid = false;
    convar[localnum].stack_valid = false;
    optfile << "\t\tsubi\t$sp\t$sp\t4" << endl;
    localnum++;
    offset += 4;
}

void inta_mips() {
    strcpy(convar[localnum].name,mid_code[donenum].rst);
    convar[localnum].addr = offset;
    convar[localnum].reg_valid = false;
    convar[localnum].stack_valid = false;
    int step;
    step = atoi(mid_code[donenum].num_b);
    optfile << "\t\tsubi\t$sp\t$sp\t" << step * 4 << endl;
    offset += step * 4;
    localnum ++;
}

void chara_mips() {
    strcpy(convar[localnum].name,mid_code[donenum].rst);
    convar[localnum].addr = offset;
    convar[localnum].reg_valid = false;
    convar[localnum].stack_valid = false;
    int step;
    step = atoi(mid_code[donenum].num_b);
    optfile << "\t\tsubi\t$sp\t$sp\t" << step * 4 << endl;
    offset += step * 4;
    localnum ++;
}


void add_mips(FOURVARCODE fourCode) {
    int addr1;
    int addr2;
    int addr3;
    // ADDI rt, rs, immediate
    if (fourCode.rst[0] == '$') {
        insertaddr(fourCode.rst);
    }
    if (isNum(fourCode.num_a[0]) || fourCode.num_a[0] == '-') {
         // rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;

        string reg_b = getReg(fourCode.num_b);
        
        string reg_c = getReg(fourCode.rst);
        optfile << "\t\taddi\t" << reg_c << "\t" << reg_b << "\t" << fourCode.num_a << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
    else if (isNum(fourCode.num_b[0]) || fourCode.num_b[0] == '-') {
        string reg_a = getReg(fourCode.num_a);
        
        string reg_c = getReg(fourCode.rst);
        optfile << "\t\taddi\t" << reg_c << "\t" << reg_a << "\t" << fourCode.num_b << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
    else {
        string reg_a = getReg(fourCode.num_a);
        string reg_b = getReg(fourCode.num_b);
        string reg_c = getReg(fourCode.rst);
        optfile << "\t\tadd\t" << reg_c << "\t" << reg_a << "\t" << reg_b << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
}

void sub_mips(FOURVARCODE fourCode) {
    int addr1;
    int addr2;
    int addr3;
    // ADDI rt, rs, immediate
    if (fourCode.rst[0] == '$') {
        insertaddr(fourCode.rst);
    }
    if (isNum(fourCode.num_a[0]) || fourCode.num_a[0] == '-') {
         // rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;

        string reg_b = getReg(fourCode.num_b);
        
        string reg_c = getReg(fourCode.rst);
        optfile << "\t\tsubi\t" << reg_c << "\t" << fourCode.num_a<< "\t" << reg_b << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
    else if (isNum(fourCode.num_b[0]) || fourCode.num_b[0] == '-') {
        string reg_a = getReg(fourCode.num_a);
        
        string reg_c = getReg(fourCode.rst);
        optfile << "\t\tsubi\t" << reg_c << "\t" << reg_a << "\t" << fourCode.num_b << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
    else {
        string reg_a = getReg(fourCode.num_a);
        string reg_b = getReg(fourCode.num_b);
        string reg_c = getReg(fourCode.rst);
        optfile << "\t\tsub\t" << reg_c << "\t" << reg_a << "\t" << reg_b << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
}


void mul_mips(FOURVARCODE fourCode) {
    int addr1;
    int addr2;
    int addr3;
    // ADDI rt, rs, immediate
    if (fourCode.rst[0] == '$') {
        insertaddr(fourCode.rst);
    }
    if (isNum(fourCode.num_a[0]) || fourCode.num_a[0] == '-') {
         // rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
        optfile << "\t\tli\t$t8\t" << fourCode.num_a << endl;
        string reg_b = getReg(fourCode.num_b);

        string reg_c = getReg(fourCode.rst);
        optfile << "\t\tmult\t"<< reg_b <<"\t$t8" << endl;
        optfile << "\t\tmflo\t" << reg_c << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
    else if (isNum(fourCode.num_b[0]) || fourCode.num_b[0] == '-') {
        optfile << "\t\tli\t$t8\t" << fourCode.num_b << endl;
        string reg_a = getReg(fourCode.num_a);
        
        string reg_c = getReg(fourCode.rst);
        optfile << "\t\tmult\t"<< reg_a <<"\t$t8" << endl;
        optfile << "\t\tmflo\t" << reg_c << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
    else {
        string reg_a = getReg(fourCode.num_a);
        string reg_b = getReg(fourCode.num_b);
        string reg_c = getReg(fourCode.rst);

        optfile << "\t\tmult\t"<< reg_a <<"\t" << reg_b << endl;
        optfile << "\t\tmflo\t" << reg_c << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
}

void div_mips(FOURVARCODE fourCode) {
    int addr1;
    int addr2;
    int addr3;
    // ADDI rt, rs, immediate
    if (fourCode.rst[0] == '$') {
        insertaddr(fourCode.rst);
    }
    if (isNum(fourCode.num_a[0]) || fourCode.num_a[0] == '-') {
         // rstfile << "\t\tli\t$t0\t" << mid_code[donenum].num_a << endl;
        optfile << "\t\tli\t$t8\t" << fourCode.num_a << endl;
        string reg_b = getReg(fourCode.num_b);

        string reg_c = getReg(fourCode.rst);
        optfile << "\t\tdiv\t" << "\t$t8" << reg_b << endl;
        optfile << "\t\tmflo\t" << reg_c << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
    else if (isNum(fourCode.num_b[0]) || fourCode.num_b[0] == '-') {
        optfile << "\t\tli\t$t8\t" << fourCode.num_b << endl;
        string reg_a = getReg(fourCode.num_a);
        
        string reg_c = getReg(fourCode.rst);
        optfile << "\t\tdiv\t"<< reg_a <<"\t$t8" << endl;
        optfile << "\t\tmflo\t" << reg_c << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
    else {
        string reg_a = getReg(fourCode.num_a);
        string reg_b = getReg(fourCode.num_b);
        string reg_c = getReg(fourCode.rst);

        optfile << "\t\tdiv\t"<< reg_a <<"\t" << reg_b << endl;
        optfile << "\t\tmflo\t" << reg_c << endl;
        setStackUnvalid(fourCode.rst);
        return;
    }
}

void prtf_mips(FOURVARCODE fourCode) {
    if (strcmp(fourCode.num_a,space) != 0) {
        optfile << "\t\tla\t$t8\t" << fourCode.num_a << endl;
        optfile << "\t\tmove\t$a0\t$t8" << endl;  // 字符串地址给$a0
        optfile << "\t\tli\t$v0\t4" << endl; // 设置操作模式 4 打印字符串
        optfile << "\t\tsyscall" << endl;
        optfile << "\t\tli\t$a0\t\'\\n\'" <<endl;
        optfile << "\t\tli\t$v0\t11" << endl;  // ?
        optfile << "\t\tsyscall" << endl;
    }
    if (strcmp(fourCode.num_b,space) == 0) {
        return;
    }
    int addr1 = -1 * findAddrInStack(fourCode.num_b);
    if (addr1 == 1) { // global
        if (isNum(fourCode.num_b[0]) || fourCode.num_b[0] == '-') {
            optfile << "\t\tli\t$a0\t" << fourCode.num_b << endl;
        }
        else {
            optfile << "\t\tla\t$t0\t" << fourCode.num_b << endl;
            optfile << "\t\tlw\t$a0\t($t0)" << endl;
        }
    }
    else {
        optfile << "\t\tlw\t$a0\t" << addr1 << "($fp)" << endl;
    }
    if (strcmp(fourCode.rst,ints) == 0) {
        optfile << "\t\tli\t$v0\t1" << endl;
    }
    else if (strcmp(fourCode.rst,chars) == 0) {
        optfile << "\t\tli\t$v0\t11" << endl;
    }
    optfile << "\t\tsyscall" << endl;
    optfile << "\t\tli\t$a0\t\'\\n\'" <<endl;
    optfile << "\t\tli\t$v0\t11" << endl;  // ?
    optfile << "\t\tsyscall" << endl;
}


void scf_mips(FOURVARCODE fourCode) {
    int addr1 = -1 * findAddrInStack(fourCode.rst);
    if (strcmp(fourCode.num_b,ints) == 0) {
        optfile << "\t\tli\t$v0\t5" << endl;
    }
    else if (strcmp (fourCode.num_b,chars) == 0) {
        optfile << "\t\tli\t$v0\t12" << endl;
    }
    optfile << "\t\tsyscall" << endl;
    if (addr1 == 1) { // global
        optfile << "\t\tla\t$t8\t" << fourCode.rst << endl;
        optfile << "\t\tsw\t$v0\t($t8)" << endl;
    }
    else {
        optfile << "\t\tsw\t$v0\t" << addr1 << "($fp)" << endl;
        convar[-addr1 / 4].stack_valid = true;
    }
}

void lab_mips(FOURVARCODE fourCode) {
    optfile << fourCode.rst << ":" <<endl;
}

void paraop_mips(FOURVARCODE fourCode) {
    // $a0 ~ $a3用于存储程序的前四个参数
    insertaddr(fourCode.rst);
    if (paranum < 4) {
        switch (paranum) {
        case 0:
            optfile << "\t\tmove\t$t0\t$a0" << endl;
            optfile << "\t\tsw\t$t0\t" << -8 << "($fp)" << endl;
            convar[2].stack_valid = true;
            break;
        case 1 :
            optfile << "\t\tmove\t$t0\t$a1" << endl;
            optfile << "\t\tsw\t$t0\t" << -12 << "($fp)" << endl;
            convar[3].stack_valid = true;
            break;
        case 2:
            optfile << "\t\tmove\t$t0\t$a2" << endl;
            optfile << "\t\tsw\t$t0\t" << -16 << "($fp)" << endl;
            convar[4].stack_valid = true;
            break;
        case 3:
            optfile << "\t\tmove\t$t0\t$a3" << endl;
            optfile << "\t\tsw\t$t0\t" << -20 << "($fp)" << endl;
            convar[5].stack_valid = true;
            break;
        default:
            break;
        }
    }
    paranum++;
}

void calpara_mips(FOURVARCODE fourCode) {
    int addr1 = -1 * findAddrInStack(fourCode.rst); // -1 表示地址从高地址向地址生长
    if (calparanum < 4) {
        switch (calparanum) {
        case 0:
            if (addr1 == 1) { // not find,is global
                if (isNum(fourCode.rst[0]) || fourCode.rst[0] == '-') {
                    optfile << "\t\tli\t$a0\t" << fourCode.rst << endl; 
                }
                else {
                    optfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                    optfile << "\t\tlw\t$a0\t($t0)" << endl;
                }
            }
            else {
                optfile << "\t\tlw\t$a0\t" << addr1 << "($fp)" << endl;
            }
            break;
        case 1:
            if (addr1 == 1) { // not find,is global
                if (isNum(mid_code[donenum].rst[0]) || mid_code[donenum].rst[0] == '-') {
                    optfile << "\t\tli\t$a1\t" << mid_code[donenum].rst << endl; 
                }
                else {
                    optfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                    optfile << "\t\tlw\t$a1\t($t0)" << endl;
                }
            }
            else {
                optfile << "\t\tlw\t$a1\t" << addr1 << "($fp)" << endl;
            }
            break;
        case 2:
            if (addr1 == 1) { // not find,is global
                if (isNum(mid_code[donenum].rst[0]) || mid_code[donenum].rst[0] == '-') {
                    optfile << "\t\tli\t$a2\t" << mid_code[donenum].rst << endl; 
                }
                else {
                    optfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                    optfile << "\t\tlw\t$a2\t($t0)" << endl;
                }
            }
            else {
                optfile << "\t\tlw\t$a2\t" << addr1 << "($fp)" << endl;
            }
            break;
        case 3:
            if (addr1 == 1) { // not find,is global
                if (isNum(mid_code[donenum].rst[0]) || mid_code[donenum].rst[0] == '-') {
                    optfile << "\t\tli\t$a3\t" << mid_code[donenum].rst << endl; 
                }
                else {
                    optfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                    optfile << "\t\tlw\t$a3\t($t0)" << endl;
                }
            }
            else {
                optfile << "\t\tlw\t$a3\t" << addr1 << "($fp)" << endl;
            }
            break;
        }
    }
    else { // not only 4 paraments
        if (addr1 == 1) { // global
            if (isNum(mid_code[donenum].rst[0]) || mid_code[donenum].rst[0] == '-') {
                optfile << "\t\tli\t$t8\t" << mid_code[donenum].rst << endl;
            }
            else {
                optfile << "\t\tla\t$t0\t" << mid_code[donenum].rst << endl;
                optfile << "\t\tlw\t$t8\t($t0)" << endl;
            }
        }
        else {
            optfile << "\t\tlw\t$t8\t" << addr1 << "($fp)" << endl;
        }
        optfile << "\t\tsw\t$t8\t" << -1 * ((calparanum - 4)*4 + 24) << "($sp)" << endl ;
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