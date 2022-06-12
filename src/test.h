#pragma once
#include "JsonParse.h"
#include "JsonStringify.h"
#include "MemManger.h"
#include "PreDefineFn.h"
#include "ResouceControl.h"
#include "Runtime.h"
#include "sion/SionGlobal.h"
#include "util.h"

namespace agumi
{
void TestMemMange()
{
    auto& mem = MemManger::Get();
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

    ASSERT_2UL(mem.gc_root["hl"].Obj().Ptr(), mem.gc_root["cao"].Obj().Ptr())
    auto o1 = mem.gc_root["cao"];
    o1["emmmm"] = 12345;
    ASSERT2(mem.gc_root["cao"]["emmmm"].Number() == 12345.0, "object的引用修改异常")
    auto& olv = o1["emmmm"].Number();
    olv = 2333.1;
    ASSERT2(mem.gc_root["cao"]["emmmm"].Number() == 2333.1, "object的引用修改异常")
    Value obj = Object();
    o1["test-obj"] = obj;
    ASSERT_2UL(o1["test-obj"].Type(), ValueType::object);
    o1["test-obj"] = nullptr;
    mem.GC();
    ASSERT(mem.gc_root["c"][6]["emmm"][2].Number(), 3.0)
    ASSERT_T(!MemAllocCollect::obj_quene.Includes(obj.Obj().Ptr()))
    ASSERT_T(MemAllocCollect::obj_quene.Includes(mem.gc_root["cao"].Obj().Ptr()))
    Value v1 = Array();
    auto v2 = v1;
    ASSERT_2UL(v1.Arr().Ptr(), v2.Arr().Ptr())
}

void TestGcPref(int count = 100 * 1000)
{
    Vector<int> ms_vec;
    auto& mem = MemManger::Get();
    Profile p;
    p.Start();
    auto c = [&] {
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
    std::cout << String::Format("保持count:{} gc:{}ms, count:{}->0 gc:{}ms", count, ms_vec[0], count, ms_vec[1]) << std::endl;
}

void TestJsonParsePref(int count = 1000)
{
    const String json = LoadFile("tweet.json");
    auto& mem = MemManger::Get();
    mem.GC();
    int start_arr_size = MemAllocCollect::vec_quene.size();
    int start_obj_size = MemAllocCollect::obj_quene.size();
    Profile p;
    p.Start();
    for (int i = count - 1; i >= 0; i--)
    {
        JsonParse::Call(json);
    }
    p.Pause();
    std::cout << String::Format("JsonParse all:{}ms count:{} per:{}ms", p.ToMs(), count, p.ToMs(count)) << std::endl
              << String::Format("新增 数组数量:{} 对象数量:{}", MemAllocCollect::vec_quene.size() - start_arr_size,
                                MemAllocCollect::obj_quene.size() - start_obj_size)
              << std::endl;
    ;
}

void TestJson()
{
    auto& mem = MemManger::Get();
    ASSERT(mem.gc_root["__23333"].ToString(), "null")
    ASSERT(JsonParse::Call(" 123.123 ").Number(), 123.123)
    ASSERT(JsonParse::Call(" [1,2,3,4,2]")[4].ToString(), "2")
    ASSERT(JsonParse::Call(" [1,2,3,4,2]").Arr().Src().Map<String>([](Value i) { return i.ToString(); }).Join(), "1,2,3,4,2")
    ASSERT(JsonParse::Call(" true").Bool(), true)
    ASSERT(JsonParse::Call("false").Bool(), false)
    ASSERT(JsonParse::Call(R"( "hello world" )").ToString(), "hello world")
    ASSERT_2UL(JsonParse::Call("null").Type(), ValueType::null)
    auto parse_obj = JsonParse::Call(R"({
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
    // cout << JsonStringify::Call(parse_obj) << endl;
    ASSERT(parse_obj["dddd"][0].Number(), 123.233)
    ASSERT(parse_obj["dddd"][2][0].Number(), double(1234))
    ASSERT(parse_obj["dddd"].Arr().Src().back().ToString(), "32]{{22")
    ASSERT(parse_obj["dddd"][2][5]["艹"].Arr().Src().size(), 4)
    ASSERT_T(parse_obj["c"]["ec"]["ddd"].Bool())
    ASSERT(parse_obj["c"]["ec"]["f\""].ToString(), "c[cc{c&%$")
    ASSERT_T(parse_obj["eeee"].Bool())
    ASSERT_2UL(parse_obj["shit"].Type(), ValueType::null)
    ASSERT_2UL(parse_obj["emmm"].Type(), ValueType::null)
    ASSERT_T(mem.gc_root.DeepCompare(mem.gc_root))
    // 测试json序列化再解析有没有变化，因为字典序会变所以不能直接比字符串，因为会有环形引用所不能用gc根
    auto str = JsonStringify::Call(mem.gc_root);
    auto jsv = JsonParse::Call(str);
    auto str_2nd = JsonStringify::Call(jsv);
    auto jsv_2nd = JsonParse::Call(str_2nd);
    ASSERT(jsv, jsv_2nd)
    {
        Object a;
        Array b;
        b.Src().push_back(a);
        b.Src().push_back(a);
        ASSERT_T(!JsonStringify::Call(b).Includes("circle ref"))
        b.Src().push_back(b);
        ASSERT_T(JsonStringify::Call(b).Includes("circle ref"))
    }
    mem.GC();
}

void TestToken()
{
    ASSERT_T(Token("1").IsNumericLiteral())
    ASSERT_T(Token("1234").IsNumericLiteral())
    ASSERT_T(!Token("123l4").IsNumericLiteral())
    ASSERT_T(!Token("123,4").IsNumericLiteral())
    ASSERT_T(Token("123.4").IsNumericLiteral())
    ASSERT_T(Token("-123.4").IsNumericLiteral())
    ASSERT_T(!Token("123__").IsNumericLiteral())
    ASSERT_T(Token("true").IsBoolLiteral())
    ASSERT_T(Token("true").IsLiteral())
    ASSERT_T(!Token("_S$abcd_").IsBoolLiteral())
    ASSERT_T(Token("_").IsIdentifier())
    ASSERT_T(Token("$").IsIdentifier())
    ASSERT_T(!Token("12334").IsIdentifier())
    ASSERT_T(Token("abcd").IsIdentifier())
    ASSERT_T(Token("ab$_cd").IsIdentifier())
    ASSERT_T(Token("_S$abcd_").IsIdentifier())
    ASSERT_T(!Token("_S$abcd_").IsKeyWord())
    ASSERT_T(Token("_S$abcd_").IsIdentifier())
}

void TestAst()
{
    /* {
        auto tf = GeneralTokenizer(R"(const a = 1;)", GeneralTokenizer::js).Start();
        ASSERT(tf.size(), 5)
        ASSERT_T(tf[0].IsKeyWord())
        ASSERT_T(tf[0].Is(const_))
        ASSERT_T(tf[3].IsNumericLiteral())
        auto p1 = ConstructAST(tf);
        ASSERT(p1.body.size(), 1)
        auto variable_declaration_sp = p1.body[0];
        ASSERT_2UL(variable_declaration_sp->Type(), StatementType::variableDeclaration);
        auto variable_declaration_p = static_cast<VariableDeclaration *>(variable_declaration_sp.get());
        ASSERT(variable_declaration_p->declarations.size(), 1)
        auto var_decl_item_sp = variable_declaration_p->declarations[0];
        ASSERT(var_decl_item_sp->initialed, true)
        ASSERT(var_decl_item_sp->id.tok.kw, "a")
        ASSERT_2UL(var_decl_item_sp->init->Type(), StatementType::numberLiteralInit)
        auto number_p = *static_cast<NumberLiteralInit *>(var_decl_item_sp->init.get());
        ASSERT(number_p.tok.kw, "1")
    }*/
}

void TestVec()
{
    auto v = Vector<int>::From({1, 2, 3, 4});
    ASSERT(v.Join(), "1,2,3,4")
    ASSERT(Vector<int>::From({1}).Join(), "1")
    ASSERT(Vector<int>().Join(), "")
}

void TestString()
{
    // split和js的基本一致.不过默认丢弃空的元素
    ASSERT(String("     ").Split(""), Vector<String>({" ", " ", " ", " ", " "}));
    ASSERT(String("123  123").Split("123"), Vector<String>({"  "}));
    ASSERT(String("123").Split("123"), Vector<String>());
    ASSERT(String("123").Split("1234"), Vector<String>({"123"}));
    ASSERT(String("1,2,3,4,54").FindAll(","), Vector<int>({1, 3, 5, 7}))
    ASSERT(String::Format("hello {}", "world"), "hello world")
    ASSERT(String::Format("hello {}", "world{}", "123"), "hello world{}")
    ASSERT(String::Format("hello {} {} {}", "world{}"), "hello world{} {} {}")
    ASSERT(String(R"({ "hello\"": "world" })").Unescape(), R"({ "hello"": "world" })")
    ASSERT(String(R"({ "hello\\\"": "world\"" })").Unescape().Unescape().Unescape(), R"({ "hello"": "world"" })")
    ASSERT(String(R"({ "hello": "world" })").Escape(), R"({ \"hello\": \"world\" })")
    ASSERT(String(R"({ "hello": "world" })").Escape().Escape(), R"({ \\"hello\\": \\"world\\" })")
    ASSERT(String(R"({ "hello": "world" })").Escape().Escape().Unescape().Unescape(), R"({ "hello": "world" })")
    const String utf8str = "苟利国家生死以";
    const String emoji = "🐶🍐🍎🏡🆙💀🐜";
    ASSERT(utf8str.Ulength(), 7)
    ASSERT(emoji.Ulength(), 7)
    ASSERT(utf8str.USubStr(0, 2), "苟利")
    ASSERT(utf8str.USubStr(2, 2), "国家")
    ASSERT(emoji.Split("", -1, true, true).Join(""), emoji)
    ASSERT(emoji.USubStr(0, 2), "🐶🍐")
    ASSERT(emoji.USubStr(2, 3), "🍎🏡🆙")
    ASSERT(emoji.USubStr(2), "🍎🏡🆙💀🐜")
    ASSERT(String::FromUtf8EncodeStr(R"(\u6d4b12\u8bd51234)"), "测12试1234")
    ASSERT(String::FromCodePoint("0x1f9d9"), "🧙")
}

Value VmRunScript(VM& vm, String src, bool ast_c = false, bool tok_c = false, String file = GeneralTokenizer::ReplFileName())
{
    auto tfv = GeneralTokenizer::Agumi(src, PathCalc(vm.working_dir, file));
    if (tok_c)
    {
        for (auto&& i : tfv)
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
        std::cout << JsonStringify::Call(ast.ToJson()) << std::endl;
    }
    return vm.Run(ast);
}

void TestScriptExec(String working_dir)
{
    VM vm;
    vm.working_dir = working_dir;
    AddPreDefine(vm);
#define RUN2STR(x) JsonStringify::Call(VmRunScript(vm, x), 0)
    VmRunScript(vm, "const fib = a => (a>1) ? (fib(a-1) + fib(a-2)) : a");
    ASSERT(RUN2STR("fib(10)"), "55")
    ASSERT(RUN2STR("'hello world'.byte_len()"), "11")
    ASSERT(RUN2STR("[1,2,3,4].push(5,6)"), "[1,2,3,4,5,6]")
    ASSERT(RUN2STR("[typeof([]),typeof(''),typeof(1),typeof(()=>1)]"), "[\"array\",\"string\",\"number\",\"function\"]")
    ASSERT(RUN2STR("lens(1,2)([0,[0,1,2]])"), "2")
    VmRunScript(vm, "const instFactory = inst => () => inst");
    VmRunScript(vm, "const getInst = instFactory([1,2,3,4,5])");
    ASSERT(RUN2STR("[] == []"), "false")
    ASSERT(RUN2STR("getInst() == getInst()"), "true")
    String file_name = PathCalc(working_dir, "script/test/index.as");
    P(file_name)
    String file = LoadFile(file_name);
    ASSERT_T(file.size() > 0)
    VmRunScript(vm, file, false, false, file_name);
}

void TestPath()
{
    ASSERT(PathCalc("/home", "dd"), "/home/dd");
    ASSERT(PathCalc("/home", "/dd"), "/dd");
    ASSERT(PathCalc("/home/e", "../../../"), "/");
    ASSERT(PathCalc("/home/cc", ".."), "/home");
    ASSERT(PathCalc("/home/cc", "dd", ".."), "/home/cc");
    ASSERT(PathCalc("/home/cc", "dd", "../"), "/home/cc");
    ASSERT(PathCalc("/home/cc", "dd/ff", "../cd"), "/home/cc/dd/cd");
    ASSERT(PathCalc("/home/cc"), "/home/cc");
}

void Test(String working_dir)
{

    String e = R"(
        {
            "hello\"": "world,\"\"\"\"&&&&"
        }
    )";
    auto v = JsonParse::Call(e);
    std::cout << JsonStringify::Call(v) << std::endl;
    std::cout << JsonStringify::Call(v, 2, false) << std::endl;
    TestJsonParsePref();
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

} // namespace agumi
