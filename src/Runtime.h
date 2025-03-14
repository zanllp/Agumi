#pragma once
#include "Event.h"
#include "Parser.h"
#include "sion/SionGlobal.h"

// msg, {__VA_ARGS__}
namespace agumi
{
using TimePoint = std::chrono::steady_clock::time_point;
class Closure
{
  public:
    Value val;
    bool initialed = false;
    // 0是当前，1是前一个
    int stack_offset;
    String kw;
    Closure(){};
    Closure(int offset, String _kw)
    {
        kw = _kw;
        stack_offset = offset;
        initialed = false;
    };
    ~Closure(){};

    static Closure From(Value v)
    {
        Closure c;
        c.val = v;
        c.initialed = true;
        return c;
    }
};

class Function
{
  public:
    Function() {}
    Function(String _id) { id = _id; }
    String id = "unnamed";
    std::shared_ptr<FunctionDeclaration> src;
    bool is_native_func = false;
    std::function<Value(Vector<Value>)> native_fn;
    std::map<String, Closure> closure;
    bool enable_closure = false;
};

class Context
{
  public:
    Value var = Object();
    std::map<String, Closure>* closeure = nullptr;
    Token* start = nullptr;
    Context(){};
    ~Context(){};
};

class LocalClassDefine
{
  public:
    std::map<String, std::function<Value(Value&, Vector<Value>)>> member_func;
    std::map<ValueType, std::map<KW, std::function<Value(Value&, Value&)>>> binary_operator_overload;
    std::map<KW, Value> unary_func;
    Value ExecFunc(String key, Value& val, Vector<Value> args)
    {
        auto iter = member_func.find(key);
        if (iter == member_func.end())
        {
            throw std::range_error(String::Format("{} is not a member function of {} class", key, val.TypeString()));
        }
        return iter->second(val, args);
    }
};

struct ClosureMemory
{
    std::map<String, ClosureMemory> children;
    std::map<String, Closure> map;
};

class TimerPackage
{
    TimePoint last_call;
    TimePoint last_poll;
    std::chrono::milliseconds interval;

  public:
    Value func;
    bool CanCall() { return std::chrono::duration_cast<std::chrono::milliseconds>(Now() - last_call) > interval; }
    bool CanImmediatlyPoll() { return std::chrono::duration_cast<std::chrono::milliseconds>(Now() - last_poll) > min_interval; }
    void UpdateCallTime() { last_call = Now(); }

    void UpdatePollTime() { last_poll = Now(); }
    void SetInterval(int ms) { interval = std::chrono::milliseconds(ms); }
    static TimePoint Now() { return std::chrono::steady_clock::now(); }
    static constexpr std::chrono::milliseconds min_interval = std::chrono::milliseconds(5);
    static constexpr std::chrono::milliseconds sleep_time = std::chrono::milliseconds(5);
    static constexpr std::chrono::milliseconds long_sleep_time = std::chrono::milliseconds(1000);
};

class VM
{
  private:
    bool curr_scope_return = false;

  public:
    VM()
    {
        ctx_stack.resize(1);
        id = ++incr_id;
        MemManger::Get().gc_root[String::Format("vm:{}", id)] = CurrCtx().var;
    }
    int id = 0;
    static int incr_id;
    String working_dir = '.';
    Vector<Context> ctx_stack;
    std::queue<Value> micro_task_queue;
    std::queue<Value> macro_task_queue;
    std::mutex event_required_queue_mutex;
    std::mutex cross_thread_mutex;
    std::mutex channel_mutex;
    std::queue<RequiredEvent> event_required_queue;
    std::queue<CrossThreadCallBack> event_cross_thread_queue;
    std::map<double, Vector<ChannelPayload>> sub_thread_channel;
    std::map<uint32_t, Value> fetch_async_pkg_cb;
    std::map<String, std::function<void(RequiredEvent)>> required_event_customers;
    int required_event_timer_id = -1;
    Vector<String> included_files;
    std::map<String, Function> func_mem;
    Vector<Value> ability_define;
    Value process_arg;
    const String ability_key = "#ability";
    const String next_stack_key = "#next-stack-key";
    std::map<int, TimerPackage> timer_map;
    std::map<ValueType, LocalClassDefine> class_define;
    Context& CurrCtx() { return ctx_stack.back(); }
    Value& CurrScope() { return CurrCtx().var; }
    std::optional<std::reference_wrapper<Value>> GetValue(String key, Vector<Context>& ctx_stack_, int& i)
    {
        for (; i >= 0; i--)
        {
            auto& ctx = ctx_stack_[i];
            if (ctx.var.In(key))
            {
                return {ctx.var[key]};
            }
        }
        return {};
    }
    void AddInitAbility()
    {
        Object obj;
        ability_define.push_back(obj);
        auto ability_gc_key = ability_key + "__gc";
        auto mem = MemManger::Get().Closure();
        if (!mem.In(ability_gc_key))
        {
            mem[ability_gc_key] = Array();
        }
        mem[ability_gc_key].Arr().Src().push_back(obj);
    }

