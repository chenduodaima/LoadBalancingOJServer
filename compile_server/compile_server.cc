#include "compiler.hpp"
#include "runner.hpp"

using namespace ns_compiler;
using namespace ns_runner;

int main()
{
    Compiler::Compile("codetest");
    Runner::Run("codetest", 10, 10);
    return 0;
}