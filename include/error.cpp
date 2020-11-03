#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

/*        error              */

using namespace std ;

#define FILEOPEN_ERROR -1 //file open failed
enum error_code {
    ZEROSTART_ERROR,
    UNDEFFUNC_ERROR,
    UNDEFCHAR_ERROR,
    UNMATCHSQ_ERROR,
    UNMATCHDQ_ERROR,
    OUTOFTABINDEXX_ERROR,
    FUNCNAMECOMPLICT_ERROR,
    VARNAMECOMPLICT_ERROR,
    NAMECOMPLICTGLOBAL_ERROR,
    DELHEAD_IDMIS_ERROR,
    AFTERASS_NOTIDEN_ERROR,
    CONSTDEF_ASSMIS_ERROR,
    CONSTDEF_TYPE_ERROR,
    SEMICSYMMIS_ERROR,
    VARDEF_ARRAYINDEX_ERROR,
    VARDEF_TYPE_ERROR,
    MAINSYM_ERROR,
    LPARENSYMMIS_ERROR,
    RPARENSYMMIS_ERROR,
    LBPARENSYMMIS_ERROR,
    RBPARENSYMMIS_ERROR,
    RMPARENSYMMIS_ERROR,
    AFTEROP_NUMMIS_ERROR,
    FACTOR_ERROR,
    ASSIGNSTATUS_ERROR,
    STATUS_ERROR,
    CONDITIONOP_ERROR,
    WHILESYMMIS_ERROR,
    IDSYMMIS_ERROR,
    FORASSIGNMIS_ERROR,
    VARTYPE_ERROR,
    FOROPMIS_ERROR,
    FORDIGSYM_ERROR,
    VOIDSYMMIS_ERROR,
    PARANUM_ERROR,
    UNEXCEPTRETURNVAL_ERROR,
    READARRAY_ERROR,
    NOT_ERROR,
    ESCAPE_CHAR_ERROR,

    LACK_ERROR,
    UNDEFINED_TYPE_ERROR,
    DAG_ASSIGN_ERROR
};
// #define ZEROSTART_ERROR 0 //
// #define UNDEFFUNC_ERROR 1
// #define UNDEFCHAR_ERROR 2 //undefined IDEN
// #define UNMATCHSQ_ERROR 3 //unmatched singal quoto
// #define UNMATCHDQ_ERROR 4 // unmatched double quoto
// #define OUTOFTABINDEXX_ERROR 5 // out of table max length
// #define FUNCNAMECOMPLICT_ERROR 6 // function name conplict
// #define VARNAMECOMPLICT_ERROR 7 //variable name complict
// #define NAMECOMPLICTGLOBAL_ERROR 8 //name can not complict with global
// #define DELHEAD_IDMIS_ERROR 9 // id missed in delcare head part
// #define AFTERASS_NOTIDEN_ERROR 10 // after a assign symbol should be a dientifier symbol
// #define CONSTDEF_ASSMIS_ERROR 11 // in a const defination assign symbol missed
// #define CONSTDEF_TYPE_ERROR 12 // const defination have only two type
// #define SEMICSYMMIS_ERROR 13 //  ; missed
// #define VARDEF_ARRAYINDEX_ERROR 14 //an array index should be number in variable defination
// #define VARDEF_TYPE_ERROR 15 // in a vardefine, identifier type error
// #define MAINSYM_ERROR 16 // in a main function, name must be "
// #define LPARENSYMMIS_ERROR 17 // main ( missed
// #define RPARENSYMMIS_ERROR 18 // main ) missed
// #define LBPARENSYMMIS_ERROR 19 // mian {
// #define RBPARENSYMMIS_ERROR 20 // main }
// #define RMPARENSYMMIS_ERROR 21 // ] missed
// #define AFTEROP_NUMMIS_ERROR 22 // -56 ok, -b not accepted
// #define FACTOR_ERROR 23 // factor type error
// #define ASSIGNSTATUS_ERROR 24 // assign status error
// #define STATUS_ERROR 25 // status error
// #define CONDITIONOP_ERROR 26 // condition status error
// #define WHILESYMMIS_ERROR 27 // while symbol missed
// #define IDSYMMIS_ERROR 28 // in a for loop, identifier missed in initial
// #define FORASSIGNMIS_ERROR 29 // int a for loop initial, assign symbol must appeare
// #define VARTYPE_ERROR 30 // in a for loop, loop variable should not be a const
// #define FOROPMIS_ERROR 31 // in for loop step part, op(+/-) missed
// #define FORDIGSYM_ERROR 32 // in for loop step part, digital missed
// #define VOIDSYMMIS_ERROR 33 // in main function, void symbol missed
// #define PARANUM_ERROR 34 // paramenter number error
// #define UNEXCEPTRETURNVAL_ERROR 35 // void function return value
// #define READARRAY_ERROR 36 // read an array name