    Value StartTimer(Value fn, int interval_ms, bool once)
    {
        static int id = 0;
        auto curr_id = ++id;
        TimerPackage tp;

        tp.func = DefineFunc(
            [&, curr_id, once, fn](Vector<Value>)
            {
                if (timer_map.find(curr_id) == timer_map.end())
                {
                    return false;
                }
                auto& tp = timer_map[curr_id];
                if (tp.CanCall())
                {
                    tp.UpdateCallTime();
                    // P("call {}", fn.ToString())
                    FuncCall(fn);
                    if (once)
                    {
                        RemoveTimer(curr_id);
                        return false;
                    }
                    if (timer_map.find(curr_id) != timer_map.end())
                    {
                        AddTask2Queue(tp.func, false);
                    }
                }
                else
                {
                    if (tp.CanImmediatlyPoll())
                    {
                        // P("poll")
                        tp.UpdatePollTime();
                    }
                    else
                    {
                        // P("sleep")
                        tp.UpdatePollTime();
                        std::this_thread::sleep_for(TimerPackage::sleep_time);
                    }
                    AddTask2Queue(tp.func, false);
                }
            });
        tp.SetInterval(interval_ms);
        tp.UpdateCallTime();
        tp.UpdatePollTime();
        timer_map[curr_id] = tp;
        AddTask2Queue(tp.func, false);
        return curr_id;
    }
    void RemoveTimer(int id) { timer_map.erase(id); }
    void AddRequiredEventCustomer(String event_name, std::function<void(RequiredEvent)> callback)
    {
        required_event_customers[event_name] = callback;
        if (required_event_timer_id == -1)
        {
            auto fn = DefineFunc(
                [&](Vector<Value>)
                {
                    RequiredEvent event;
                    std::function<void(RequiredEvent)> cb;
                    {
                        std::lock_guard<std::mutex> m(event_required_queue_mutex);
                        if (event_required_queue.size() == 0)
                        {
                            return false;
                        }
                        event = event_required_queue.front();
                        event_required_queue.pop();
                        auto iter = required_event_customers.find(event.event_name);
                        if (iter == required_event_customers.end())
                        {
                            return false;
                        }
                        cb = iter->second;
                        required_event_customers.erase(event.event_name);
                        if (required_event_customers.size() == 0)
                        {
                            RemoveTimer(required_event_timer_id);
                            required_event_timer_id = -1;
                        }
                    }
                    cb(event);
                },
                "RequiredEventTimerPollFunc");
            // P("define RequiredEventTimerPollFunc")
            required_event_timer_id = StartTimer(fn, 0, false).Number();
        }
    }

    void Push2RequiredEventPendingQueue(RequiredEvent event)
    {
        // P("Push2RequiredEventPendingQueue")
        std::lock_guard<std::mutex> m(event_required_queue_mutex);
        event_required_queue.push(event);
    }

    void Push2CrossThreadEventPendingQueue(CrossThreadCallBack cb)
    {
        // P("Push2CrossThreadEventPendingQueue")
        std::lock_guard<std::mutex> m(event_required_queue_mutex);
        event_cross_thread_queue.push(cb);
    }

    void ChannelPublish(double tid, ChannelPayload payload)
    {
        // P("ChannelPublish")
        std::lock_guard<std::mutex> m(channel_mutex);
        if (sub_thread_channel.find(tid) == sub_thread_channel.end())
        {
            sub_thread_channel[tid] = {};
        }
        sub_thread_channel[tid].push_back(payload);
    }

    std::optional<std::reference_wrapper<Value>> GetValue(String key)
    {
        auto clos = CurrCtx().closeure;
        if (clos != nullptr)
        {
            auto iter = clos->find(key);
            if (iter != clos->end() && iter->second.stack_offset != 0)
            {
                ASSERT_T(iter->second.initialed)
                return {iter->second.val};
            }
        }
        int i = ctx_stack.size() - 1;
        return GetValue(key, ctx_stack, i);
    }
    std::optional<std::reference_wrapper<Value>> GetValue(Closure c)
    {
        if (c.initialed)
        {
            return {c.val};
        }
        auto stack_idx = ctx_stack.size() - 1 - c.stack_offset;
        auto& scope = ctx_stack[stack_idx].var;
        if (!scope.In(c.kw))
        {
            THROW_STACK_MSG("[dev only] token {} is not found", c.kw)
        }
        return {scope[c.kw]};
    }
    Value& ValueOrUndef(String key) { return GetValue(key).value_or(Value::null); }

