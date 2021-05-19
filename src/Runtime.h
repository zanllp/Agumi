#pragma once
#include "Parser.h"
namespace agumi
{
    class Context
    {
    public:
        JsValue var = JsObject();
        Context(){};
        ~Context(){};
    };

    class Function
    {
    public:
        std::shared_ptr<FunctionDeclaration> src;
        bool is_native_func = false;
        std::function<JsValue(Vector<JsValue>)> native_fn;
    };

    class LocalClassDefine
    {
    public:
        std::map<String, std::function<JsValue(JsValue &, Vector<JsValue>)>> member_func;
        std::map<JsType, std::map<KW, std::function<JsValue(JsValue &, JsValue &)>>> binary_operator_overload;
        JsValue ExecFunc(String key, JsValue &val, Vector<JsValue> args)
        {
            auto iter = member_func.find(key);
            if (iter == member_func.end())
            {
                THROW_MSG("{} is not a function", key)
            }
            return iter->second(val, args);
        }
        JsValue ExecBinaryOperator(JsType t, KW op, JsValue &l, JsValue &r)
        {
            auto type_def = binary_operator_overload.find(t);
            if (type_def == binary_operator_overload.end())
            {
                String t_str = jstype_emun2str[static_cast<int>(t)];
                THROW_MSG("{} is not defined", t_str)
            }
            auto op_def = type_def->second.find(op);
            if (op_def == type_def->second.end())
            {
                THROW_MSG("{} is not defined", Token::Kw2Str(op))
            }
            return op_def->second(l, r);
        }
    };

    class VM
    {
    public:
        VM()
        {
            ctx_stack.resize(1);
        }
        Vector<Context> ctx_stack;
        Vector<JsValue> temp_stack;
        std::map<String, Function> func_mem;
        std::map<JsType, LocalClassDefine> class_define;
        Context &CurrCtx()
        {
            return ctx_stack.back();
        }
        std::tuple<JsValue, bool> Value(String key)
        {
            for (int i = ctx_stack.size() - 1; i >= 0; i--)
            {
                auto &ctx = ctx_stack[i];
                if (ctx.var.In(key))
                {
                    return {ctx.var[key], true};
                }
            }
            return {JsValue::undefined, false};
        }
        JsValue &ValueOrUndef(String key)
        {
            for (size_t i = 0; i < ctx_stack.size(); i++)
            {
                auto &ctx = ctx_stack[i];
                if (ctx.var.In(key))
                {
                    return ctx.var[key];
                }
            }
            return JsValue::undefined;
        }
        JsValue SetValue(String key, JsValue val)
        {
            for (size_t i = 0; i < ctx_stack.size(); i++)
            {
                auto &ctx = ctx_stack[i];
                if (ctx.var.In(key))
                {
                    return ctx.var[key] = val;
                }
            }
            THROW_MSG("{} is not defined", key)
        }
        JsValue Run(Program prog)
        {
            JsValue v;
            for (auto &stat : prog.body)
            {
                v = Dispatch(stat);
            }
            return v;
        }

        void DefineGlobalFunc(String name, const std::function<JsValue(Vector<JsValue>)> &native_fn)
        {
            static int id = 0;
            auto fn_unique_id = String::Format("native global func code:{}", ++id);
            auto fn = JsValue::CreateFunc(fn_unique_id);
            ctx_stack[0].var[name] = fn;
            Function fn_src;
            fn_src.is_native_func = true;
            fn_src.native_fn = native_fn;
            func_mem[fn_unique_id] = fn_src;
        }

        JsValue DefineFunc(const std::function<JsValue(Vector<JsValue>)> &native_fn)
        {
            static int id = 0;
            auto fn_unique_id = String::Format("native func code:{}", ++id);
            auto fn = JsValue::CreateFunc(fn_unique_id);
            Function fn_src;
            fn_src.is_native_func = true;
            fn_src.native_fn = native_fn;
            func_mem[fn_unique_id] = fn_src;
            return fn;
        }

