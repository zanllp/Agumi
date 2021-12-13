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
        vm.DefineGlobalFunc("env", VM_FN(return Object({ {"working_dir", vm.working_dir} })));
        auto fs_exist_vm = [&](String file_name) -> bool
        {
            std::fstream file(file_name, std::ios_base::in);
            return file.good();
        };
        auto fs_exist = [&](Vector<Value> args) -> Value
        {
            std::fstream file(PathCalc(vm.working_dir, args.GetOrDefault(0).ToString()), std::ios_base::in);
            return file.good();
        };
        auto fs_write = [&](Vector<Value> args) -> Value
        {
            auto path = PathCalc(vm.working_dir, args.GetOrDefault(0).ToString());
            auto src = args.GetOrDefault(1).ToString();
            std::fstream file(path, std::ios_base::out);
            file << src;
            return file.good();
        };
        vm.ctx_stack[0].var["fs"] = Object({{"exist", vm.DefineFunc(fs_exist)},
                                            {"write", vm.DefineFunc(fs_write)},
                                            {"read", vm.DefineFunc(VM_FN(return LoadFile(PathCalc(vm.working_dir, args.GetOrDefault(0).ToString()))))}});
        auto json_parse = VM_FN(return JSON_PARSE(args.GetOrDefault(0).ToString()));
        auto json_stringify = VM_FN(
            auto indent = args.GetOrDefault(1);
            return Json::Stringify(args.GetOrDefault(0), indent.NotUndef() ? indent.Get<double>() : 4););
        auto json_module = Object({{"parse", vm.DefineFunc(json_parse)},
                                   {"stringify", vm.DefineFunc(json_stringify)}});
        vm.ctx_stack[0].var["json"] = json_module;
        auto fetch_bind = [&](Vector<Value> args) -> Value
        {
            auto url = args.GetOrDefault(0).ToString();
            auto params_i = args.GetOrDefault(1);
            auto req = sion::Request()
                           .SetUrl(url)
                           .SetHttpMethod(sion::Method::Get);
            if (params_i.NotUndef())
            {
                if (params_i.Type() != ValueType::object)
                {
                    THROW_MSG("params必须为object类型，当前为{}", params_i.TypeString())
                }
                auto method_i = params_i["method"];
                auto headers_i = params_i["headers"];
                auto data_i = params_i["data"];
                if (method_i.NotUndef())
                {
                    req.SetHttpMethod(method_i.ToString().ToUpperCase());
                }
                if (data_i.NotUndef())
                {
                    req.SetBody(json_stringify({data_i, 0}).ToString());
                }
                if (headers_i.NotUndef())
                {
                    if (headers_i.Type() != ValueType::object)
                    {
                        THROW_MSG("headers必须为object类型，当前为{}", headers_i.TypeString())
                    }
                    for (auto &i : headers_i.ObjC().SrcC())
                    {
                        req.SetHeader(i.first, i.second.ToString());
                    }
                }
            }

            auto resp = req.Send();
            auto res = Object();
            res["data"] = resp.Body();
            res["code"] = resp.Code();
            res["headers"] = Array();
            for (auto &i : resp.HeaderSrc().data)
            {
                auto obj = Object();
                obj["k"] = i.first;
                obj["v"] = i.second;
                res["headers"].Arr().Src().push_back(obj);
            }

            return res;
        };
        vm.DefineGlobalFunc("fetch", fetch_bind);
        vm.DefineGlobalFunc("runInMicroQueue", VM_FN(vm.AddTask2Queue(args.GetOrDefault(0), true); return nullptr;));
        vm.DefineGlobalFunc("runInMacroQueue", VM_FN(vm.AddTask2Queue(args.GetOrDefault(0), false); return nullptr;));
        vm.DefineGlobalFunc("format", VM_FN(
                                          std::vector<String> rest = args.Slice(1).Map<String>([](Value arg)
                                                                                               { return arg.ToString(); });
                                          return String::Format(args.GetOrDefault(0).ToString(), rest)));
        vm.DefineGlobalFunc("typeof", VM_FN(return args.GetOrDefault(0).TypeString()));
        vm.DefineGlobalFunc("path_calc", VM_FN(return PathCalc(args.Map<String>([](Value arg){ return arg.ToString();}))));
        auto assert_bind = VM_FN(
            if (!args.GetOrDefault(0).ToBool()) {
                String msg = args.GetOrDefault(1).NotUndef() ? args.GetOrDefault(1).ToString() : "assert error";
                THROW_MSG(msg)
            } return nullptr;);
        vm.DefineGlobalFunc("assert", assert_bind);
        auto parse_agumi_script_bind = VM_FN(
            auto script = args.GetOrDefault(0).ToString();
            auto tfv = GeneralTokenizer::Agumi(script);
            auto ast = Compiler().ConstructAST(tfv);
            return ast.ToJson(););
        vm.DefineGlobalFunc("parse_agumi_script", parse_agumi_script_bind);
        auto eval = [&](Vector<Value> args) -> Value
        {
            auto script = args.GetOrDefault(0).ToString();
            auto enable_curr_vm = args.GetOrDefault(1).ToBool();
            auto tfv = GeneralTokenizer::Agumi(script);
            auto ast = Compiler().ConstructAST(tfv);
            if (!enable_curr_vm) {
                VM vm;
                AddPreDefine(vm);
                return vm.Run(ast);
            };
            auto ctx_save = vm.ctx_stack;
            if (vm.ctx_stack.size() > 1)
            {
                vm.ctx_stack.resize(1);
            }
            auto r = vm.Run(ast); // 在最上层栈中执行
            for (size_t i = 1; i < ctx_save.size(); i++)
            {
                vm.ctx_stack.push_back(ctx_save[i]);
            }
            
            return r ; };
        vm.DefineGlobalFunc("eval", eval);
        vm.DefineGlobalFunc("include", [&](Vector<Value> args) -> Value {
            auto path = args.GetOrDefault(0).ToString();
            auto absolute_path = PathCalc(vm.CurrCtx().start->file, "..", path);
            if (vm.included_files.Includes(absolute_path))
            {
                return nullptr;
            }
            if (!fs_exist_vm(absolute_path))
            {
                THROW_MSG("the file '{}' is not found", absolute_path)
            }
            
            auto file = LoadFile(absolute_path);
            vm.included_files.push_back(absolute_path);
            auto tfv = GeneralTokenizer::Agumi(file, absolute_path);
            auto ast = Compiler().ConstructAST(tfv);
            return vm.Run(ast);
        });
        auto log = VM_FN(
            auto out = args.Map<String>([](Value arg)
                                        { return arg.ToString(); })
                           .Join();
            std::cout << out << std::endl;
            return nullptr;);
        vm.DefineGlobalFunc("log", log);
        auto mem_bind = VM_FN(
            size_t idx = 0;
            if (args.size()) {
                idx = args[0].Get<double>();
                if (idx > vm.ctx_stack.size())
                {
                    THROW_MSG("内存越界 参数:{} vm ctx_stack size:{}", idx, vm.ctx_stack.size())
                }
            } return vm.ctx_stack[idx]
                .var;);
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
        array_def.member_func["length"] = [](Value &_this, Vector<Value> args) -> Value
        {
            return double(_this.ArrC().SrcC().size());
        };
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
        array_def.member_func["select"] = [&](Value &_this, Vector<Value> args) -> Value
        {
            Array arr;
            auto v = args.GetOrDefault(0);
            if (v.Type() != ValueType::function)
            {
                THROW_MSG("array::select 的参数必须为function类型，当前为{}", v.TypeString())
            }

            for (auto &i : _this.Arr().Src())
            {

                arr.Src().push_back(vm.FuncCall(v, i));
            }
            return arr;
        };
        array_def.member_func["where"] = [&](Value &_this, Vector<Value> args) -> Value
        {
            Array arr;
            auto v = args.GetOrDefault(0);
            if (v.Type() != ValueType::function)
            {
                THROW_MSG("array::where 的参数必须为function类型，当前为{}", v.TypeString())
            }

            for (auto &i : _this.Arr().Src())
            {
                if (vm.FuncCall(v, i).ToBool())
                {
                    arr.Src().push_back(i);
                }
            }
            return arr;
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

        vm.DefineGlobalFunc("object_entries", [](Vector<Value> args) -> Value
                            {
            Array arr;
            for (auto& i : args.GetOrDefault(0).ObjC().SrcC()) {
                auto obj = Object();
                obj["k"] = i.first;
                obj["v"] = i.second;
                 arr.Src().push_back(obj);
            }
            return arr; });
    }
}