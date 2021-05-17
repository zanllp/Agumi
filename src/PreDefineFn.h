#pragma once
#include "JsValue.h"
#include "Runtime.h"
#include "JsArray.h"
#include "Json.h"
#include "JsObject.h"
#include "sion.h"
#define JSON_PARSE(e) JsonNext().JsonParse(e)
namespace agumi
{
    void AddPreDefine(VM &vm)
    {
        auto json_parse = [&](Vector<JsValue> args) -> JsValue
        {
            return JSON_PARSE(args.GetOrDefault(0).ToString());
        };
        auto json_stringify = [&](Vector<JsValue> args) -> JsValue
        {
            return Json::Stringify(args.GetOrDefault(0));
        };
        auto json_module = JsObject({{"parse", vm.DefineFunc(json_parse)},
                                     {"stringify", vm.DefineFunc(json_stringify)}});
        vm.ctx_stack[0].var["json"] = json_module;
        auto fetch_bind = [&](Vector<JsValue> args) -> JsValue
        {
            auto resp = sion::Fetch(args[0].ToString());
            auto res = JsObject();
            res["data"] = resp.Body().c_str();
            return res;
        };
        vm.DefineGlobalFunc("fetch", fetch_bind);
        auto eval = [&](Vector<JsValue> args) -> JsValue
        {
            auto script = args.GetOrDefault(0).ToString();
            auto enable_curr_vm = args.GetOrDefault(1).ToBool();
            auto tfv = GeneralTokenizer::Js(script);
            auto ast = Compiler().ConstructAST(tfv);
            return enable_curr_vm ? vm.Run(ast) : VM().Run(ast);
        };
        vm.DefineGlobalFunc("eval", eval);
        auto log = [&](Vector<JsValue> args) -> JsValue
        {
            auto out = args.Map<String>([](JsValue arg)
                                        { return arg.ToString(); })
                           .Join();
            std::cout << out << std::endl;
            return JsValue::undefined;
        };
        vm.DefineGlobalFunc("log", log);
        auto load_file_bind = [&](Vector<JsValue> args) -> JsValue
        {
            return LoadFile(args.GetOrDefault(0).ToString());
        };
        vm.DefineGlobalFunc("loadFile", load_file_bind);
        auto mem_bind = [&](Vector<JsValue> args) -> JsValue
        {
            size_t idx = 0;
            if (args.size())
            {
                idx = args[0].Get<double>();
                if (idx > vm.ctx_stack.size())
                {
                    THROW_MSG("内存越界 参数:{} vm ctx_stack size:{}", idx, vm.ctx_stack.size())
                }
            }
            return vm.ctx_stack[idx].var;
        };
        vm.DefineGlobalFunc("mem", mem_bind);

        // 定义本地类成员函数
        LocalClassDefine string_def;
        string_def.member_func["length"] = [](JsValue &_this, Vector<JsValue> args) -> JsValue
        {
            return static_cast<int>(_this.GetC<String>().length());
        };
        vm.class_define[JsType::string] = string_def;

        LocalClassDefine array_def;
        array_def.member_func["push"] = [](JsValue &_this, Vector<JsValue> args) -> JsValue
        {
            for (auto &i : args)
            {
                _this.Array().Src().push_back(i);
            }
            return _this;
        };
        array_def.member_func["get"] = [](JsValue &_this, Vector<JsValue> args) -> JsValue
        {
            if (args.size() < 1)
            {
                THROW
            }
            int idx = args[0].GetC<double>();
            return _this[idx];
        };
        vm.class_define[JsType::array] = array_def;
    }
}