    Value& GlobalVal(String key)
    {
        auto& mem = ctx_stack[0].var;
        return mem.In(key) ? mem[key] : Value::null;
    }
    Value SetValue(String key, Value val)
    {
        for (size_t i = 0; i < ctx_stack.size(); i++)
        {
            auto& ctx = ctx_stack[i];
            if (ctx.var.In(key))
            {
                return ctx.var[key] = val;
            }
        }
        THROW_STACK_MSG("{} is not defined", key)
    }
    Value Run(Program prog)
    {
        Value v;
        for (auto& stat : prog.body)
        {
            v = Dispatch(stat);
        }
        RunQueueTaskUntilEmpty();
        return v;
    }

    void DefineGlobalFunc(String name, const std::function<Value(Vector<Value>)>& native_fn)
    {
        ctx_stack[0].var[name] = DefineFunc(native_fn);
    }

    Value DefineFunc(const std::function<Value(Vector<Value>)>& native_fn, String name = "")
    {
        // P("DefineFunc")
        static int id = 0;
        auto fn_unique_id = String::Format("native func code:{}{}", ++id, name.size() ? String::Format("  {}", name) : name);
        auto fn = Value::CreateFunc(fn_unique_id);
        Function fn_src(fn_unique_id);
        fn_src.is_native_func = true;
        fn_src.native_fn = native_fn;
        func_mem[fn_unique_id] = fn_src;
        return fn;
    }

    void AddTask2Queue(Value task, bool is_micro)
    {
        if (task.Type() != ValueType::function)
        {
            THROW_STACK_MSG("task 只能为函数,当前为{}", task.TypeString())
        }
        auto& q = is_micro ? micro_task_queue : macro_task_queue;
        q.push(task);
    }

    void RunQueueTaskUntilEmpty()
    {
        auto& mem = MemManger::Get();
        while (micro_task_queue.size() + macro_task_queue.size())
        {
            if (mem.enable_gc)
            {
                if (mem.last_gc + mem.gc_step < MemAllocCollect::size())
                {
                    mem.GC();
                    mem.last_gc = MemAllocCollect::size();
                }
            }
            {
                std::lock_guard<std::mutex> m(cross_thread_mutex);
                while (event_cross_thread_queue.size())
                {
                    auto cb = event_cross_thread_queue.front();
                    event_cross_thread_queue.pop();
                    FuncCall(cb.cb, cb.event.val);
                }
            }
            while (micro_task_queue.size())
            {
                FuncCall(micro_task_queue.front());
                micro_task_queue.pop();
            }
            if (macro_task_queue.size())
            {
                FuncCall(macro_task_queue.front());
                macro_task_queue.pop();
            }
        }
    }

    String StackTrace()
    {
        String res;
        for (int i = ctx_stack.size() - 1; i >= 0; i--)
        {
            auto& ctx = ctx_stack[i];
            if (!ctx.start)
            {
                continue;
            }
            res += String::Format("\tat {} -- {} \n", i, ctx.start->ToPosStr());
        }
        return res;
    }

    template <typename... Ts> Value FuncCall(Value loc, Ts... args) { return FuncCall(loc, Vector<Value>::From({args...})); }
    /**
     * @brief 在c++调用agumi函数
     *
     * @param loc
     * @param args
     * @return Value
     */
    Value FuncCall(Value loc, Vector<Value> args)
    {
        auto fn_iter = func_mem.find(loc.StrC());
        if (fn_iter == func_mem.end())
        {
            THROW_STACK_MSG("function {} is not defined", loc.ToString())
        }
        Context fn_ctx;
        Value v;
        auto& fn = fn_iter->second;
        fn_ctx.closeure = &fn.closure;
        if (fn.is_native_func)
        {
            v = fn.native_fn(args);
        }
        else
        {
            auto& src_args = fn.src->arguments;
            for (size_t i = 0; i < src_args.size(); i++)
            {
                auto arg = src_args[i];
                auto key = arg.name.kw;
                if (i < args.size())
                {
                    fn_ctx.var[key] = args[i];
                }
                else
                {
                    if (!arg.initialed)
                    {
                        THROW_STACK_MSG(
                            "function call argument lost at pos: {} (name: {}) and no default value\n function is defined on the {}", i,
                            key, fn.src->start.ToPosStr())
                    }
                    fn_ctx.var[key] = Dispatch(arg.init);
                }
            }
            PushContext(fn_ctx);
            for (auto& stat : fn.src->body)
            {
                v = Dispatch(stat);
                if (curr_scope_return)
                {
                    curr_scope_return = false;
                    break;
                }
            }
            PopContext();
        }
        return v;
    }

