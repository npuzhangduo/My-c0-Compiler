#include <iostream>
using namespace std;

int main(){
	char t = 1 + '0';
	string a = "$t";
	a.append(1,t);
	cout << a << endl;
	return 0;
}
