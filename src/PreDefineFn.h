#pragma once
#include "JsValue.h"
#include "Runtime.h"
#include "JsArray.h"
#include "Json.h"
#include "JsObject.h"
#include "sion.h"
#define JSON_PARSE(e) JsonNext().JsonParse(e)
#define BIN_OPERATOR(body) [](JsValue &l, JsValue &r) { return body; };
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
        auto typeof_bind = [&](Vector<JsValue> args) -> JsValue
        {
            return jstype_emun2str[static_cast<int>(args.GetOrDefault(0).Type())];
        };
        vm.DefineGlobalFunc("typeof", typeof_bind);
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
        std::map<KW, std::function<JsValue(JsValue &, JsValue &)>> str_op_def;
        str_op_def[add_] = BIN_OPERATOR(l.GetC<String>() + r.GetC<String>());
        str_op_def[eqeq_] = BIN_OPERATOR(l.GetC<String>() == r.GetC<String>());
        str_op_def[eqeqeq_] = BIN_OPERATOR(l.GetC<String>() == r.GetC<String>());
        str_op_def[add_equal_] = BIN_OPERATOR(l.Get<String>() += r.GetC<String>());
        str_op_def[mul_] = BIN_OPERATOR(l.GetC<String>().Repeat(stoi(r.GetC<String>())));
        string_def.binary_operator_overload[JsType::string] = str_op_def;
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
            if (idx >= _this.ArrayC().SrcC().size())
            {
                THROW
            }

            return _this[idx];
        };
        vm.class_define[JsType::array] = array_def;

        LocalClassDefine num_def;
        num_def.member_func["incr"] = [](JsValue &_this, Vector<JsValue> args) -> JsValue
        {
            return ++_this.Get<double>();
        };
        std::map<KW, std::function<JsValue(JsValue &, JsValue &)>> num_op_def;
        num_op_def[eqeq_] = BIN_OPERATOR(l.GetC<double>() == r.GetC<double>());
        num_op_def[eqeqeq_] = BIN_OPERATOR(l.GetC<double>() == r.GetC<double>());
        num_op_def[not_eq_] = BIN_OPERATOR(l.GetC<double>() != r.GetC<double>());
        num_op_def[not_eqeq_] = BIN_OPERATOR(l.GetC<double>() != r.GetC<double>());
        num_op_def[more_than_] = BIN_OPERATOR(l.GetC<double>() > r.GetC<double>());
        num_op_def[more_than_equal_] = BIN_OPERATOR(l.GetC<double>() >= r.GetC<double>());
        num_op_def[less_than_] = BIN_OPERATOR(l.GetC<double>() < r.GetC<double>());
        num_op_def[less_than_equal_] = BIN_OPERATOR(l.GetC<double>() <= r.GetC<double>());
        num_op_def[add_] = BIN_OPERATOR(l.GetC<double>() + r.GetC<double>());
        num_op_def[sub_] = BIN_OPERATOR(l.GetC<double>() - r.GetC<double>());
        num_op_def[mul_] = BIN_OPERATOR(l.GetC<double>() * r.GetC<double>());
        num_op_def[div_] = BIN_OPERATOR(l.GetC<double>() / r.GetC<double>());
        num_op_def[mod_] = BIN_OPERATOR(fmod(l.GetC<double>(), r.GetC<double>()));
        num_op_def[sub_equal_] = BIN_OPERATOR(l.Get<double>() -= r.GetC<double>());
        num_op_def[add_equal_] = BIN_OPERATOR(l.Get<double>() += r.GetC<double>());
        num_def.binary_operator_overload[JsType::number] = num_op_def;
        vm.class_define[JsType::number] = num_def;
    }
}