  private:
    void PushContext(Context fn_ctx)
    {
        CurrScope()[next_stack_key] = fn_ctx.var; // 产生gc关联
        ctx_stack.push_back(fn_ctx);
    }

    void PopContext()
    {
        ctx_stack.pop_back();
        CurrScope().Obj().Src().erase(next_stack_key);
    }

    Value Dispatch(StatPtr stat)
    {
        CurrCtx().start = &stat->start;
        switch (stat->Type())
        {
        case StatementType::variableDeclaration:
            return ResolveVariableDeclaration(stat);
        case StatementType::assigmentStatement:
            return ResolveAssigmentStat(stat);
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
        case StatementType::objectInit:
            return ResolveObjectInit(stat);
        case StatementType::nullLiteral:
            return nullptr;
        case StatementType::unaryExpression:
            return ResolveUnaryExpr(stat);
        case StatementType::blockStatment:
            return ResolveBlock(stat);
        case StatementType::returnSatement:
            return ResolveReturn(stat);
        case StatementType::matchStatment:
            return ResolvePatternMatch(stat);
        }
        THROW_STACK_MSG("未定义类型:{}", (int)stat->Type())
    }
    /**
     * @brief 解决函数调用语句
     *
     * @param stat
     * @param fn_loc_optional
     * @param extra_args 塞一下参数到最前面，例如this
     * @return Value
     */
    Value ResolveFuncCall(StatPtr stat, Value fn_loc_optional = nullptr, Vector<Value> extra_args = {})
    {
        SRC_REF(fn_call, FunctionCall, stat)
        auto is_use_optional = fn_loc_optional.Type() == ValueType::function;
        auto fn_loc = is_use_optional ? fn_loc_optional : Dispatch(fn_call.id);
        if (fn_loc.Type() != ValueType::function)
        {
            auto msg = String::Format("'{}' is not a function", fn_loc.ToString());
            THROW_STACK_MSG(is_use_optional ? "{}" : "{} key:{}", msg, fn_call.id->start.kw)
        }
        auto fn_iter = func_mem.find(fn_loc.StrC());
        if (fn_iter == func_mem.end())
        {
            THROW_STACK_MSG("function {} is not defined", fn_loc.ToString())
        }
        Context fn_ctx;
        Value v;
        auto& fn = fn_iter->second;
        fn_ctx.closeure = &fn.closure;
        if (fn.is_native_func)
        {
            Vector<Value> args;
            for (size_t i = 0; i < fn_call.arguments.size(); i++)
            {
                auto incoming_val = Dispatch(fn_call.arguments[i]);
                args.push_back(incoming_val);
            }
            v = fn.native_fn(args);
        }
        else
        {
            auto& src_args = fn.src->arguments;
            Vector<Value> args;
            args.insert(args.begin(), extra_args.begin(), extra_args.end());
            for (auto&& i : fn_call.arguments)
            {
                args.push_back(Dispatch(i));
            }

            PushContext(fn_ctx);
            for (size_t i = 0; i < src_args.size(); i++)
            {
                auto arg = src_args[i];
                auto key = arg.name.kw;
                if (i < args.size())
                {
                    fn_ctx.var[key] = args[i];
                }
                else
                {
                    if (!arg.initialed)
                    {
                        THROW_STACK_MSG(
                            "function call argument lost at pos: {} (name: {}) and no default value\n function is defined on the {}", i,
                            key, fn.src->start.ToPosStr())
                    }
                    fn_ctx.var[key] = Dispatch(arg.init);
                }
            }
            // 执行函数
            for (auto& stat : fn.src->body)
            {
                v = Dispatch(stat);
                if (curr_scope_return)
                {
                    curr_scope_return = false;
                    break;
                }
            }
            PopContext();
        }

        return v;
    }

