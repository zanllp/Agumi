
#include "Json.h"
#include "MemManger.h"
#include "JsonParse.h"
#include "util.h"
#include "sion.h"
#include "Runtime.h"
#include "PreDefineFn.h"
#include <filesystem>

using namespace std;
using namespace agumi;
const string json = LoadFile("tweet.json");
const String color_red_s = "\033[31m";
const String color_green_s = "\033[32m";
const String color_blue_s = "\033[34m";
const String color_e = "\033[0m";

String FormatError(String err, String stack_trace)
{
    stringstream str;
    str << color_red_s << "error :" << color_e << "\t" << err << "\n";
    if (err.find("stack trace:") == -1)
    {
        str << stack_trace << '\n';
    }
    
    return str.str();
}

void TestMemMange()
{
    auto &mem = MemManger::Get();
    mem.gc_root["c"] = Array();
    mem.gc_root["c"].Arr().Src().resize(10);
    mem.gc_root["c"][1] = 1;
    mem.gc_root["c"][6] = Object({
        {"fuck", "ass"},
        {"cao s", Object({{"fuck", "cao"}})},
        {"emmm", Array({1, 2, 3, 4, 5, 6, 7, 23333.666, mem.gc_root["c"], Object(), Object()})},
    });
    mem.gc_root["cao"] = Object({
        {"fuck", "ass"},
        {"cao s", Object({{"fuck", "cao"}})},
        {"emmm", 23333},
    });
    mem.gc_root["hl"] = mem.gc_root["cao"];

    ASS_2UL(mem.gc_root["hl"].Obj().Ptr(), mem.gc_root["cao"].Obj().Ptr())
    auto o1 = mem.gc_root["cao"];
    o1["emmmm"] = 12345;
    ASS2(mem.gc_root["cao"]["emmmm"].Get<double>() == (double)12345, "object的引用修改异常")
    auto &olv = o1["emmmm"].Get<double>();
    olv = 2333.1;
    ASS2(mem.gc_root["cao"]["emmmm"].Get<double>() == 2333.1, "object的引用修改异常")
    Value obj = Object();
    o1["test-obj"] = obj;
    ASS_2UL(o1["test-obj"].Type(), ValueType::object);
    o1["test-obj"] = nullptr;
    mem.GC();
    ASS(mem.gc_root["c"][6]["emmm"][2].Get<double>(), (double)3)
    ASS_T(!MemAllocCollect::obj_quene.Includes(obj.Obj().Ptr()))
    ASS_T(MemAllocCollect::obj_quene.Includes(mem.gc_root["cao"].Obj().Ptr()))
    Value v1 = Array();
    auto v2 = v1;
    ASS_2UL(v1.Arr().Ptr(), v2.Arr().Ptr())
}

void TestGcPref(int count = 100 * 1000)
{
    vector<int> ms_vec;
    auto &mem = MemManger::Get();
    Profile p;
    p.Start();
    auto c = [&]
    {
        auto arr = Array();
        mem.gc_root["ccc"] = arr;
        for (int i = count - 1; i >= 0; i--)
        {
            auto arr2 = Array();
            arr.Src().push_back(arr2);
            arr = arr2;
        }
        mem.GC();
    };
    c();
    p.Pause();
    ms_vec.push_back(p.ToMs());
    p.Reset();
    mem.gc_root["ccc"] = nullptr;
    mem.GC();
    p.Pause();
    ms_vec.push_back(p.ToMs());
    cout << String::Format("保持count:{} gc:{}ms, count:{}->0 gc:{}ms", count, ms_vec[0], count, ms_vec[1]) << endl;
}

void TestJsonNextPref(int count = 1000)
{
    auto &mem = MemManger::Get();
    mem.GC();
    int start_arr_size = MemAllocCollect::vec_quene.size();
    int start_obj_size = MemAllocCollect::obj_quene.size();
    Profile p;
    p.Start();
    for (int i = count - 1; i >= 0; i--)
    {
        JsonNext().JsonParse(json);
    }
    p.Pause();
    cout << String::Format("JsonNext all:{}ms count:{} per:{}ms", p.ToMs(), count, p.ToMs(count)) << endl
         << String::Format("新增 数组数量:{} 对象数量:{}", MemAllocCollect::vec_quene.size() - start_arr_size, MemAllocCollect::obj_quene.size() - start_obj_size) << endl;
    ;
}

