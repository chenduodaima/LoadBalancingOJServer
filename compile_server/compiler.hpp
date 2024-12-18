#pragma once


#include "../commen/util.hpp"
#include "../commen/log.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>   


/***************
 * 编译模块
 * 1. 指定唯一值filename找到对应后缀文件xxx.cpp
 * 2. 编译文件
 * 3. 报错是编译时报错，放在xxx.stderr
 * 4. 对于编译来说，只关心编译是否成功，不关心程序是否正确
 ****************/

namespace ns_compiler
{
    using namespace ns_util;
    using namespace ns_log;

    class Compiler
    {
    public:
        // 编译文件
        static bool Compile(const std::string &filename)
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                LogMessage(Error, "编译创建子进程失败\n");
                return false;
            }
            else if (pid == 0)
            {
                // 子进程
                // 创建源文件、可执行程序、编译错误文件
                std::string src_path = PathUtil::SrcFile(filename);
                std::string exe_path = PathUtil::ExeFile(filename);
                std::string err_path = PathUtil::ErrFile(filename);
                umask(0);
                int src_fd = open(src_path.c_str(), O_CREAT | O_WRONLY, 0644);
                int err_fd = open(err_path.c_str(), O_CREAT | O_WRONLY, 0644);
                if (src_fd < 0 || err_fd < 0)
                {
                    LogMessage(Warning, "open file failed, path: %s, %s, %s\n", src_path.c_str(), exe_path.c_str(), err_path.c_str());
                    exit(1); // 文件打开出错
                }
                //替换程序
                dup2(err_fd, 2);
                execlp("g++", "g++", "-o", exe_path.c_str(), src_path.c_str(), "-std=c++11", nullptr);
                LogMessage(Error, "excelp failed, path: %s\n", err_path.c_str());
                exit(2);//替换出错
            }
            else
            {
                // 父进程
                int status = 0;
                waitpid(pid, &status, 0);
                LogMessage(Info, "编译完毕, pid: %d, status: %d\n", pid, status);
                return true;
            }
        }
    };
}