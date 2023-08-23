# string堆栈分配长度测试结果

## centos7验证
gcc 7.5
len <= 15 在栈上分配

gcc 11.2
len = 0 才会在栈上分配