void error(int errorcode, int indexInFile);
char WARNING[] = {'W','a','r','n','i','n','g',':',' ','\0'} ;
char ERROR[] =  {'E','r','r','o','r',':',' ','\0'} ;
char LINE[] = {'i','n',' ','l','i','n','e',' ','\0'} ;



void error(int errorcode, int indexInFile){
    switch(errorcode){
        case -1 :
            cout << "file open failed!" << endl ;
            exit(0) ;
            break ;
        case ZEROSTART_ERROR ://warning
            cout << LINE << indexInFile << "\t" <<  WARNING << "a num should not start with 0 " << endl ;
            break ;
        case UNDEFFUNC_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "undefined function name " << endl ;
            break ;
        case UNDEFCHAR_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "undefined identifier name " << endl ;
            break ;
        case UNMATCHSQ_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "unmatched singal quotes " << endl ;
            break ;
        case UNMATCHDQ_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "unmatched double quotes " << endl ;
            break ;
        case OUTOFTABINDEXX_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "to much defination " << endl ;
            exit(0) ;
            break ;
        case FUNCNAMECOMPLICT_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "function name complicted " << endl ;
            break ;
        case VARNAMECOMPLICT_ERROR  :
            cout << LINE << indexInFile << "\t" << ERROR << "variable name complicted " << endl ;
            break ;
        case NAMECOMPLICTGLOBAL_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "variable name complicted with global " << endl ;
            break ;
        case DELHEAD_IDMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "function name missed " << endl ;
            break ;
        case AFTERASS_NOTIDEN_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "const defination without value " << endl ;
            break ;
        case CONSTDEF_ASSMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "const defination without assign symbol " << endl ;
            break ;
        case CONSTDEF_TYPE_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "undefined variable type in const defination" << endl ;
            break ;
        case SEMICSYMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "\';\'\tmissed " << endl ;
            break ;
        case VARDEF_ARRAYINDEX_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "unaccepted array index in array defination " << endl ;
            break ;
        case VARDEF_TYPE_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "undefined type in variable defination " << endl ;
            break ;
        case MAINSYM_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "false function name in main " << endl ;
            break ;
        case LPARENSYMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "\'(\'\tmissed in main function " << endl ;
            break ;
        case RPARENSYMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "\')\'\tmissed in main function " << endl ;
            break ;
        case LBPARENSYMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "\'{\'\tmissed in main function  " << endl ;
            break ;
        case RBPARENSYMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "\'}\'\tmissed in main function  " << endl ;
            break ;
        case RMPARENSYMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "\']\'\tmissed  " << endl ;
            break ;
        case AFTEROP_NUMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "\'-\'' can only appeared before a number " << endl ;
            break ;
        case FACTOR_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "unidentified factor " << endl ;
            break ;
        case ASSIGNSTATUS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "unidentified identifier in assign status " << endl ;
            break ;
        case STATUS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "unidentified states " << endl ;
            break ;
        case CONDITIONOP_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "unidentified condition symbol " << endl ;
            break ;
        case WHILESYMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "\'while\' symbol missed " << endl ;
            break ;
        case IDSYMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "identifier lack in for loop " << endl ;
            break ;
        case FORASSIGNMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "assign symbol lack in for loop " << endl ;
            break ;
        case VARTYPE_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "loop variable should not be const in for loop " << endl ;
            break ;
        case FOROPMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "+/- missed in loop step part " << endl ;
            break ;
        case FORDIGSYM_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "step digital missed in for loop " << endl ;
            break ;
        case VOIDSYMMIS_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "\'void\'\tmissed in main function " << endl ;
            break ;
        case PARANUM_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "paramenter number unmatched " << endl ;
            break ;
        case UNEXCEPTRETURNVAL_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "void function return a value " << endl ;
            break ;
        case READARRAY_ERROR :
            cout << LINE << indexInFile << "\t" << ERROR << "read a array name !" << endl ;
            break ;
        case NOT_ERROR:
            cout << LINE << indexInFile << "\t" << ERROR << "! not defined" << endl ;
            break ;
        case ESCAPE_CHAR_ERROR:
            cout << LINE << indexInFile << "\t" << ERROR << "undefined escape char!" << endl ;
            break ;
        case UNDEFINED_TYPE_ERROR:
            cout << LINE << indexInFile << "\t" << ERROR << "undefined Type error !" << endl ;
            break ;
        case DAG_ASSIGN_ERROR:
            cout << ERROR << "DAG divide error !" << endl ;
            break ;
    }
}