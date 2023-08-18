#include <iostream>
using namespace std;

void CreateNData() {
    // 造数据
    int n = 1000;
    srand((unsigned int)time(NULL));
    const char* file = "data.txt";
    FILE* fin = fopen(file, "w");
    if (NULL == fin) {
        perror("fopen error:");
        return;
    }

    for (int i = 0; i < n; i++) {
        int x = rand() % 100000;
        fprintf(fin, "%d\n", x);
    }

    fclose(fin);
}

int main() {
    // 生成一些测试数据
    CreateNData();
    return 0;
}