    Value ResolveLocalClassFuncCall(StatPtr stat, ValueType t, String key, Value& val)
    {
        SRC_REF(fn_call, FunctionCall, stat)
        auto class_iter = class_define.find(t);
        if (class_iter == class_define.end())
        {
            THROW_STACK_MSG("class {} is not defined", type_emun2str[(int)t])
        }
        Vector<Value> args;
        for (size_t i = 0; i < fn_call.arguments.size(); i++)
        {
            auto incoming_val = Dispatch(fn_call.arguments[i]);
            args.push_back(incoming_val);
        }
        try
        {
            return class_iter->second.ExecFunc(key, val, args);
        }
        catch (const std::exception& e)
        {
            THROW_STACK_MSG(e.what())
        }
    }
    Value ResolveVariableDeclaration(StatPtr stat)
    {
        SRC_REF(decl, VariableDeclaration, stat)
        for (auto& i : decl.declarations)
        {
            auto key = i->id.tok.kw;
            if (CurrScope().In(key))
            {
                THROW_STACK_MSG("Identifier '{}' has already been declared", key)
            }
            if (i->type.Is(const_) && !i->initialed)
            {
                THROW_STACK_MSG("Missing initializer in const declaration")
            }
            auto val = i->initialed ? Dispatch(i->init) : nullptr;
            return CurrScope()[key] = val;
        }
    }

    Value ResolveUnaryExpr(StatPtr stat)
    {
        SRC_REF(expr, UnaryExpression, stat);
        auto val = Dispatch(expr.stat);
        auto def = class_define[val.Type()];
        auto fn = def.unary_func.find(expr.op.GetKwEnum());
        if ((fn == def.unary_func.end()) || (fn->second.Type() != ValueType::function))
        {
            THROW_STACK_MSG("Unary operator {} for {} types is undefined", expr.op.kw, val.TypeString())
        }
        return FuncCall(fn->second, val);
    }

    Value ResolvePatternMatch(StatPtr stat)
    {
        SRC_REF(expr, PatternMatchStatment, stat);
        auto arg = Dispatch(expr.arg);
        for (auto&& i : expr.pairs)
        {
            auto& cond_stat = *i.first.get();
            auto t = cond_stat.Type();
            auto cond_val = Dispatch(i.first);
            if (t == StatementType::objectInit && arg.Type() == cond_val.Type())
            {
                bool all = true;
                for (auto&& i : cond_val.ObjC().SrcC())
                {
                    if (!arg.In(i.first) || !(arg[i.first] == i.second))
                    {
                        all = false;
                        break;
                    }
                }
                if (all)
                {
                  return Dispatch(i.second);
                }
            }
            else if (t == StatementType::arrayInit && arg.Type() == cond_val.Type())
            {
                bool all = true;
                auto& cond_arr = cond_val.Arr().Src();
                for (size_t i = 0; i < cond_arr.size(); i++)
                {
                    if (!arg.In(i) || !(arg[i] == cond_arr[i]))
                    {
                        all = false;
                        break;
                    }
                }
                if (all)
                {
                  return Dispatch(i.second);
                }

            }
            else if (t == StatementType::nullLiteral || cond_stat.IsLiteral())
            {

                if (cond_val == arg)
                {
                    return Dispatch(i.second);
                }
            }
            else
            {
                if (cond_val.ToBool())
                {
                    return Dispatch(i.second);
                }
            }
        }
        return Dispatch(expr.default_val);
    }

