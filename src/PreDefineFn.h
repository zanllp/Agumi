#pragma once
#include "JsValue.h"
#include "Runtime.h"
#include "JsArray.h"
#include "Json.h"
#include "JsObject.h"
#include "sion.h"
#define JSON_PARSE(e) JsonNext().JsonParse(e)
void AddPreDefine (VM& vm)
{
    auto json_module = JsObject();
        vm.ctx_stack[0].var["json"] = json_module;
        json_module["parse"] = vm.DefineFunc([&](Vector<JsValue> args) -> JsValue {
            return JSON_PARSE(args.GetOrDefault(0).ToString());
        });
        json_module["stringify"] = vm.DefineFunc([&](Vector<JsValue> args) -> JsValue {
            return Json::Stringify(args.GetOrDefault(0).ToString());
        });
        vm.DefineGlobalFunc("request", [&](Vector<JsValue> args) -> JsValue {
            auto resp = sion::Fetch(args[0].ToString());
            auto res = JsObject();
            res["data"] = resp.Body();
            return res;
        });
        vm.DefineGlobalFunc("eval", [&](Vector<JsValue> args) -> JsValue {
            auto script = args.GetOrDefault(0).ToString();
            auto enable_curr_vm = args.GetOrDefault(1).ToBool();
            auto tfv = GeneralTokenizer::Js(script);
            auto ast = Compiler().ConstructAST(tfv);
            return enable_curr_vm ? vm.Run(ast) : VM().Run(ast);
        });
        vm.DefineGlobalFunc("str", [&](Vector<JsValue> args) -> JsValue {
            if (args.size() == 0)
            {
                return "";
            }
            auto tpl = args[0].ToString();
            if (args.size() != 1)
            {
                return String::Format(tpl);
            }
            
            return tpl;
        });
        vm.DefineGlobalFunc("log", [&](Vector<JsValue> args) -> JsValue {
            cout << args.Map<String>([](JsValue arg) { return arg.ToString(); }).Join() << endl;
            return JsValue::undefined;
        });
        vm.DefineGlobalFunc("loadFile", [&](Vector<JsValue> args) -> JsValue {\
            return LoadFile(args.GetOrDefault(0).ToString());
        });
        vm.DefineGlobalFunc("mem", [&](Vector<JsValue> args) -> JsValue {
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
        });
}