#pragma once
#include "Value.h"
#include "Runtime.h"
#include "Array.h"
#include "Json.h"
#include "Object.h"
#include "sion.h"
#define JSON_PARSE(e) JsonNext().JsonParse(e)
#define BIN_OPERATOR(body) [](Value &l, Value &r) { return body; };
#define VM_FN(body) [&](Vector<Value> args) -> Value { body; }

namespace agumi
{
    void AddPreDefine(VM &vm)
    {
        auto o1 = Object({{"d", "hello world"}});
        auto o2 = Object({{"hello", o1}});
        auto o3 = Object({{"o3", o2}});
        vm.ctx_stack[0].var["b"] = Object({{"dd", o3}, {"cc", 1}});
        auto json_parse = VM_FN(return JSON_PARSE(args.GetOrDefault(0).ToString()));
        auto json_stringify = VM_FN(
            auto indent = args.GetOrDefault(1);
            return Json::Stringify(args.GetOrDefault(0), indent.NotUndef() ? indent.Get<double>() : 4););
        auto json_module = Object({{"parse", vm.DefineFunc(json_parse)},
                                   {"stringify", vm.DefineFunc(json_stringify)}});
        vm.ctx_stack[0].var["json"] = json_module;
        auto fetch_bind = [&](Vector<Value> args) -> Value
        {
            auto resp = sion::Fetch(args[0].ToString());
            auto res = Object();
            res["data"] = resp.Body().c_str();
            return res;
        };
        vm.DefineGlobalFunc("fetch", fetch_bind);
        vm.DefineGlobalFunc("typeof", VM_FN(return args.GetOrDefault(0).TypeString()));
        auto assert_bind =  [&](Vector<Value> args) -> Value {
            if(!args.GetOrDefault(0).ToBool())
            {
                THROW_STACK(vm.StackTrace(), "assert error")
            }
            return Value::undefined;
        };
        vm.DefineGlobalFunc("assert", assert_bind);
        auto eval = [&](Vector<Value> args) -> Value
        {
            auto script = args.GetOrDefault(0).ToString();
            auto enable_curr_vm = args.GetOrDefault(1).ToBool();
            auto tfv = GeneralTokenizer::Agumi(script);
            auto ast = Compiler().ConstructAST(tfv);
            return enable_curr_vm ? vm.Run(ast) : VM().Run(ast);
        };
        vm.DefineGlobalFunc("eval", eval);
        auto log = [&](Vector<Value> args) -> Value
        {
            auto out = args.Map<String>([](Value arg)
                                        { return arg.ToString(); })
                           .Join();
            std::cout << out << std::endl;
            return Value::undefined;
        };
        vm.DefineGlobalFunc("log", log);
        vm.DefineGlobalFunc("loadFile", VM_FN(return LoadFile(args.GetOrDefault(0).ToString())));
        auto mem_bind = [&](Vector<Value> args) -> Value
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
        auto lens_bind = [&](Vector<Value> keys) -> Value
        {
            auto fn = [=, &vm](Vector<Value> args) -> Value
            {
                auto self = args[0];
                for (size_t i = 0; i < keys.size(); i++)
                {
                    auto key = keys[i];
                    auto t = key.Type();
                    if (!(t == ValueType::number || t == ValueType::string))
                    {
                        THROW_MSG("{} 不允许用来索引", key.TypeString())
                    }
                    self = t == ValueType::number ? self[key.GetC<double>()] : self[key.ToString()];
                }
                return self;
            };
            return vm.DefineFunc(fn);
        };
        vm.DefineGlobalFunc("lens", lens_bind);
        // 定义本地类成员函数
        LocalClassDefine string_def;
        std::map<KW, std::function<Value(Value &, Value &)>> str_op_def;
        str_op_def[add_] = BIN_OPERATOR(l.GetC<String>() + r.GetC<String>());
        str_op_def[eqeq_] = BIN_OPERATOR(l.GetC<String>() == r.GetC<String>());
        str_op_def[eqeqeq_] = BIN_OPERATOR(l.GetC<String>() == r.GetC<String>());
        str_op_def[add_equal_] = BIN_OPERATOR(l.Get<String>() += r.GetC<String>());
        str_op_def[mul_] = BIN_OPERATOR(l.GetC<String>().Repeat(stoi(r.GetC<String>())));
        string_def.binary_operator_overload[ValueType::string] = str_op_def;
        string_def.member_func["length"] = [](Value &_this, Vector<Value> args) -> Value
        {
            return static_cast<int>(_this.GetC<String>().length());
        };
        vm.class_define[ValueType::string] = string_def;

        LocalClassDefine array_def;
        std::map<KW, std::function<Value(Value &, Value &)>> array_op_def;
        array_op_def[eqeq_] = BIN_OPERATOR(l.Arr().Ptr() == r.Arr().Ptr());
        array_op_def[not_eq_] = BIN_OPERATOR(l.Arr().Ptr() != r.Arr().Ptr());
        array_def.binary_operator_overload[ValueType::array] = array_op_def;
        array_def.member_func["push"] = [](Value &_this, Vector<Value> args) -> Value
        {
            for (auto &i : args)
            {
                _this.Arr().Src().push_back(i);
            }
            return _this;
        };
        array_def.member_func["get"] = [](Value &_this, Vector<Value> args) -> Value
        {
            if (args.size() < 1)
            {
                THROW
            }
            int idx = args[0].GetC<double>();
            if (idx >= _this.ArrC().SrcC().size())
            {
                THROW
            }

            return _this[idx];
        };
        vm.class_define[ValueType::array] = array_def;

        LocalClassDefine num_def;
        num_def.member_func["incr"] = [](Value &_this, Vector<Value> args) -> Value
        {
            return ++_this.Get<double>();
        };
        std::map<KW, std::function<Value(Value &, Value &)>> num_op_def;
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
        num_def.binary_operator_overload[ValueType::number] = num_op_def;
        vm.class_define[ValueType::number] = num_def;

        LocalClassDefine fn_def;
        std::map<KW, std::function<Value(Value &, Value &)>> fn_op_def;
        fn_op_def[add_] = [&](Value &l, Value &r)
        {
            auto new_fn = [=, &vm](Vector<Value> args) -> Value
            {
                return vm.FuncCall(r, vm.FuncCall(l, args));
            };
            return vm.DefineFunc(new_fn);
        };
        fn_def.binary_operator_overload[ValueType::function] = fn_op_def;
        vm.class_define[ValueType::function] = fn_def;
    }
}