    private:
        JsValue Dispatch(StatPtr stat)
        {
            switch (stat->Type())
            {
            case StatementType::variableDeclaration:
                return ResolveVariableDeclaration(stat);
            case StatementType::assigmentStatement:
                return ResolveAssigmentStat(stat);
            case StatementType::functionCall:
            case StatementType::binaryExpression:
            case StatementType::identifier:
            case StatementType::conditionExpression:
            case StatementType::boolLiteralInit:
            case StatementType::numberLiteralInit:
            case StatementType::stringLiteralInit:
            case StatementType::functionDeclaration:
            case StatementType::indexStatement:
            case StatementType::arrayInit:
                return ResolveExecutable(stat);
            }
            THROW_MSG("未定义类型:{}", (int)stat->Type())
        }
        JsValue ResolveFuncCall(StatPtr stat, JsValue fn_loc_optional = JsValue::undefined)
        {
            SRC_REF(fn_call, FunctionCall, stat)
            auto fn_loc = fn_loc_optional.Type() == JsType::function ? fn_loc_optional : ResolveExecutable(fn_call.id);
            if (fn_loc.Type() != JsType::function)
            {
                THROW_MSG("'{}' is not a function", fn_loc.ToString())
            }
            auto fn_iter = func_mem.find(fn_loc.GetC<String>());
            if (fn_iter == func_mem.end())
            {
                THROW_MSG("function {} is not defined", fn_loc)
            }
            Context fn_ctx;
            JsValue v;
            auto &fn = fn_iter->second;
            if (fn.is_native_func)
            {
                Vector<JsValue> args;
                for (size_t i = 0; i < fn_call.arguments.size(); i++)
                {
                    auto incoming_val = ResolveExecutable(fn_call.arguments[i]);
                    args.push_back(incoming_val);
                }
                v = fn.native_fn(args);
            }
            else
            {
                auto &src_args = fn.src->arguments;
                if (src_args.size() != fn_call.arguments.size())
                {
                    THROW_MSG("传入参数数量错误 需要：{} 实际：{}", src_args.size(), fn_call.arguments.size())
                }
                for (size_t i = 0; i < src_args.size(); i++)
                {
                    auto arg = src_args[i];
                    auto key = arg.name.kw;
                    auto incoming_val = ResolveExecutable(fn_call.arguments[i]);
                    fn_ctx.var[key] = incoming_val;
                }
                ctx_stack.push_back(fn_ctx);
                // 执行函数
                for (auto &stat : fn.src->body)
                {
                    v = Dispatch(stat);
                }
                ctx_stack.pop_back();
            }

            return v;
        }
        JsValue ResolveLocalClassFuncCall(StatPtr stat, JsType t, String key, JsValue &val)
        {
            SRC_REF(fn_call, FunctionCall, stat)
            Vector<JsValue> args;
            for (size_t i = 0; i < fn_call.arguments.size(); i++)
            {
                auto incoming_val = ResolveExecutable(fn_call.arguments[i]);
                args.push_back(incoming_val);
            }
            auto class_iter = class_define.find(t);
            if (class_iter == class_define.end())
            {
                THROW_MSG("class {} is not a defined", jstype_emun2str[(int)t])
            }
            return class_iter->second.ExecFunc(key, val, args);
        }
        JsValue ResolveVariableDeclaration(StatPtr stat)
        {
            SRC_REF(decl, VariableDeclaration, stat)
            auto &ctx = CurrCtx();
            for (auto &i : decl.declarations)
            {
                auto key = i->id.tok.kw;
                if (ctx.var.In(key))
                {
                    THROW_MSG("Identifier '{}' has already been declared", key)
                }
                if (i->type.Is(const_) && !i->initialed)
                {
                    THROW_MSG("Missing initializer in const declaration")
                }
                auto val = i->initialed ? ResolveExecutable(i->init) : JsValue::undefined;
                return ctx.var[key] = val;
            }
        }
        JsValue ResolveConditionExpression(StatPtr stat)
        {
            SRC_REF(expr, ConditionExpression, stat);
            auto cond = ResolveExecutable(expr.cond).ToBool();
            if (cond)
            {
                return ResolveExecutable(expr.left);
            }
            return ResolveExecutable(expr.right);
        }
        JsValue ResolveAssigmentStat(StatPtr stat)
        {
            SRC_REF(assig, AssigmentStatement, stat);
            auto id = assig.id.kw;
            auto val = ResolveExecutable(assig.value);
            return SetValue(id, val);
        }
        JsValue ResolveIdentifier(StatPtr stat)
        {
            SRC_REF(id, Identifier, stat);
            auto [val, exist] = Value(id.tok.kw);
            if (!exist)
            {
                THROW_MSG("{} is not defined", id.tok.kw)
            }
            return val;
        }
        JsValue ResolveBinaryExpression(StatPtr stat)
        {
            SRC_REF(expr, BinaryExpression, stat);
            auto left = ResolveExecutable(expr.left);
            auto right = ResolveExecutable(expr.right);
            auto type = left.Type();
            auto type_r = right.Type();
#define ERR_ResolveBinaryExpression THROW_MSG("type:{} {} type:{} is not defined", jstype_emun2str[(int)type], expr.op.kw, jstype_emun2str[(int)type_r])
            auto left_type_def = class_define.find(type);
            if (left_type_def == class_define.end())
            {
                ERR_ResolveBinaryExpression
            }
            auto r_type_def_set = left_type_def->second.binary_operator_overload;
            auto target_type_def = r_type_def_set.find(type_r);
            if (target_type_def == r_type_def_set.end())
            {
                ERR_ResolveBinaryExpression
            }
            auto op = expr.op.ToKwEnum();
            auto target_op_def = target_type_def->second.find(op);
            return target_op_def->second(left, right);
        }
        JsValue ResolveBoolLiteralInit(StatPtr stat)
        {
            SRC_REF(init, BoolLiteralInit, stat);
            return init.tok.kw == "true";
        }
        JsValue ResolveNumberLiteralInit(StatPtr stat)
        {
            SRC_REF(init, NumberLiteralInit, stat);
            return stod(init.tok.kw);
        }
        JsValue ResolveStringLiteralInit(StatPtr stat)
        {
            SRC_REF(init, StringLiteralInit, stat);
            // auto& ctx  = CurrCtx();
            return init.tok.toStringContent();
        }
        JsValue ResolveObjectIndex(StatPtr stat, JsValue &par, bool is_literal = false)
        {
            if (stat->Type() == StatementType::identifier) // 索引到最后一个属性
            {
                SRC_REF(key, Identifier, stat);
                if (par.Type() == JsType::object)
                {
                    return par[key.tok.kw];
                }
                else
                {
                    // todo 类成员set get
                }
            }
            if (stat->Type() == StatementType::indexStatement)
            {
                SRC_REF(idx, IndexStatement, stat);
                if (par == JsValue::undefined)
                {
                    auto obj_p = idx.object;
                    auto obj_is_idx = obj_p->Type() == StatementType::identifier;
                    if (obj_is_idx)
                    {
                        JsValue &obj = ValueOrUndef(obj_p->tok.kw);
                        return ResolveObjectIndex(idx.property, obj);
                    }
                    else
                    {
                        auto v = ResolveExecutable(obj_p); // 可能是字面量
                        temp_stack.push_back(v); //  压入临时栈
                        return ResolveObjectIndex(idx.property, temp_stack.back(), true); // 持有刚才值的引用，并标明是一个字面量
                    }
                }
                else
                {
                    auto key = idx.object->tok.kw;
                    if (par.Type() == JsType::object)
                    {
                        auto &next_par = par[key];
                        return ResolveObjectIndex(idx.property, next_par);
                    }
                    // todo 类成员set get
                }
            }
            if (stat->Type() == StatementType::functionCall)
            {
                SRC_REF(fn, FunctionCall, stat);
                SRC_REF(key, Identifier, fn.id);
                auto t = par.Type();
                auto key_str = key.tok.kw;
                if (t == JsType::object)
                {
                    return ResolveFuncCall(stat, par[key_str]);
                }
                auto v = ResolveLocalClassFuncCall(stat, t, key_str, par);
                if (is_literal) 
                {
                    temp_stack.pop_back(); // 释放临时的字面量
                }
                return v;
            }
            THROW
        }

