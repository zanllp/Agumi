#include "test.h"
#include <csignal>


agumi::String FormatError(agumi::String err, agumi::String stack_trace)
{
    std::stringstream str;
    str << color_red_s << "error :" << color_e << "\t" << err << "\n";
    if (err.find("stack trace:") == -1)
    {
        str << stack_trace << '\n';
    }

    return str.str();
}

void SignalHandler(int signal)
{
    if (signal == SIGABRT)
    {
        std::exit(1);
    }
    else if (signal == SIGINT)
    {
        P("中断")
        std::abort();
    }
}

int main(int argc, char** argv)
{
    std::signal(SIGABRT, SignalHandler);
    std::signal(SIGINT, SignalHandler);
#ifdef LET_IT_CRASH
    P("{}let it Crash{}", color_green_s, color_e)
#endif
    agumi::Token::Init();
    SionGlobal::async_thread_pool.SetThrowIfHasErrMsg(true).Start();
    auto arg = agumi::CreateVecFromStartParams(argc, argv);
    auto working_dir = std::filesystem::current_path().generic_string(); // 文件地址获取文件夹地址
    auto test_relative_path = getenv("WORKING_DIR_RELATIVE_PATH");
    if (test_relative_path != nullptr)
    {
        P("test_relative_path:{}", test_relative_path)
        working_dir = agumi::PathCalc(working_dir, test_relative_path);
    }
    P("working dir:{}", working_dir)
    if (argc < 2)
    {
        P("see https://github.com/zanllp/agumi for more help information");
        return 1;
    }
    agumi::Value conf = agumi::Object();
    for (auto i : arg)
    {
        if (i.StartWith('-'))
        {
            if (i.find('=') != std::string::npos)
            {
                auto kv = agumi::String(i.substr(1)).Split('=', 1);
                conf[kv[0]] = kv[1];
            }
            else
            {
                conf[i.substr(1)] = true;
            }
        }
    }
    auto test = conf.In("test");
    auto repl = conf.In("repl");
    auto exec = conf.In("exec") ? conf["exec"] : nullptr;
    if (exec.Type() == agumi::ValueType::boolean)
    {
        exec = "index.as";
    }

    if (repl)
    {
        agumi::VM vm;
        vm.process_arg = conf;
        vm.working_dir = working_dir;
        try
        {
            AddPreDefine(vm);
        }
        catch(const std::exception& e)
        {
            std::cerr << FormatError(e.what(), vm.StackTrace());
            std::abort();
        }
        std::array<char, 1000> buf = {0};
        auto ptr = &buf.at(0);
        std::cout << color_green_s << "input :" << color_e << "\t";
        while (std::cin.getline(ptr, 999))
        {
#ifndef LET_IT_CRASH
            try
            {
#endif
                agumi::String src = ptr;
                auto tfv = agumi::GeneralTokenizer::Agumi(src);

                auto ast = agumi::Compiler().ConstructAST(tfv);
                auto res = vm.Run(ast);
                std::cout << color_blue_s << "output:" << color_e << "\t" << agumi::JsonStringify::Call(res) << std::endl;
#ifdef LET_IT_CRASH
                try
                {
#endif
                }
                catch (const std::exception& e)
                {
                    std::cerr << FormatError(e.what(), vm.StackTrace());
                    vm.ctx_stack.resize(1); // 抛异常时只保留顶层的变量
                }

                buf.fill(0);
                std::cout << color_green_s << "input :" << color_e << "\t";
            }

            return 1;
        }

        if (exec.ToBool())
        {
            agumi::VM vm;
            vm.working_dir = working_dir;
            vm.process_arg = conf;
#ifndef LET_IT_CRASH
            try
#endif
            {
                AddPreDefine(vm);
                VmRunScript(vm, LoadFile(exec.ToString()), false, false, exec.ToString());
            }
#ifndef LET_IT_CRASH
            catch (const std::exception& e)
            {
                std::cerr << FormatError(e.what(), vm.StackTrace());
            }
#endif
            return 1;
        }
        if (test)
        {
            agumi::Test(working_dir);
        }
        return 1;
    }