    Value ResolveConditionExpression(StatPtr stat)
    {
        SRC_REF(expr, ConditionExpression, stat);
        auto cond = Dispatch(expr.cond).ToBool();
        if (cond)
        {
            return Dispatch(expr.left);
        }
        return Dispatch(expr.right);
    }
    Value ResolveAssigmentStat(StatPtr stat)
    {
        SRC_REF(assig, AssigmentStatement, stat);
        auto val = Dispatch(assig.value);
        if (assig.target->Type() == StatementType::identifier)
        {
            SRC_REF(id, Identifier, assig.target)
            return SetValue(id.tok.kw, val);
        }
        SRC_REF(idx, IndexStatement, assig.target)
        return ResolveObjectIndex(assig.target, {val});
    }
    Value ResolveIdentifier(StatPtr stat)
    {
        SRC_REF(id, Identifier, stat);
        auto closure = CurrCtx().closeure;
        if (closure != nullptr)
        {
            auto iter = closure->find(id.tok.kw);
            if (iter != closure->end())
            {
                return GetValue(iter->second)->get();
            }
        }
        auto v = GetValue(id.tok.kw);
        if (!v)
        {
            THROW_STACK_MSG("{} is not defined", id.tok.kw)
        }
        return v->get();
    }
    Value ResolveReturn(StatPtr stat)
    {
        SRC_REF(ret, ReturnStatment, stat);
        curr_scope_return = true;
        return ret.Type() == StatementType::statement ? nullptr : Dispatch(ret.expr);
    }
    Value ResolveBinaryExpression(StatPtr stat)
    {
        SRC_REF(expr, BinaryExpression, stat);
        auto left = Dispatch(expr.left);
        auto type = left.Type();
        auto op = expr.op.InitKwEnum();
        // 针对这两种特殊处理
        if ((op == and_and_ && !left.ToBool()) || (op == or_or_ && left.ToBool()))
        {
            return left;
        }
        auto right = Dispatch(expr.right);
        auto type_r = right.Type();
#define ERR_ResolveBinaryExpression THROW_STACK_MSG("type:{} {} type:{} is not defined", left.TypeString(), expr.op.kw, right.TypeString())
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

        auto target_op_def = target_type_def->second.find(op);
        if (target_op_def == target_type_def->second.end())
        {
            if (op == not_eq_ && (target_type_def->second.find(eqeq_) != target_type_def->second.end()))
            {
                auto res = target_type_def->second[eqeq_](left, right);
                return !res.ToBool();
            }

            ERR_ResolveBinaryExpression
        }
        return target_op_def->second(left, right);
    }
    Value ResolveBoolLiteralInit(StatPtr stat)
    {
        SRC_REF(init, BoolLiteralInit, stat);
        return init.val;
    }
    Value ResolvNullLiteralInit(StatPtr) { return nullptr; }
    Value ResolveNumberLiteralInit(StatPtr stat)
    {
        SRC_REF(init, NumberLiteralInit, stat);
        return stod(init.start.kw);
    }
    Value ResolveStringLiteralInit(StatPtr stat)
    {
        SRC_REF(init, StringLiteralInit, stat);
        return init.start.toStringContent();
    }
    Value ResolveObjectIndex(StatPtr stat, std::optional<Value> set_value = {})
    {
        auto t = stat->Type();
        SRC_REF(idx_stat, IndexStatement, stat);
        Value par;
        bool is_set_val = set_value.has_value();
        std::optional<String> defer_func_key;
        for (size_t index = 0; index < idx_stat.indexes.size(); index++)
        {
            auto& i = idx_stat.indexes[index];
            bool is_last = index + 1 == idx_stat.indexes.size();
            bool is_set = is_last && is_set_val;
            if (index == 0)
            {
                par = Dispatch(i.stat);
                continue;
            }
            if (i.type == IndexType::property)
            {
                SRC_REF(id, Identifier, i.stat)
                String key = id.tok.kw;
                if (is_set)
                {
                    par[key] = set_value.value();
                }
                auto need_defer_exec_fn = false;
                auto is_next_fn = !is_last && idx_stat.indexes[index + 1].type == IndexType::call;
                if (par.Type() == ValueType::object)
                {
                    if (!par.In(key) && is_next_fn)
                    {
                        need_defer_exec_fn = true;
                    }
                }
                else
                {
                    if (is_next_fn)
                    {
                        need_defer_exec_fn = true;
                    }
                }

                if (need_defer_exec_fn)
                {
                    defer_func_key = {key};
                    continue;
                }

                par = par[key];
            }
            else if (i.type == IndexType::index)
            {
                auto key = Dispatch(i.stat);
                auto kt = key.Type();
                if (kt == ValueType::string)
                {
                    if (is_set)
                    {
                        par[key.ToString()] = set_value.value();
                    }
                    par = par.In(key.ToString()) ? par[key.ToString()] : nullptr;
                    continue;
                }
                else if (kt == ValueType::number)
                {
                    auto idx = key.Number();
                    if (is_set)
                    {
                        par[idx] = set_value.value();
                    }
                    par = par.In(idx) ? par[idx] : nullptr;
                    continue;
                }
                THROW_STACK_MSG("仅允许数字和字符串类型作为索引，当前:{}", key.TypeString())
            }
            else
            {
                SRC_REF(fn, FunctionCall, i.stat);
                SRC_REF(key, Identifier, fn.id);
                auto t = par.Type();
                auto key_str = defer_func_key.value_or("");
                defer_func_key = {};
                switch (t)
                {
                case ValueType::null:
                    THROW_STACK_MSG("NullPointerException property:{}", key_str);
                case ValueType::function:
                    par = ResolveFuncCall(i.stat, par);
                    break;
                case ValueType::object:
                    if (par.In(ability_key))
                    {
                        bool continue_flag = false;
                        for (auto&& abi_item : par[ability_key].Arr().Src())
                        {
                            auto abi_idx = abi_item["key"].Number();
                            auto target_abi = ability_define[abi_idx];
                            if (target_abi.In(key_str))
                            {
                                par = ResolveFuncCall(i.stat, target_abi[key_str], {par});
                                continue_flag = true;
                                continue;
                            }
                        }
                        if (continue_flag)
                        {
                            continue;
                        }
                    }
                default:
                    ASSERT_T(key_str.size() > 0)
                    par = ResolveLocalClassFuncCall(i.stat, t, key_str, par);
                    break;
                }
            }
        }
        return par;
    }