        JsValue ResolveArrayInit(StatPtr stat)
        {
            SRC_REF(arr, ArrayInit, stat)
            JsArray arr_src;
            for (auto &i : arr.src)
            {
                arr_src.Src().push_back(ResolveExecutable(i));
            }
            return arr_src;
        }

        JsValue ResolveExecutable(StatPtr stat)
        {
            switch (stat->Type())
            {
            case StatementType::binaryExpression:
                return ResolveBinaryExpression(stat);
            case StatementType::identifier:
                return ResolveIdentifier(stat);
            case StatementType::conditionExpression:
                return ResolveConditionExpression(stat);
            case StatementType::boolLiteralInit:
                return ResolveBoolLiteralInit(stat);
            case StatementType::numberLiteralInit:
                return ResolveNumberLiteralInit(stat);
            case StatementType::stringLiteralInit:
                return ResolveStringLiteralInit(stat);
            case StatementType::functionDeclaration:
                return ResolveFuncDeclear(stat);
            case StatementType::functionCall:
                return ResolveFuncCall(stat);
            case StatementType::indexStatement:
                return ResolveObjectIndex(stat, JsValue::undefined);
            case StatementType::arrayInit:
                return ResolveArrayInit(stat);
            }
            THROW_MSG("未定义类型:{}", (int)stat->Type())
        }
        JsValue ResolveFuncDeclear(StatPtr stat)
        {
            SRC_REF(fn_stat, FunctionDeclaration, stat);
            auto func_id = fn_stat.start.UniqId();
            Function fn;
            fn.src = std::static_pointer_cast<FunctionDeclaration>(stat);
            func_mem[func_id] = fn;
            return JsValue::CreateFunc(func_id);
        }
    };
}