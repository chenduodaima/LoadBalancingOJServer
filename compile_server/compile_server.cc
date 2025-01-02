#include "compile_run.hpp"
#include "../commen/err.hpp"
#include "../commen/httplib.h"

using namespace ns_compiler;
using namespace ns_runner;
using namespace ns_err;
using namespace ns_compile_run;

void Usage(std::string proc)
{
    std::cerr << "Usage: " << "\n\t" << proc << " port" << std::endl;
}

void Start(const httplib::Request &req, httplib::Response &resp)
{
    LogMessage(Debug, "recv request, body: %s\n", req.body.c_str());
    std::string in_json = req.body;
    std::string out_json;
    if (!in_json.empty())
    {
        CompileAndRun::Start(in_json, out_json);
        resp.set_content(out_json, "application/json;charset=utf-8");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }
    lg.Clear();
    int port = atoi(argv[1]);
    httplib::Server svr;
    svr.Post("/compile_and_run", Start);
    svr.listen("0.0.0.0", port);
    return 0;
}