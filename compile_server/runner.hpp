#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "../commen/log.hpp"
#include "../commen/util.hpp"

namespace ns_runner
{
    using namespace ns_log;
    using namespace ns_util;

    class Runner
    {
    public:
        static void SetProcLimit(int cpu_limit, int mem_limit)
        {
            struct rlimit cpu_rlimit;
            cpu_rlimit.rlim_max = RLIM_INFINITY;
            cpu_rlimit.rlim_cur = cpu_limit;
            setrlimit(RLIMIT_CPU, &cpu_rlimit);

            struct rlimit mem_rlimit;
            mem_rlimit.rlim_max = RLIM_INFINITY;
            mem_rlimit.rlim_cur = mem_limit * 1024;
            setrlimit(RLIMIT_AS, &mem_rlimit);
        }

        /***************
         * 运行模块
         * 对于运行来说，并不关心运行的结果是否正确，只关心程序是否跑完
         * 因此对于运行模块来说，子进程的结果只有两种
         * 1. 子进程执行完毕
         * 虽然子进程不关心结果正确，但是系统可以限制程序的时间和空间
         * 2. 子进程运行出错，进程终止，终止的原因是抛出异常，而异常是由于进程收到信号
         ****************/
        static int Run(std::string filename, int cpu_limit, int mem_limit)
        {
            std::string exe_path = PathUtil::ExeFile(filename);
            std::string stdin_path = PathUtil::StdinFile(filename);
            std::string stdout_path = PathUtil::StdoutFile(filename);
            std::string stderr_path = PathUtil::StderrFile(filename);
            umask(0);
            int stdin_fd = open(stdin_path.c_str(), O_CREAT | O_RDONLY, 0644);
            int stdout_fd = open(stdout_path.c_str(), O_CREAT | O_WRONLY, 0644);
            int stderr_fd = open(stderr_path.c_str(), O_CREAT | O_WRONLY, 0644);
            if (stdin_fd < 0 || stdout_fd < 0 || stderr_fd < 0)
            {
                LogMessage(Error, "runner open file failed\n");
                return -1; // 文件打开错误
            }
            pid_t pid = fork();
            if (pid < 0)
            {
                LogMessage(Error, "运行时创建子进程失败\n");
                close(stdin_fd);
                close(stdout_fd);
                close(stderr_fd);
                exit(-2); // 表示子进程创建错误
            }
            else if (pid == 0)
            {
                // 子进程
                dup2(stdin_fd, 0);
                dup2(stdout_fd, 1);
                dup2(stderr_fd, 2);
                SetProcLimit(cpu_limit, mem_limit);
                // std::cout << exe_path << std::endl;
                // struct stat st;
                // std::cout << stat(exe_path.c_str(), &st) << std::endl;
                execl(exe_path.c_str(), exe_path.c_str(), nullptr);
                LogMessage(Error, "运行替换程序错误, pid: %d, exe_path: %s\n", pid, exe_path.c_str());
                exit(1); // 替换错误
            }
            else
            {
                close(stdin_fd);
                close(stdout_fd);
                close(stderr_fd);
                int status = 0;
                waitpid(pid, &status, 0);
                LogMessage(Info, "运行完毕, info: %d\n", (status & 0x7F));
                return (status & 0x7F);
            }
        }
    };
}