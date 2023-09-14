#include <iostream>
#include <algorithm>
#define TOP_NUM 100

void adjustDown(int* arr, int top_size, int parent) {
	int child = parent * 2 + 1;
	while (child < top_size)//当child大于了数组大小就跳出循环
	{
		//找出左右孩子中小/大的那个（假设法）
		if (child + 1 < top_size && arr[child + 1] < arr[child])
		{
			child++;
		}
 
		if (arr[child] < arr[parent])
		{
			std::swap(arr[parent], arr[child]);
 
			parent = child;
			child = parent * 2 + 1;
		}
		else
		{
			break;
		}
	}
}

void dump(int* arr) {
    for (int i = 0; i < TOP_NUM; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
}
int main() {
    // 生成一些测试数据
    //  CreateNData();

    const char* file = "data.txt";
    FILE* fout = fopen(file, "r");
    if (NULL == fout) {
        perror("fopen error:");
        return -1;
    }

    int* heap = new int[TOP_NUM];
    for (int i = 0; i < TOP_NUM; ++i) {
        fscanf(fout, "%d", &heap[i]);
    }

    // 调整最小堆
    for (int i = (TOP_NUM - 2) / 2; i >= 0; --i) {
        adjustDown(heap, TOP_NUM, i);
    }

    int val = 0;
    while (!feof(fout)) {
        fscanf(fout, "%d", &val);
        if (val > heap[0]) {
            heap[0] = val;
            adjustDown(heap, TOP_NUM, 0);
        }
    }

    // 调整最小堆	
    for (int i = TOP_NUM - 1; i > 0; --i) {
		std::swap(heap[0], heap[i]);
        adjustDown(heap, i, 0);
    }

    dump(heap);

    delete[] heap;
    return 0;
}