void TestJson()
{
    auto &mem = MemManger::Get();
    ASS(mem.gc_root["__23333"].ToString(), "null")
    ASS(JSON_PARSE(" 123.123 ").Get<double>(), 123.123)
    ASS(JSON_PARSE(" [1,2,3,4,2]")[4].ToString(), "2")
    ASS(JSON_PARSE(" [1,2,3,4,2]").Arr().Src().Map<String>([](Value i)
                                                           { return i.ToString(); })
            .Join(),
        "1,2,3,4,2")
    ASS(JSON_PARSE(" true").Get<bool>(), true)
    ASS(JSON_PARSE("false").Get<bool>(), false)
    ASS(JSON_PARSE(R"( "hello world" )").ToString(), "hello world")
    ASS_2UL(JSON_PARSE("null").Type(), ValueType::null)
    auto parse_obj = JSON_PARSE(R"({
    "exc": 2333,
    "c": {
        "c": "{}\"",
        "ec": { // 歪比歪比
            "ddd": true,
            "f\"": "c[cc{c&%$"
        }
    },
    "dddd": [
        123.233,
        /* 歪比巴卜 */
        1,
        [1234,
            -12.1,
            -2,
            3.3,
            null,
            {
                "艹":/* are you good 马来西亚 */ ["中文","// 22333","/* 222*/",null]
            }
        ],
        "2ca]o",
        "3\"2',2$%^&^&2",
        {
            "dd": null
        },
        "32]{{22"
    ],
    "shit": null,
    "eeee": true
})");
    // cout << Json::Stringify(parse_obj) << endl;
    ASS(parse_obj["dddd"][0].Get<double>(), 123.233)
    ASS(parse_obj["dddd"][2][0].Get<double>(), double(1234))
    ASS(parse_obj["dddd"].Arr().Src().back().ToString(), "32]{{22")
    ASS(parse_obj["dddd"][2][5]["艹"].Arr().Src().size(), 4)
    ASS_T(parse_obj["c"]["ec"]["ddd"].Get<bool>())
    ASS(parse_obj["c"]["ec"]["f\""].ToString(), "c[cc{c&%$")
    ASS_T(parse_obj["eeee"].Get<bool>())
    ASS_2UL(parse_obj["shit"].Type(), ValueType::null)
    ASS_2UL(parse_obj["emmm"].Type(), ValueType::null)
    ASS_T(mem.gc_root.DeepCompare(mem.gc_root))
    // 测试json序列化再解析有没有变化，因为字典序会变所以不能直接比字符串，因为会有环形引用所不能用gc根
    auto str = Json::Stringify(mem.gc_root);
    auto jsv = JSON_PARSE(str);
    auto str_2nd = Json::Stringify(jsv);
    auto jsv_2nd = JSON_PARSE(str_2nd);
    ASS(jsv, jsv_2nd)
    mem.GC();
}

void TestToken()
{
    ASS_T(Token("1").IsNumericLiteral())
    ASS_T(Token("1234").IsNumericLiteral())
    ASS_T(!Token("123l4").IsNumericLiteral())
    ASS_T(!Token("123,4").IsNumericLiteral())
    ASS_T(Token("123.4").IsNumericLiteral())
    ASS_T(Token("-123.4").IsNumericLiteral())
    ASS_T(!Token("123__").IsNumericLiteral())
    ASS_T(Token("true").IsBoolLiteral())
    ASS_T(Token("true").IsLiteral())
    ASS_T(!Token("_S$abcd_").IsBoolLiteral())
    ASS_T(Token("_").IsIdentifier())
    ASS_T(Token("$").IsIdentifier())
    ASS_T(!Token("12334").IsIdentifier())
    ASS_T(Token("abcd").IsIdentifier())
    ASS_T(Token("ab$_cd").IsIdentifier())
    ASS_T(Token("_S$abcd_").IsIdentifier())
    ASS_T(!Token("_S$abcd_").IsKeyWord())
    ASS_T(Token("_S$abcd_").IsIdentifier())
}

