#pragma once

#include "compiler.hpp"
#include "runner.hpp"
#include "jsoncpp/json/json.h"

using namespace ns_compiler;
using namespace ns_runner;
using namespace ns_err;
/***************************************
 * 输入:
 * code： 用户提交的代码
 * input: 用户给自己提交的代码对应的输入，不做处理
 * cpu_limit: 时间要求
 * mem_limit: 空间要求
 *
 * 输出:
 * 必填
 * status: 状态码
 * reason: 请求结果
 * 选填：
 * stdout: 我的程序运行完的结果
 * stderr: 我的程序运行完的错误结果
 *
 * 参数：
 * in_json: {"code": "#include...", "input": "","cpu_limit":1, "mem_limit":10240}
 * out_json: {"status":"0", "reason":"","stdout":"","stderr":"",}
 * ************************************/
//状态码：0-编译和运行成功，-1-代码为空，-2-写源文件失败，-3-编译失败，-4表示运行错误，大于0表示运行异常，
namespace ns_compile_run
{
    class CompileAndRun
    {
    public:
        static std::string CodeToDesc(int status_code, const std::string &filename)
        {
            std::string desc;
            switch (status_code)
            {
            case 0:
                desc = "编译和运行成功";
                break;
            case CompileRunErrCode::CODE_EMPTY://-1
                desc = "代码为空";
                break;
            case CompileRunErrCode::WRITE_FILE_FAILED://-2
                desc = "写源文件失败";
                break;
            case CompileRunErrCode::COMPILE_FAILED://-3
                FileUtil::ReadFile(PathUtil::ErrFile(filename), desc, true);
                break;
            case CompileRunErrCode::RUN_FAILED://-4
                desc = "运行失败";
                break;
            case SIGSEGV://11
                desc = "段错误";
                break;
            case SIGABRT://6
                desc = "内存超过限制";
                break;
            case SIGXCPU://24
                desc = "运行超时";
                break;
            case SIGFPE://8
                desc = "浮点异常";
                break;
            case SIGKILL://9
                desc = "被杀死";
                break;
            case SIGTERM://15
                desc = "被终止";
                break;
            default:
                desc = "未知错误" + std::to_string(status_code);
                break;
            }
            return desc;
        }

        static void RemoveFile(const std::string &filename)
        {
            //删除源文件、可执行文件、编译错误文件、标准输出、标准输入、标准错误
            std::string src_path = PathUtil::SrcFile(filename);
            std::string exe_path = PathUtil::ExeFile(filename);
            std::string err_path = PathUtil::ErrFile(filename);
            std::string stdin_path = PathUtil::StdinFile(filename);
            std::string stdout_path = PathUtil::StdoutFile(filename);
            std::string stderr_path = PathUtil::StderrFile(filename);
            if (FileUtil::IsFileExist(src_path))    
            {
                std::remove(src_path.c_str());
            }
            if (FileUtil::IsFileExist(exe_path))
            {
                std::remove(exe_path.c_str());
            }
            if (FileUtil::IsFileExist(err_path))
            {
                std::remove(err_path.c_str());
            }
            if (FileUtil::IsFileExist(stdin_path))
            {
                std::remove(stdin_path.c_str());
            }
            if (FileUtil::IsFileExist(stdout_path))
            {
                std::remove(stdout_path.c_str());
            }
            if (FileUtil::IsFileExist(stderr_path))
            {
                std::remove(stderr_path.c_str());
            }
        }

        static void Start(const std::string &in_json, std::string &out_json)
        {
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json, in_value);
            std::string code = in_value["code"].asString();
            LogMessage(Debug, "code: %s\n", code.c_str());
            std::string input = in_value["input"].asString();
            LogMessage(Debug, "input: %s\n", input.c_str());
            int cpu_limit = in_value["cpu_limit"].asInt();
            LogMessage(Debug, "cpu_limit: %d\n", cpu_limit);
            int mem_limit = in_value["mem_limit"].asInt();
            LogMessage(Debug, "mem_limit: %d\n", mem_limit);
            int status_code = 0;
            int run_result = 0;
            if (code.empty())
            {
                status_code = CompileRunErrCode::CODE_EMPTY;
            }
            else
            {
                LogMessage(Debug, "code is not empty\n");
                std::string filename = FileUtil::UniqueFilename();
                if (!FileUtil::WriteFile(filename, code))
                {
                    status_code = CompileRunErrCode::WRITE_FILE_FAILED;
                }
                else
                {
                    LogMessage(Debug, "write file success\n");
                    com_t status = Compiler::Compiler::Compile(filename);
                    //为0表示编译成功
                    if (status == 0)
                    {
                        LogMessage(Debug, "compile success\n");
                        run_result = Runner::Run(filename, cpu_limit, mem_limit);
                        if (run_result < 0)//运行错误
                        {
                            status_code = CompileRunErrCode::RUN_FAILED;
                        }
                        else
                        {
                            LogMessage(Debug, "run success\n");
                            status_code = run_result;
                        }
                    }
                    else
                    {
                        status_code = CompileRunErrCode::COMPILE_FAILED;//编译失败
                        LogMessage(Debug, "compile failed\n");
                    }
                }
                Json::Value out_value;
                out_value["status"] = status_code;
                out_value["reason"] = CodeToDesc(status_code, filename);
                if (status_code == 0)
                {
                    LogMessage(Debug, "status_code is 0\n");
                    std::string stdout_path = PathUtil::StdoutFile(filename);
                    std::string stderr_path = PathUtil::StderrFile(filename);
                    std::string _stdout;
                    std::string _stderr;
                    //运行成功了，标准输出是运行的结果信息，返回给用户
                    if (FileUtil::ReadFile(stdout_path, _stdout, true))
                    {
                        out_value["stdout"] = _stdout;
                        LogMessage(Debug, "stdout: %s\n", _stdout.c_str());
                    }
                    //标准错误是运行错误信息，返回给用户
                    if (FileUtil::ReadFile(stderr_path, _stderr, true))
                    {
                        out_value["stderr"] = _stderr;
                        LogMessage(Debug, "stderr: %s\n", _stderr.c_str());
                    }
                }
                Json::FastWriter writer;
                out_json = writer.write(out_value);
                LogMessage(Debug, "out_json: %s\n", out_json.c_str());
                RemoveFile(filename);
                LogMessage(Debug, "remove file success\n");
            }
        }
    };
}