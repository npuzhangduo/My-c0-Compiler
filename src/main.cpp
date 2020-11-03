
// #include <grammer.cpp>

#include <asm.cpp>
//#include <optasm.cpp>
int main(int argc, char const *argv[])
{
    fin = fopen(argv[1],"rb");
    
    _gramma.open("../src/grammer.txt",ios::out);
    outputfile.open("../src/midcode.txt",ios::out);
    afteroutputfile.open("../src/aftermidcode.txt",ios::out);
    rstfile.open("../src/asmcode.asm",ios::out);

    init();
    getch();
    getsym();
    program();
    cout << "已经生成四元式,展示符号表" << endl;
    stt();
    cout  <<"常数合并优化" <<endl;
    combine();
    _gramma.close();
    outputfile.close();
    afteroutputfile.close();

    run();
    // cout << "Here:" << endl; 
    // // toBlock();
    // displayBlock();
    
    rstfile.close();
    fclose(fin);

    return 0;
}

// 词法分析测试
// int main(int argc, char const *argv[])
// {
//     fin = fopen(argv[1],"rb");
    
//     cout << "词法分析开始:" << endl;
//     getch();
//     do {
//         getsym();
//     } while (symId != TK_EOF);
//     fclose(fin);
//     cout << "词法分析结束" << endl;
//     return 0;
// }