void TestAst()
{
    /* {
        auto tf = GeneralTokenizer(R"(const a = 1;)", GeneralTokenizer::js).Start();
        ASS(tf.size(), 5)
        ASS_T(tf[0].IsKeyWord())
        ASS_T(tf[0].Is(const_))
        ASS_T(tf[3].IsNumericLiteral())
        auto p1 = ConstructAST(tf);
        ASS(p1.body.size(), 1)
        auto variable_declaration_sp = p1.body[0];
        ASS_2UL(variable_declaration_sp->Type(), StatementType::variableDeclaration);
        auto variable_declaration_p = static_cast<VariableDeclaration *>(variable_declaration_sp.get());
        ASS(variable_declaration_p->declarations.size(), 1)
        auto var_decl_item_sp = variable_declaration_p->declarations[0];
        ASS(var_decl_item_sp->initialed, true)
        ASS(var_decl_item_sp->id.tok.kw, "a")
        ASS_2UL(var_decl_item_sp->init->Type(), StatementType::numberLiteralInit)
        auto number_p = *static_cast<NumberLiteralInit *>(var_decl_item_sp->init.get());
        ASS(number_p.tok.kw, "1")
    }*/
}

void TestVec()
{
    auto v = Vector<int>::From({1, 2, 3, 4});
    ASS(v.Join(), "1,2,3,4")
    ASS(Vector<int>::From({1}).Join(), "1")
    ASS(Vector<int>().Join(), "")
}

