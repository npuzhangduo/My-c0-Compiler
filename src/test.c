const int tmp = 200;

int a;
char cstr;
int arr[100];

int duo(int a,int b) {
    int zhang;
    zhang = 2;
    return (a + b + zhang);
}
void zh() {
    const int a = 3;
    int l;
    l = 100;
    printf(a+l);
}

void main()
{
    int res;
    int a;
    if (tmp > 100) {
        a = 20;
        res = duo(tmp,a);
    }
    else {
        a = 100;
        res = duo(tmp,a);
    }
    
    printf (res);
    return;
}
