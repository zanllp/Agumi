#pragma once
#include "Parser.h"
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
    shared_ptr<FunctionDeclaration> src;
    bool is_native_func = false;
    function<JsValue(Vector<JsValue>)> native_fn;
};

class VM
{
public:
    VM()
    {
        ctx_stack.resize(1);
    }
    Vector<Context> ctx_stack;
    map<string, Function> func_mem;
    Context &CurrCtx()
    {
        return ctx_stack.back();
    }
    tuple<JsValue, bool> Value(String key)
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
    JsValue ValueOrUndef(String key)
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

    void DefineGlobalFunc(String name, const function<JsValue(Vector<JsValue>)> &native_fn)
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

    JsValue DefineFunc(const function<JsValue(Vector<JsValue>)> &native_fn)
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
        auto &op = expr.op;
        if (op.Is(eqeq_) || op.Is(eqeqeq_))
        {
            return left == right;
        }
        else if (op.Is(not_eq_) || op.Is(not_eqeq_))
        {
            return !(left == right);
        }

        if (type != type_r && (type == JsType::string || type == JsType::number))
        {
            ERR_ResolveBinaryExpression
        }
        if (type == JsType::number)
        {
            auto l_n = left.GetC<double>();
            auto r_n = right.GetC<double>();
            switch (expr.op.kw[0])
            {
            case '*':
                return l_n * r_n;
            case '/':
                return l_n / r_n;
            case '+':
                return l_n + r_n;
            case '-':
                return l_n - r_n;
            case '%':
                return fmod(l_n, r_n);
            }
            if (op.Is(more_than_))
            {
                return l_n > r_n;
            }
            if (op.Is(more_than_equal_))
            {
                return l_n >= r_n;
            }
            if (op.Is(less_than_))
            {
                return l_n < r_n;
            }
            if (op.Is(less_than_equal_))
            {
                return l_n <= r_n;
            }
        }
        else if (type == JsType::string && expr.op.kw[0] == '+')
        {
            return left.GetC<String>() + right.GetC<String>();
        }

        ERR_ResolveBinaryExpression
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
    JsValue ResolveObjectIndex(StatPtr stat, JsValue par = JsValue::undefined)
    {
        if (stat->Type() == StatementType::identifier) // 索引到最后一个属性
        {
            SRC_REF(key, Identifier, stat);
            return par[key.tok.kw];
        }
        if (stat->Type() == StatementType::indexStatement)
        {
            SRC_REF(idx, IndexStatement, stat);
            if (par == JsValue::undefined)
            {
                JsValue obj = ResolveExecutable(idx.object);
                return ResolveObjectIndex(idx.property, obj);
            }
            else
            {
                auto key = idx.object->tok.kw;
                auto next_par = par[key];
                return ResolveObjectIndex(idx.property, next_par);
            }
        }
        if (stat->Type() == StatementType::functionCall)
        {
            SRC_REF(fn, FunctionCall, stat);
            SRC_REF(key, Identifier, fn.id);
            auto fn_loc = par[key.tok.kw];
            return ResolveFuncCall(stat, fn_loc);
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
            return ResolveObjectIndex(stat);
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
        fn.src = static_pointer_cast<FunctionDeclaration>(stat);
        func_mem[func_id] = fn;
        return JsValue::CreateFunc(func_id);
    }
};
