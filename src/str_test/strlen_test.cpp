#include<string>
#include <iostream>

using namespace std;
void* operator new(std::size_t count) {
    cout << "分配堆内存" << count << "字节" << endl;
    return malloc(count);
}
void operator delete(void* p) {
    cout << "释放堆内存" << p << endl;
    free(p);
}
void show_str(const string& str) {
    cout << endl;
    cout << __func__ << "() 临时变量初始化" << endl;
    string tmp = str;
    printf("str的副本地址: %p\n", str.c_str());
    printf("tmp的副本地址: %p\n", tmp.c_str());
}

int main() {
    // cout << endl;
    // cout << "-------初始化string对象-------" << endl;
    // string you = "Hello World!";
    // cout << endl;
    // cout << "-------show_str-------" << endl;
    // printf("main函数: you副本中的字符串地址: %p\n", you.c_str());
    // show_str(you);
    // cout << endl;

    cout << "字符串字面量直接传参方式" << endl;
    show_str("Hello World!");
    cout << endl;
    cout << "字符数组传参方式" << endl;
    const char mes[] = "Hello World!";
    printf("main函数: mes副本中的字符串地址: %p\n", mes);
    show_str(mes);
    cout << endl;
    return 0;
} /*Hello World!Hello World!Hello World!*/