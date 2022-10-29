#pragma once
#include "Array.h"
#include "JsonStringify.h"
#include "Object.h"
#include "Runtime.h"
#include "ServerBind.h"
#include "Value.h"
#include "sion/SionGlobal.h"
#include "sion/sion.h"

#define JSON_PARSE(e) JsonParse::Call(e)
#define CLONE(v) JSON_PARSE(JsonStringify::Call(v))
#define BIN_OPERATOR(body) [](Value& l, Value& r) { return body; };
#define VM_FN(body) [&](Vector<Value> args) -> Value { body; }

namespace agumi
{

Value DefineOperator(VM& vm, Vector<Value> args)
{
    auto l_cls_str = args.GetOrDefault(0).Str();
    ASSERT_T(typestr_2enum.find(l_cls_str) != typestr_2enum.end())
    auto l_cls = typestr_2enum[l_cls_str];
    auto local_class = vm.class_define.find(l_cls);
    if (local_class == vm.class_define.end())
    {
        vm.class_define[l_cls] = LocalClassDefine();
        local_class = vm.class_define.find(l_cls);
    }
    if (args.size() == 3)
    {
        auto fn = args.GetOrDefault(2);
        auto op = Token::Str2kw(args.GetOrDefault(1).Str());
        vm.class_define[l_cls].unary_func[op] = fn;
        return nullptr;
    }
    auto r_cls_str = args.GetOrDefault(1).Str();
    ASSERT_T(typestr_2enum.find(r_cls_str) != typestr_2enum.end())
    auto r_cls = typestr_2enum[r_cls_str];
    auto op = Token::Str2kw(args.GetOrDefault(2).Str());
    auto cb = args.GetOrDefault(3);
    auto& bin_op = local_class->second.binary_operator_overload;
    auto r_cls_op_def = bin_op.find(r_cls);
    if (r_cls_op_def == bin_op.end())
    {
        bin_op[r_cls] = std::map<KW, std::function<Value(Value&, Value&)>>();
        r_cls_op_def = bin_op.find(r_cls);
    }
    r_cls_op_def->second[op] = [&, cb](Value& l, Value& r) { return vm.FuncCall(cb, l, r); };
    return nullptr;
}

void AddPreDefine(VM& vm)
{
    auto& global = vm.ctx_stack[0].var;
    vm.DefineGlobalFunc("get_io_mark_color",
                        VM_FN(return Object({{"end", color_e}, {"red", color_red_s}, {"blue", color_blue_s}, {"green", color_green_s}})));
    vm.DefineGlobalFunc("env", VM_FN(return Object({{"working_dir", vm.working_dir},
                                                    {"process_arg", vm.process_arg},
                                                    {"curr_dir", vm.DefineFunc(VM_FN(return PathCalc(vm.CurrCtx().start->file, "..");))},
                                                    {"curr_file", vm.DefineFunc(VM_FN(return vm.CurrCtx().start->file;))}})));
    vm.DefineGlobalFunc("make_ability", [&](Vector<Value> args) -> Value {
        auto name = args.GetOrDefault(0);
        auto curr_key = vm.ability_define.size();
        vm.AddInitAbility();
        return Object({{"key", double(curr_key)}, {"name", name.NotUndef() ? name : "anonymous"}});
    });
    vm.DefineGlobalFunc("use_ability", [&](Vector<Value> args) -> Value {
        auto v = args.GetOrDefault(0);
        if (v.Type() != ValueType::object)
        {
            THROW_VM_STACK_MSG("use_ability only allows using in object")
        }
        auto abi = args.GetOrDefault(1);
        if (abi["key"].Number() > vm.ability_define.size())
        {
            THROW_VM_STACK_MSG("it is an invalid ability")
        }
        if (!v.Obj()[vm.ability_key].NotUndef())
        {
            v.Obj()[vm.ability_key] = Array();
        }

        v.Obj()[vm.ability_key].Arr().Src().push_back(abi);
        return v;
    });
    vm.DefineGlobalFunc("define_member_function", [&](Vector<Value> args) -> Value {
        auto target = args.GetOrDefault(0);
        auto arg2 = args.GetOrDefault(1);
        auto func = args.GetOrDefault(2);
        if (arg2.Type() == ValueType::object)
        {
            auto define_obj = arg2;
            if (target.Type() == ValueType::string)
            {
                auto type = typestr_2enum[target.ToString()];
                auto iter = vm.class_define.find(type);
                ASSERT_T(iter != vm.class_define.end())
                for (auto&& i : define_obj.ObjC().SrcC())
                {
                    auto name = i.first;
                    auto func = i.second;
                    iter->second.member_func[name] = [=, &vm](Value& _this, Vector<Value> args) -> Value {
                        args.insert(args.begin(), _this);
                        return vm.FuncCall(func, args);
                    };
                }
            }
            else
            {
                auto idx = target["key"].NumberC();
                if (idx > vm.ability_define.size())
                {
                    THROW_VM_STACK_MSG("it is an invalid ability")
                }
                auto abi = vm.ability_define[idx];
                for (auto&& i : define_obj.ObjC().SrcC())
                    for (auto&& i : define_obj.ObjC().SrcC())
                    {
                        auto name = i.first;
                        auto func = i.second;
                        abi[name] = func;
                    }
            }
        }
        else
        {
            auto name = arg2.ToString();
            if (target.Type() == ValueType::string)
            {
                auto type = typestr_2enum[target.ToString()];
                auto iter = vm.class_define.find(type);
                ASSERT_T(iter != vm.class_define.end())
                iter->second.member_func[name] = [=, &vm](Value& _this, Vector<Value> args) -> Value {
                    args.insert(args.begin(), _this);
                    return vm.FuncCall(func, args);
                };
            }
            else
            {
                auto idx = target["key"].NumberC();
                if (idx > vm.ability_define.size())
                {
                    THROW_VM_STACK_MSG("it is an invalid ability")
                }
                auto abi = vm.ability_define[idx];
                abi[name] = func;
            }
        }
        return true;
    });

    auto fs_exist_vm = [&](String file_name) -> bool {
        std::fstream file(file_name, std::ios_base::in);
        return file.good();
    };
    auto fs_exist = [&](Vector<Value> args) -> Value {
        std::fstream file(PathCalc(vm.working_dir, args.GetOrDefault(0).ToString()), std::ios_base::in);
        return file.good();
    };
    auto fs_write = [&](Vector<Value> args) -> Value {
        auto path = PathCalc(vm.working_dir, args.GetOrDefault(0).ToString());
        auto src = args.GetOrDefault(1).ToString();
        std::fstream file(path, std::ios_base::out);
        file << src;
        return file.good();
    };
    vm.ctx_stack[0].var["fs"] =
        Object({{"exist", vm.DefineFunc(fs_exist)},
                {"write", vm.DefineFunc(fs_write)},
                {"read", vm.DefineFunc(VM_FN(return LoadFile(PathCalc(vm.working_dir, args.GetOrDefault(0).ToString()))))}});
    auto json_parse = VM_FN(return JSON_PARSE(args.GetOrDefault(0).ToString()));
    vm.DefineGlobalFunc("print_call_stack", VM_FN(std::cout << vm.StackTrace() << std::endl; return nullptr;));
    auto json_stringify = VM_FN(auto indent = args.GetOrDefault(1);
                                return JsonStringify::Call(args.GetOrDefault(0), indent.NotUndef() ? indent.Number() : 4););
    auto json_module = Object({{"parse", vm.DefineFunc(json_parse)}, {"stringify", vm.DefineFunc(json_stringify)}});
    vm.ctx_stack[0].var["json"] = json_module;
    auto fetch_bind = [&](Vector<Value> args) -> Value {
        auto url = args.GetOrDefault(0).ToString();
        auto params_i = args.GetOrDefault(1);
        auto req = sion::Request().SetUrl(url).SetHttpMethod(sion::Method::Get);
        if (params_i.NotUndef())
        {
            if (params_i.Type() != ValueType::object)
            {
                THROW_VM_STACK_MSG("params必须为object类型，当前为{}", params_i.TypeString())
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
                    THROW_VM_STACK_MSG("headers必须为object类型，当前为{}", headers_i.TypeString())
                }
                for (auto& i : headers_i.ObjC().SrcC())
                {
                    req.SetHeader(i.first, i.second.ToString());
                }
            }
        }

        auto resp = req.Send();
        auto res = Object();
        res["data"] = resp.StrBody();
        res["code"] = resp.Code();
        res["headers"] = Array();
        for (auto& i : resp.GetHeader().Data())
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
    vm.DefineGlobalFunc("format", VM_FN(std::vector<String> rest = args.Slice(1).Map<String>([](Value arg) { return arg.ToString(); });
                                        return String::Format(args.GetOrDefault(0).ToString(), rest)));
    vm.DefineGlobalFunc("typeof", VM_FN(return args.GetOrDefault(0).TypeString()));
    vm.DefineGlobalFunc("path_calc", VM_FN(return PathCalc(args.Map<String>([](Value arg) { return arg.ToString(); }))));
    auto raise_native_exception_bind = VM_FN(THROW_VM_STACK_MSG(args.GetOrDefault(0).ToString()) return nullptr;);
    vm.DefineGlobalFunc("raise_native_exception", raise_native_exception_bind);
    auto parse_agumi_script_bind =
        VM_FN(auto script = args.GetOrDefault(0).ToString(); auto tfv = GeneralTokenizer::Agumi(script, args.GetOrDefault(1).ToString());
              auto ast = Compiler().ConstructAST(tfv); return ast.ToJson(););
    vm.DefineGlobalFunc("parse_agumi_script", parse_agumi_script_bind);
    auto generate_agumi_script_token_bind =
        VM_FN(auto script = args.GetOrDefault(0).ToString(); auto tfv = GeneralTokenizer::Agumi(script, args.GetOrDefault(1).ToString());
              Array arr; for (auto&& i
                              : tfv) { arr.Src().push_back(i.ToJson()); } return arr;);
    vm.DefineGlobalFunc("generate_agumi_script_token", generate_agumi_script_token_bind);
    auto eval = [&](Vector<Value> args) -> Value {
        auto script = args.GetOrDefault(0).ToString();
        auto enable_curr_vm = args.GetOrDefault(1).ToBool();
        auto tfv = GeneralTokenizer::Agumi(script);
        auto ast = Compiler().ConstructAST(tfv);
        if (!enable_curr_vm)
        {
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

        return r;
    };
    vm.DefineGlobalFunc("eval", eval);

    vm.DefineGlobalFunc("include", [&](Vector<Value> args) -> Value {
        auto path = args.GetOrDefault(0).ToString();
        auto use_working_dir_as_relative_path = args.GetOrDefault(1).ToBool();
        auto absolute_path =
            use_working_dir_as_relative_path ? PathCalc(vm.working_dir, path) : PathCalc(vm.CurrCtx().start->file, "..", path);
        if (absolute_path.find(".") == std::string::npos)
        {
            absolute_path += ".as";
        }

        if (vm.included_files.Includes(absolute_path))
        {
            return nullptr;
        }
        if (!fs_exist_vm(absolute_path))
        {
            THROW_VM_STACK_MSG("the file '{}' is not found", absolute_path)
        }

        auto file = LoadFile(absolute_path);
        vm.included_files.push_back(absolute_path);
        auto tfv = GeneralTokenizer::Agumi(file, absolute_path);
        auto ast = Compiler().ConstructAST(tfv);
        return vm.Run(ast);
    });
    auto log =
        VM_FN(auto out = args.Map<String>([](Value arg) { return arg.ToString(); }).Join(); std::cout << out << std::endl; return nullptr;);
    vm.DefineGlobalFunc("log", log);
    auto mem_bind = VM_FN(size_t idx = 0; if (args.size()) {
        idx = args[0].Number();
        if (idx > vm.ctx_stack.size())
        {
            THROW_VM_STACK_MSG("内存越界 参数:{} vm ctx_stack size:{}", idx, vm.ctx_stack.size())
        }
    } return vm.ctx_stack[idx]
                                              .var;);
    vm.DefineGlobalFunc("mem", mem_bind);
    auto lens_bind = [&](Vector<Value> keys) -> Value {
        auto fn = [=, &vm](Vector<Value> args) -> Value {
            auto self = args[0];
            for (size_t i = 0; i < keys.size(); i++)
            {
                auto key = keys[i];
                auto t = key.Type();
                if (!(t == ValueType::number || t == ValueType::string))
                {
                    THROW_VM_STACK_MSG("{} 不允许用来索引", key.TypeString())
                }
                self = t == ValueType::number ? self[key.NumberC()] : self[key.ToString()];
            }
            return self;
        };
        return vm.DefineFunc(fn);
    };
    vm.DefineGlobalFunc("lens", lens_bind);
    vm.ctx_stack[0].var["utf8"] =
        Object({{"from_code_point", vm.DefineFunc([&](Vector<Value> args) {
                     return String::FromCodePoint(args.GetOr(0, "").ToString(), args.GetOr(1, 16).Number());
                 })},
                {"decode", vm.DefineFunc([&](Vector<Value> args) { return String::FromUtf8EncodeStr(args.GetOr(0, "").ToString()); })}});
    // 定义本地类成员函数
    LocalClassDefine string_def;
    std::map<KW, std::function<Value(Value&, Value&)>> str_op_def;
    str_op_def[add_] = BIN_OPERATOR(l.StrC() + r.StrC());
    str_op_def[eqeq_] = BIN_OPERATOR(l.StrC() == r.StrC());
    str_op_def[eqeqeq_] = BIN_OPERATOR(l.StrC() == r.StrC());
    // str_op_def[add_equal_] = BIN_OPERATOR(l.Str() += r.StrC());
    str_op_def[mul_] = BIN_OPERATOR(l.StrC().Repeat(stoi(r.StrC())));
    string_def.binary_operator_overload[ValueType::string] = str_op_def;
    string_def.member_func["byte_len"] = [](Value& _this, Vector<Value> args) -> Value { return static_cast<int>(_this.StrC().length()); };
    string_def.member_func["length"] = [](Value& _this, Vector<Value> args) -> Value { return static_cast<int>(_this.StrC().Ulength()); };
    string_def.member_func["substr"] = [](Value& _this, Vector<Value> args) -> Value {
        auto start = args.GetOrDefault(0);
        auto count = args.GetOrDefault(1);
        return _this.StrC().USubStr(start.GetOr(0.0, ValueType::number), count.GetOr(-1.0, ValueType::number));
    };
    string_def.member_func["split"] = [](Value& _this, Vector<Value> args) -> Value {
        auto r = _this.StrC().Split(args.GetOr(0, "").ToString(), args.GetOrDefault(1).GetOr(-1.0, ValueType::number),
                                    args.GetOr(2, false).ToBool(), true);
        Array res;
        res.Src().insert(res.Src().begin(), r.begin(), r.end());
        return res;
    };
    string_def.member_func["byte_to_lowercase"] = [](Value& _this, Vector<Value> args) -> Value { return _this.StrC().ToLowerCase(); };
    string_def.member_func["byte_find"] = [](Value& _this, Vector<Value> args) -> Value {
        return int(_this.StrC().find(args.GetOrDefault(0).ToString(), args.GetOrDefault(1).GetOr(0.0, ValueType::number)));
    };
    string_def.member_func["trim"] = [](Value& _this, Vector<Value> args) -> Value { return _this.StrC().Trim(); };
    vm.class_define[ValueType::string] = string_def;

    LocalClassDefine array_def;
    std::map<KW, std::function<Value(Value&, Value&)>> array_op_def;
    array_op_def[eqeq_] = BIN_OPERATOR(l.Arr().Ptr() == r.Arr().Ptr());
    array_def.binary_operator_overload[ValueType::array] = array_op_def;
    array_def.member_func["length"] = [](Value& _this, Vector<Value> args) -> Value { return double(_this.ArrC().SrcC().size()); };
    array_def.member_func["resize"] = [](Value& _this, Vector<Value> args) -> Value {
        auto size = args.GetOrDefault(0);
        if (size.NotUndef())
        {
            _this.Arr().Src().resize(size.Number());
            return true;
        }
        return false;
    };
    array_def.member_func["push"] = [](Value& _this, Vector<Value> args) -> Value {
        for (auto& i : args)
        {
            _this.Arr().Src().push_back(i);
        }
        return _this;
    };
    array_def.member_func["select"] = [&](Value& _this, Vector<Value> args) -> Value {
        Array arr;
        auto v = args.GetOrDefault(0);
        auto start_raw = args.GetOrDefault(1);
        if (start_raw.NotUndef() && start_raw.Type() != ValueType::number)
        {
            THROW_VM_STACK_MSG("array::select 的第2个参数必须为number类型，当前为{}", start_raw.TypeString())
        }

        if (v.Type() != ValueType::function)
        {
            THROW_VM_STACK_MSG("array::select 的参数必须为function类型，当前为{}", v.TypeString())
        }
        auto& src = _this.Arr().Src();
        bool stop = false;
        auto stop_handler = vm.DefineFunc(VM_FN(stop = true; return true;));
        auto start = start_raw.NotUndef() ? start_raw.Number() : 0;
        for (size_t i = start; i < src.size(); i++)
        {
            if (stop)
            {
                break;
            }
            auto args = Vector<Value>::From({src[i], double(i), stop_handler});
            arr.Src().push_back(vm.FuncCall(v, args));
        }

        return arr;
    };
    vm.class_define[ValueType::array] = array_def;

    LocalClassDefine num_def;
    num_def.member_func["incr"] = [](Value& _this, Vector<Value> args) -> Value { return ++_this.Number(); };
    std::map<KW, std::function<Value(Value&, Value&)>> num_op_def;
#define BIN_OP_NUM(tk, op) num_op_def[tk] = BIN_OPERATOR(l.NumberC() op r.NumberC())
    BIN_OP_NUM(eqeq_, ==);
    BIN_OP_NUM(more_than_, >);
    BIN_OP_NUM(more_than_equal_, >=);
    BIN_OP_NUM(less_than_, <);
    BIN_OP_NUM(less_than_equal_, <=);
    BIN_OP_NUM(add_, +);
    BIN_OP_NUM(sub_, -);
    BIN_OP_NUM(mul_, *);
    BIN_OP_NUM(div_, /);
    num_op_def[mod_] = BIN_OPERATOR(fmod(l.NumberC(), r.NumberC()));
    num_op_def[sub_equal_] = BIN_OPERATOR(l.Number() -= r.NumberC());
    // num_op_def[add_equal_] = BIN_OPERATOR(l.Number() += r.NumberC());
    num_def.binary_operator_overload[ValueType::number] = num_op_def;
    vm.class_define[ValueType::number] = num_def;
    vm.DefineGlobalFunc("object_entries", [](Vector<Value> args) -> Value {
        Array arr;
        for (auto& i : args.GetOrDefault(0).ObjC().SrcC())
        {
            auto obj = Object();
            obj["k"] = i.first;
            obj["v"] = i.second;
            arr.Src().push_back(obj);
        }
        return arr;
    });

    vm.DefineGlobalFunc("set_gc", [&](Vector<Value> args) -> Value {
        auto conf = args.GetOrDefault(0);
        auto& mem = MemManger::Get();
        if (conf.In("enable"))
        {
            mem.enable_gc = conf["enable"].ToBool();
        }
        if (conf.In("step"))
        {
            mem.gc_step = conf["step"].Number();
        }
        if (conf.In("log"))
        {
            mem.gc_log = conf["log"].ToBool();
        }
        return nullptr;
    });

    vm.DefineGlobalFunc("delete", [&](Vector<Value> args) -> Value {
        args.GetOrDefault(0).Obj().Src().erase(args.GetOrDefault(1).ToString());
        return args.GetOrDefault(0);
    });
    vm.DefineGlobalFunc("to_str", [&](Vector<Value> args) -> Value { return args.GetOrDefault(0).ToString(); });
    vm.DefineGlobalFunc("or", [&](Vector<Value> args) -> Value { return args.GetOrDefault(0).ToBool() || args.GetOrDefault(1).ToBool(); });
    vm.DefineGlobalFunc("gc", [&](Vector<Value> args) -> Value {
        MemManger::Get().GC();
        return nullptr;
    });
    vm.DefineGlobalFunc("start_timer", [&](Vector<Value> args) -> Value {
        return vm.StartTimer(args.GetOrDefault(0), args.GetOr(1, 1000.0).Number(), args.GetOrDefault(2).ToBool());
    });

    vm.DefineGlobalFunc("time", [&](Vector<Value> args) -> Value {
        return double(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    });

    vm.DefineGlobalFunc("remove_timer", [&](Vector<Value> args) -> Value {
        vm.RemoveTimer(args.GetOrDefault(0).GetOr(-1.0, ValueType::number));
        return nullptr;
    });
    vm.DefineGlobalFunc("sys_call", [&](Vector<Value> args) -> Value { return system(args.GetOrDefault(0).ToString().c_str()); });
    vm.DefineGlobalFunc("apply",
                        [&](Vector<Value> args) -> Value { return vm.FuncCall(args.GetOrDefault(0), args.GetOrDefault(1).ArrC().SrcC()); });
    vm.DefineGlobalFunc("fetch_async", [&](Vector<Value> args) -> Value {
        auto url = args.GetOrDefault(0).ToString();
        auto cb = args.GetOrDefault(2);
        auto params_i = args.GetOrDefault(1);
        if (params_i.Type() != ValueType::object)
        {
            THROW_VM_STACK_MSG("params必须为object类型，当前为{}", params_i.TypeString())
        }
        static int id = 0;
        String event_name = String::Format("fetch_async:{}", ++id);
        vm.AddRequiredEventCustomer(event_name, [&, cb](RequiredEvent e) { vm.FuncCall(cb, e.val); });
        SionGlobal::async_thread_pool.Run(
            [&, url, params_i] {
                auto req = sion::Request().SetUrl(url);
                auto p_i = params_i;
                auto method_i = p_i["method"];
                auto headers_i = p_i["headers"];
                auto data_i = p_i["data"];
                if (method_i.NotUndef())
                {
                    req.SetHttpMethod(method_i.ToString().ToUpperCase());
                }
                if (data_i.NotUndef())
                {
                    req.SetBody(data_i.ToString());
                }
                if (headers_i.NotUndef() && headers_i.Type() == ValueType::object)
                {
                    for (auto& i : headers_i.ObjC().SrcC())
                    {
                        req.SetHeader(i.first, i.second.ToString());
                    }
                }
                return req;
            },
            [&, event_name](sion::AsyncResponse async_resp) {
                auto resp = async_resp.resp;
                auto res = Object();
                if (async_resp.err_msg.length())
                {
                    res["err_msg"] = async_resp.err_msg;
                }
                else
                {
                    res["data"] = resp.StrBody();
                    res["code"] = resp.Code();
                    res["headers"] = Array();
                    for (auto& i : resp.GetHeader().Data())
                    {
                        auto obj = Object();
                        obj["k"] = i.first;
                        obj["v"] = i.second;
                        res["headers"].Arr().Src().push_back(obj);
                    }
                }
                vm.Push2RequiredEventPendingQueue(RequiredEvent(event_name, res));
            });
        // vm.fetch_async_pkg_cb[id] = cb;

        return nullptr;
    });

    vm.class_define[ValueType::object] = LocalClassDefine();
    vm.class_define[ValueType::boolean] = LocalClassDefine();
    ServerBind(vm);
    vm.DefineGlobalFunc("define_operator", VM_FN_BIND(DefineOperator));
    auto libPath = PathCalc(__FILE__, "../../script/lib/index.as");
    P("lib path: {}", libPath)
    vm.FuncCall(vm.GlobalVal("include"), {libPath, true});
}
} // namespace agumi
