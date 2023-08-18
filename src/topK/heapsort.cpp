#include <algorithm>
#include <iostream>
#include <vector>

void adjust(int* arr, int top_size, int parent) {
    int child = parent * 2 + 1;
    while (child < top_size)
    {
        if(child + 1 < top_size && arr[child + 1] < arr[child]){
            ++child;
        }
        
        if(arr[child] < arr[parent]){
            std::swap(arr[child], arr[parent]);

            parent = child;
            child = parent * 2 + 1;
        }else{
            break;
        }
    }
}

void dump(int* arr, int size) {
    for (int i = 0; i < size; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
}

int main() {
    int arr[] = { 10, 20, 5, 6, 8, 22, 15, 66, 42, 23 };
    int size = sizeof(arr) / sizeof(arr[0]);
    
    //建堆 -- 向下调整，时间复杂度：O(N)
	//倒着调整
	//叶子节点不需要处理
	//倒数第一个非叶子节点：最后一个节点的父亲开始调整
    for (int i = (size - 1) / 2; i >= 0; --i) {
        adjust(arr, size, i);
    }

    dump(arr, size);

    //O(N*log(N))
    for (int i = size - 1; i > 0; --i) {
		std::swap(arr[0], arr[i]);
        adjust(arr, i, 0);
    }

    dump(arr, size);
    return 0;
}