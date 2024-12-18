#pragma once

#include <string>

namespace ns_util
{
    // 路径模块
    class PathUtil
    {
#define DIR_PATH "./temp/"
    public:
        static std::string ContactFileSuffix(const std::string &filename, const std::string &suffix)
        {
            return DIR_PATH + filename + "." + suffix;
        }

        // 源文件
        static std::string SrcFile(const std::string &filename)
        {
            return ContactFileSuffix(filename, "cc");
        }

        // 可执行文件
        static std::string ExeFile(const std::string &filename)
        {
            return ContactFileSuffix(filename, "exe");
        }

        // 错误信息
        static std::string ErrFile(const std::string &filename)
        {
            return ContactFileSuffix(filename, "err");
        }

        // 标准输出
        static std::string StdoutFile(const std::string &filename)
        {
            return ContactFileSuffix(filename, "stdout");
        }

        // 标准输入
        static std::string StdinFile(const std::string &filename)
        {
            return ContactFileSuffix(filename, "stdin");
        }
        
        // 标准错误
        static std::string StderrFile(const std::string &filename)
        {
            return ContactFileSuffix(filename, "stderr");
        }
    };
}