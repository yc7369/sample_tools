#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "zlog/ztp_log.h"

int main() {
    ztp::SlogInit("./logs", "zerocopy", 0);    

    // 打开输入文件（源文件）
    int inputFd = open("input.txt", O_RDONLY);
    if (inputFd == -1) {
        ZLOG_ERROR("open");
        return 1;
    }

    // 获取输入文件的大小
    struct stat statBuf;
    if (fstat(inputFd, &statBuf) == -1) {
        ZLOG_ERROR("fstat");
        close(inputFd);
        return 1;
    }

    // 打开输出文件（目标文件）
    int outputFd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outputFd == -1) {
        ZLOG_ERROR("open");
        close(inputFd);
        return 1;
    }

    // 使用 sendfile 进行零拷贝传输
    off_t offset = 0;
    ssize_t sentBytes = sendfile(outputFd, inputFd, &offset, statBuf.st_size);

    if (sentBytes == -1) {
        ZLOG_ERROR("sendfile");
        close(inputFd);
        close(outputFd);
        return 1;
    }

    // 关闭文件描述符
    close(inputFd);
    close(outputFd);

    return 0;
}