    Value ResolveArrayInit(StatPtr stat)
    {
        SRC_REF(arr, ArrayInit, stat)
        Array arr_src;
        for (auto& i : arr.src)
        {
            arr_src.Src().push_back(Dispatch(i));
        }
        return arr_src;
    }

    Value ResolveObjectInit(StatPtr stat)
    {
        SRC_REF(obj, ObjectInit, stat)
        Object obj_src;
        for (auto& i : obj.src)
        {
            obj_src[i.first] = Dispatch(i.second);
        }
        return obj_src;
    }

    Value ResolveBlock(StatPtr stat)
    {
        Value v;
        SRC_REF(block, BlockStatment, stat);
        Context block_ctx;
        PushContext(block_ctx);
        for (auto&& i : block.stats)
        {
            v = Dispatch(i);
        }
        PopContext();
        return v;
    }

    Value ResolveFuncDeclear(StatPtr stat)
    {
        SRC_REF(fn_stat, FunctionDeclaration, stat);
        static std::map<String, ClosureMemory> closure_mem;
        auto generate_func_id = [&](String fn_pos_id)
        {
            static std::map<String, int> func_assig_id_set;
            auto iter = func_assig_id_set.find(fn_pos_id);
            String tpl = "{ offset:{}, pos:{} }";
            if (iter == func_assig_id_set.end())
            {
                func_assig_id_set[fn_pos_id] = 0;
                return String::Format(tpl, 0, fn_pos_id);
            }
            return String::Format(tpl, ++func_assig_id_set[fn_pos_id], fn_pos_id);
        };

        Vector<Context> virtual_ctx_stack;
        std::function<void(StatPtr, ClosureMemory&)> Visitor = [&](StatPtr s, ClosureMemory& closure)
        {
            auto save_value_to_closure = [&](String kw)
            {
                int virtual_ctx_stack_idx = virtual_ctx_stack.size() - 1;
                int idx_mut = virtual_ctx_stack_idx;
                auto val = GetValue(kw, virtual_ctx_stack, idx_mut);
                if (val)
                {
                    closure.map[kw] = Closure(virtual_ctx_stack_idx - idx_mut, kw);
                }
                else
                {
                    val = GetValue(kw);
                    if (val)
                    {
                        closure.map[kw] = Closure::From(val->get());
                    }
                }
                return val;
            };
            switch (s->Type())
            {
            case StatementType::assigmentStatement:
            {
                SRC_REF(stat, AssigmentStatement, s)
                Visitor(stat.value, closure);
                Visitor(stat.target, closure);
                return;
            }
            case StatementType::blockStatment:
            {
                SRC_REF(stat, BlockStatment, s)
                Context ctx;
                virtual_ctx_stack.push_back(ctx);
                for (auto&& i : stat.stats)
                {
                    Visitor(i, closure);
                }
                virtual_ctx_stack.pop_back();
                return;
            }
            case StatementType::functionDeclaration:
            {
                SRC_REF(stat, FunctionDeclaration, s)
                Context ctx;
                ClosureMemory closure_next;
                for (auto& i : stat.arguments)
                {
                    ctx.var[i.name.kw] = true;
                }
                virtual_ctx_stack.push_back(ctx);
                for (auto& i : stat.arguments)
                {
                    if (i.initialed)
                    {
                        Visitor(i.init, closure_next);
                    }
                }
                for (auto i : stat.body)
                {
                    Visitor(i, closure_next);
                }
                virtual_ctx_stack.pop_back();
                closure_mem[stat.start.UniqId()] = closure_next;
                closure.children[stat.start.UniqId()] = closure_next;
                return;
            }
            case StatementType::functionCall:
            {
                SRC_REF(call, FunctionCall, s)
                for (auto& i : call.arguments)
                {
                    Visitor(i, closure);
                }
                s = call.id;
            }
            case StatementType::identifier:
            {
                SRC_REF(id, Identifier, s)
                auto kw = id.tok.kw;
                save_value_to_closure(kw);
                return;
            }
            case StatementType::indexStatement:
            {
                SRC_REF(obj_idx_stat, IndexStatement, s)
                auto par = obj_idx_stat.indexes[0];
                if (par.stat->Type() == StatementType::identifier)
                {
                    SRC_REF(obj_stat, Identifier, par.stat)
                    save_value_to_closure(obj_stat.tok.kw);
                }
                for (auto&& i : obj_idx_stat.indexes.Slice(1))
                {
                    if (i.type == IndexType::property || i.type == IndexType::call)
                    {
                        if (i.stat->Type() == StatementType::functionCall)
                        {
                            SRC_REF(fn, FunctionCall, i.stat)
                            for (auto&& arg : fn.arguments)
                            {
                                Visitor(arg, closure);
                            }
                        }
                    }
                    else
                    {
                        Visitor(i.stat, closure);
                    }
                }
                return;
            }
            case StatementType::conditionExpression:
            {
                SRC_REF(cond, ConditionExpression, s)
                for (auto& i : {cond.cond, cond.left, cond.right})
                {
                    Visitor(i, closure);
                }
                return;
            }
            case StatementType::unaryExpression:
            {
                SRC_REF(unary, UnaryExpression, s)
                Visitor(unary.stat, closure);
                return;
            }
            case StatementType::arrayInit:
            {
                SRC_REF(arr, ArrayInit, s)
                for (auto& i : arr.src)
                {
                    Visitor(i, closure);
                }

                return;
            }
            case StatementType::objectInit:
            {
                SRC_REF(obj, ObjectInit, s)
                for (auto& i : obj.src)
                {
                    Visitor(i.second, closure);
                }

                return;
            }
            case StatementType::binaryExpression:
            {
                SRC_REF(bin, BinaryExpression, s)
                for (auto& i : {bin.left, bin.right})
                {
                    Visitor(i, closure);
                }
                return;
            }
            case StatementType::variableDeclaration:
            {
                SRC_REF(decl, VariableDeclaration, s)
                auto& ctx = virtual_ctx_stack.back();
                auto& scope = ctx.var;
                for (auto& i : decl.declarations)
                {
                    auto& e = *i;
                    scope[e.id.tok.kw] = true;
                    if (e.initialed)
                    {
                        Visitor(e.init, closure);
                    }
                }
                return;
            }
            }
        };
        auto func_pos_id = fn_stat.start.UniqId();
        Function fn(generate_func_id(func_pos_id));
        fn.enable_closure = true;
        fn.src = std::static_pointer_cast<FunctionDeclaration>(stat);
        auto closure_iter = closure_mem.find(func_pos_id);
        if (closure_iter != closure_mem.end())
        {
            const int curr_ctx_idx = ctx_stack.size() - 1;
            auto closure_curr = closure_mem[func_pos_id];
            std::function<void(ClosureMemory&, int)> traverse = [&](ClosureMemory& mem, int deep)
            {
                for (auto& i : mem.map)
                {
                    auto& clos = i.second;
                    // P("key:{} init:{} offset:{}", i.first,
                    // clos.initialed, clos.stack_offset)
                    if (clos.initialed) // 已经赋值完的不处理
                    {
                        continue;
                    }
                    if (deep > clos.stack_offset) // 还没生成
                    {
                        // P("waiting for current context
                        // generate kw:{}", clos.kw)
                    }
                    else if (deep <= clos.stack_offset) // 在当前上下文生成，已生成的
                    {
                        // P("has created kw:{} value:{}",
                        // clos.kw, ValueOrUndef(clos.kw))
                        closure_curr.map[clos.kw] = Closure::From(ValueOrUndef(clos.kw)); // 会优先从当前上下文的闭包中取
                        auto& v = closure_curr.map[clos.kw].val;
                        if (v.Type() == ValueType::array)
                        {
                            MemManger::Get().Closure()[std::to_string(size_t(v.ArrC().Ptr()))] = v;
                        }
                        else if (v.Type() == ValueType::object)
                        {
                            MemManger::Get().Closure()[std::to_string(size_t(v.ObjC().Ptr()))] = v;
                        }
                    }
                }
                for (auto& i : mem.children) // 递归处理嵌套的函数
                {
                    // P("next deep:{}", deep + 1)
                    traverse(i.second, deep + 1);
                }
            };
            traverse(closure_curr, 1);
            fn.closure = closure_curr.map;
        }
        else
        {
            ClosureMemory closure;
            Visitor(stat, closure);
            fn.closure = closure.map;
            closure_mem[func_pos_id] = closure;
        }

        func_mem[fn.id] = fn;
        return Value::CreateFunc(fn.id);
    }
};
int VM::incr_id = 0;
} // namespace agumi