void TestString()
{
    // split和js的基本一致.不过默认丢弃空的元素
    ASS(String("     ").Split(""), Vector<String>({" ", " ", " ", " ", " "}));
    ASS(String("123  123").Split("123"), Vector<String>({"  "}));
    ASS(String("123").Split("123"), Vector<String>());
    ASS(String("123").Split("1234"), Vector<String>({"123"}));
    ASS(String("1,2,3,4,54").FindAll(","), Vector<int>({1, 3, 5, 7}))
    ASS(String::Format("hello {}", "world"), "hello world")
    ASS(String::Format("hello {}", "world{}", "123"), "hello world{}")
    ASS(String::Format("hello {} {} {}", "world{}"), "hello world{} {} {}")
    ASS(String(R"({ "hello\"": "world" })").Unescape(), R"({ "hello"": "world" })")
    ASS(String(R"({ "hello\\\"": "world\"" })").Unescape().Unescape().Unescape(), R"({ "hello"": "world"" })")
    ASS(String(R"({ "hello": "world" })").Escape(), R"({ \"hello\": \"world\" })")
    ASS(String(R"({ "hello": "world" })").Escape().Escape(), R"({ \\"hello\\": \\"world\\" })")
    ASS(String(R"({ "hello": "world" })").Escape().Escape().Unescape().Unescape(), R"({ "hello": "world" })")
}

Value VmRunScript(VM &vm, String src, bool ast_c = false, bool tok_c = false, String file = GeneralTokenizer::ReplFileName())
{
    auto tfv = GeneralTokenizer::Agumi(src, PathCalc(vm.working_dir, file));
    if (tok_c)
    {
        for (auto &&i : tfv)
        {
            if (i.IsIdentifier())
            {
                P("{} {}", i.ToDebugStr(), i.UniqId());
            }
        }
    }
    auto ast = Compiler().ConstructAST(tfv);
    if (ast_c)
    {
        cout << Json::Stringify(ast.ToJson()) << endl;
    }
    return vm.Run(ast);
}

void TestScriptExec(String working_dir)
{
    VM vm;
    vm.working_dir = working_dir;
    AddPreDefine(vm);
#define RUN2STR(x) Json::Stringify(VmRunScript(vm, x), 0)
    VmRunScript(vm, "const fib = a => (a>1) ? (fib(a-1) + fib(a-2)) : a");
    ASS(RUN2STR("fib(10)"), "55")
    ASS(RUN2STR("'hello world'.length()"), "11")
    ASS(RUN2STR("[1,2,3,4].push(5,6)"), "[1,2,3,4,5,6]")
    ASS(RUN2STR("[typeof([]),typeof(''),typeof(1),typeof(()=>1)]"), "[\"array\",\"string\",\"number\",\"function\"]")
    ASS(RUN2STR("lens(1,2)([0,[0,1,2]])"), "2")
    VmRunScript(vm, "const instFactory = inst => () => inst");
    VmRunScript(vm, "const getInst = instFactory([1,2,3,4,5])");
    ASS(RUN2STR("[] == []"), "false")
    ASS(RUN2STR("getInst() == getInst()"), "true")
    String file_name = PathCalc(working_dir, "script/test/index.as");
    P(file_name)
    String file = LoadFile(file_name);
    ASS_T(file.size() > 0)
    VmRunScript(vm, file, false, false, file_name);
}

void TestPath()
{
    ASS(PathCalc("/home", "dd"), "/home/dd");
    ASS(PathCalc("/home", "/dd"), "/dd");
    ASS(PathCalc("/home/e", "../../../"), "/");
    ASS(PathCalc("/home/cc", ".."), "/home");
    ASS(PathCalc("/home/cc", "dd", ".."), "/home/cc");
    ASS(PathCalc("/home/cc", "dd", "../"), "/home/cc");
    ASS(PathCalc("/home/cc", "dd/ff", "../cd"), "/home/cc/dd/cd");
    ASS(PathCalc("/home/cc"), "/home/cc");
}

int main(int argc, char **argv)
{
#ifdef LET_IT_CRASH
    P("{}let it Crash{}", color_green_s, color_e)
#endif
    Token::Init();
    auto arg = CreateVecFromStartParams(argc, argv);
    auto working_dir = filesystem::current_path().generic_string(); // 文件地址获取文件夹地址
    auto test_relative_path = getenv("WORKING_DIR_RELATIVE_PATH");
    if (test_relative_path != nullptr)
    {
        P("test_relative_path:{}", test_relative_path)
        working_dir = PathCalc(working_dir, test_relative_path);
    }
    P("working dir:{}", working_dir)
    if (argc < 2)
    {
        P("see https://github.com/zanllp/agumi for more help information");
        return 1;
    }
    Value conf = Object();
    for (auto i : arg)
    {
        if (i.StartWith('-'))
        {
            if (i.find('=') != string::npos)
            {
                auto kv = String(i.substr(1)).Split('=', 1);
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
    if (exec.Type() == ValueType::boolean)
    {
        exec = "index.as";
    }

    if (repl)
    {
        VM vm;
        vm.process_arg = conf;
        vm.working_dir = working_dir;
        AddPreDefine(vm);
        array<char, 1000> buf = {0};
        auto ptr = &buf.at(0);
        cout << color_green_s << "input :" << color_e << "\t";
        while (cin.getline(ptr, 999))
        {
#ifndef LET_IT_CRASH
            try
            {
#endif
                String src = ptr;
                auto tfv = GeneralTokenizer::Agumi(src);

                auto ast = Compiler().ConstructAST(tfv);
                auto res = vm.Run(ast);
                cout << color_blue_s << "output:" << color_e << "\t" << Json::Stringify(res) << endl;
#ifdef LET_IT_CRASH
                try
                {
#endif
                }
                catch (const std::exception &e)
                {
                    std::cerr << FormatError(e.what(), vm.StackTrace());
                    vm.ctx_stack.resize(1); // 抛异常时只保留顶层的变量
                }

                buf.fill(0);
                cout << color_green_s << "input :" << color_e << "\t";
            }

            return 1;
        }

        if (exec.ToBool())
        {
            VM vm;
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
            catch (const std::exception &e)
            {
                std::cerr << FormatError(e.what(), vm.StackTrace());
            }
#endif
            return 1;
        }
        if (test)
        {
            string e = R"(
        {
            "hello\"": "world,\"\"\"\"&&&&"
        }
    )";
            auto v = JsonNext().JsonParse(e);
            cout << Json::Stringify(v) << endl;
            cout << Json::Stringify(v, 2, false) << endl;
            TestJsonNextPref();
            TestString();
            TestVec();
            TestToken();
            TestGcPref();
            TestMemMange();
            TestJson();
            TestPath();
            TestScriptExec(working_dir);
            std::cout << "TestPassed;All ok" << std::endl;
        }
        return 0;
    }