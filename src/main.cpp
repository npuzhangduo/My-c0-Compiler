
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
    cout << "�Ѿ�������Ԫʽ,չʾ���ű�" << endl;
    stt();
    cout  <<"�����ϲ��Ż�" <<endl;
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

// �ʷ���������
// int main(int argc, char const *argv[])
// {
//     fin = fopen(argv[1],"rb");
    
//     cout << "�ʷ�������ʼ:" << endl;
//     getch();
//     do {
//         getsym();
//     } while (symId != TK_EOF);
//     fclose(fin);
//     cout << "�ʷ���������" << endl;
//     return 0;
// }
