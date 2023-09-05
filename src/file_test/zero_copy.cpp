#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "zlog/ztp_log.h"

//这里的log是新增的一套log，原有一套zlog
#define BASE_DATA_LOGGER_NAME "zerocopy"
#define BSLOG_TRACE(...) _ZLOG_TRACE(BASE_DATA_LOGGER_NAME, __VA_ARGS__)
#define BSLOG_DEBUG(...) _ZLOG_DEBUG(BASE_DATA_LOGGER_NAME, __VA_ARGS__)
#define BSLOG_INFO(...) _ZLOG_INFO(BASE_DATA_LOGGER_NAME, __VA_ARGS__)
#define BSLOG_WARN(...) _ZLOG_WARN(BASE_DATA_LOGGER_NAME, __VA_ARGS__)
#define BSLOG_ERROR(...) _ZLOG_ERROR(BASE_DATA_LOGGER_NAME, __VA_ARGS__)
#define BSLOG_CRITICAL(...) _ZLOG_CRITICAL(BASE_DATA_LOGGER_NAME, __VA_ARGS__)

int main() {
    ztp::setup_log_system("./logs", BASE_DATA_LOGGER_NAME, 0, BASE_DATA_LOGGER_NAME);
    ztp::setup_log_system("./logs", BASE_LOGGER_NAME, 0, BASE_LOGGER_NAME);

    ZLOG_TRACE("start");
    BSLOG_TRACE("start");
    // 打开输入文件（源文件）
    int inputFd = open("input.txt", O_RDONLY);
    if (inputFd == -1) {
        BSLOG_TRACE("open");
        return 1;
    }

    // 获取输入文件的大小
    struct stat statBuf;
    if (fstat(inputFd, &statBuf) == -1) {
        BSLOG_TRACE("fstat");
        close(inputFd);
        return 1;
    }

    // 打开输出文件（目标文件）
    int outputFd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outputFd == -1) {
        BSLOG_TRACE("open");
        close(inputFd);
        return 1;
    }

    // 使用 sendfile 进行零拷贝传输
    off_t offset = 0;
    ssize_t sentBytes = sendfile(outputFd, inputFd, &offset, statBuf.st_size);

    if (sentBytes == -1) {
        BSLOG_TRACE("sendfile");
        close(inputFd);
        close(outputFd);
        return 1;
    }

    // 关闭文件描述符
    close(inputFd);
    close(outputFd);

    return 0;

}