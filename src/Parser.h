#pragma once
#include "Tokenizer.h"
namespace agumi
{
enum class StatementType
{
    statement,
    identifier,
    variableDeclarator,
    functionDeclaration,
    functionCall,
    variableDeclaration,
    numberLiteralInit,
    stringLiteralInit,
    nullLiteral,
    boolLiteralInit,
    ifStatment,
    conditionExpression,
    binaryExpression,
    unaryExpression,
    assigmentStatement,
    indexStatement,
    arrayInit,
    objectInit,
    blockStatment
};

class Statement
{
  public:
    virtual StatementType Type() { return StatementType::statement; }
    virtual bool Is(StatementType type) { return Type() == type; }
    virtual bool IsEmpty() { return Is(StatementType::statement); }
    Token start;
    virtual bool IsLiteral()
    {
        auto t = Type();
        return t == StatementType::boolLiteralInit || t == StatementType::numberLiteralInit || t == StatementType::stringLiteralInit;
    }
    virtual Value ToJson() { return "base statement"; }
    virtual ~Statement(){};
    virtual void Expect(StatementType type)
    {
        if (Type() != type)
        {
            throw std::logic_error(String::Format("parse error! expectd {} :  received {}", int(type), int(Type())));
        }
    }
};

using StatPtr = std::shared_ptr<Statement>;

class Identifier : public Statement
{
  public:
    Token tok;
    Identifier(Token _tok) : tok(_tok) {}

    StatementType Type() { return StatementType::identifier; }
    Value ToJson()
    {
        auto r = Object();
        r["type"] = "Identifier";
        r["name"] = tok.ToJson();
        r["start"] = start.ToPosStr();
        return r;
    }
};

class Program
{
  private:
  public:
    Vector<StatPtr> body;
    Program(/* args */) = default;
    ~Program() = default;
    Value ToJson()
    {
        Value res = Array();
        for (auto& i : body)
        {
            res.Arr().Src().push_back(i->ToJson());
        }
        return res;
    }
};

// 声明项
class VariableDeclarator : public Statement
{
  public:
    VariableDeclarator(Identifier _id) : id(_id) {}
    Identifier id;
    Token type;
    bool initialed = false;
    StatPtr init;
    StatementType Type() { return StatementType::variableDeclarator; }
    Value ToJson()
    {
        Value r = Object();
        r["type"] = "VariableDeclarator";
        r["id"] = id.ToJson();
        r["initialed"] = initialed;
        r["type"] = type.ToJson();
        r["init"] = init->ToJson();
        r["start"] = start.ToPosStr();
        return r;
    }
};

class VariableDeclaration : public Statement
{
  public:
    Token kind;
    Vector<std::shared_ptr<VariableDeclarator>> declarations = {};
    VariableDeclaration() = default;
    VariableDeclaration(Token _kind) : kind(_kind) {}

    StatementType Type() { return StatementType::variableDeclaration; }

    Value ToJson()
    {
        auto decls = Array();
        for (auto& i : declarations)
        {
            decls.Src().push_back(i->ToJson());
        }
        auto res = Object();
        res["type"] = "VariableDeclaration";
        res["kind"] = kind.ToJson();
        res["declarations"] = decls;
        res["start"] = start.ToPosStr();
        return res;
    }
};

class ArrayInit : public Statement
{
  public:
    Vector<std::shared_ptr<Statement>> src;
    ArrayInit(){};
    StatementType Type() { return StatementType::arrayInit; }
    Value ToJson()
    {
        auto arr = Array();
        for (auto& i : src)
        {
            arr.Src().push_back(i->ToJson());
        }
        auto r = Object();
        r["src"] = arr;
        r["type"] = "arrayInit";
        r["start"] = start.ToPosStr();
        return r;
    }
};

class ObjectInit : public Statement
{
  public:
    std::map<String, StatPtr> src;
    ObjectInit(){};
    StatementType Type() { return StatementType::objectInit; }
    Value ToJson()
    {
        auto obj = Object();
        for (auto& i : src)
        {
            obj[i.first] = i.second->ToJson();
        }
        auto r = Object();
        r["src"] = obj;
        r["type"] = "objectInit";
        r["start"] = start.ToPosStr();
        return r;
    }
};

class ConditionExpression : public Statement
{
  public:
    // ? 左边
    StatPtr cond;

    // : 左边
    StatPtr left;

    // ：右边
    StatPtr right;
    StatementType Type() { return StatementType::conditionExpression; }

    Value ToJson()
    {
        Value r = Object();
        r["type"] = "ConditionExpression";
        r["cond"] = cond->ToJson();
        r["left"] = left->ToJson();
        r["right"] = right->ToJson();
        r["start"] = start.ToPosStr();
        return r;
    }
};

class BinaryExpression : public Statement
{

  public:
    // 运算符
    Token op;
    // 左边
    StatPtr left;
    // 右边
    StatPtr right;
    StatementType Type() { return StatementType::binaryExpression; }
    Value ToJson()
    {
        Value r = Object();
        r["type"] = "BinaryExpression";
        r["op"] = op.ToJson();
        r["left"] = left->ToJson();
        r["right"] = right->ToJson();
        r["start"] = start.ToPosStr();
        return r;
    }
};
class UnaryExpression : public Statement
{

  public:
    // 运算符
    Token op;
    // 左边
    StatPtr stat;
    StatementType Type() { return StatementType::unaryExpression; }
    Value ToJson()
    {
        Value r = Object();
        r["type"] = "UnaryExpression";
        r["op"] = op.ToJson();
        r["left"] = stat->ToJson();
        r["start"] = start.ToPosStr();
        return r;
    }
};

struct FunctionArgument
{
    Token name;
    bool initialed = false;
    StatPtr init;
};

class FunctionDeclaration : public Statement
{
  public:
    Vector<FunctionArgument> arguments;
    Vector<StatPtr> body;
    StatementType Type() { return StatementType::functionDeclaration; }
    Value ToJson()
    {
        Value r = Object();
        r["type"] = "FunctionDeclaration";
        Array args;
        for (auto i : arguments)
        {
            Value a = Object();
            a["name"] = i.name.ToJson();
            a["initialed"] = i.initialed;
            if (i.initialed)
            {
                a["init"] = i.init->ToJson();
            }
            args.Src().push_back(a);
        }
        r["arguments"] = args;

        Array body_stat;
        for (auto& i : body)
        {
            body_stat.Src().push_back(i->ToJson());
        }
        r["body"] = body_stat;
        r["start"] = start.ToPosStr();
        return r;
    }
};

class FunctionCall : public Statement
{
  public:
    Vector<StatPtr> arguments;
    StatPtr id;
    StatementType Type() { return StatementType::functionCall; }
    Value ToJson()
    {
        Value r = Object();
        r["type"] = "FunctionCall";
        Array args;
        for (auto& i : arguments)
        {
            args.Src().push_back(i->ToJson());
        }
        r["id"] = id->ToJson();
        r["args"] = args;
        r["start"] = start.ToPosStr();
        return r;
    }
};

enum class IndexType
{
    // aa.dd aa.dd()
    property,
    // sss()()()
    call,
    // sss[1]
    index
};
class IndexNodeWrapper
{
  public:
    IndexNodeWrapper(StatPtr _stat, IndexType _type = IndexType::property)
    {
        stat = _stat;
        type = _type;
    }
    StatPtr stat;
    IndexType type;

    Value ToJson()
    {
        Value r = Object();
        r["type"] = "IndexNodeWrapper";
        r["stat"] = stat->ToJson();
        r["type"] = int(type);
        return r;
    }
};

class IndexStatement : public Statement
{
  public:
    Vector<IndexNodeWrapper> indexes;
    StatementType Type() { return StatementType::indexStatement; }
    Value ToJson()
    {
        Value r = Object();
        r["type"] = "IndexStatement";
        auto arr = Array();
        for (auto&& i : indexes)
        {
            arr.Src().push_back(i.ToJson());
        }

        r["indexes"] = arr;
        return r;
    }
};

class NumberLiteralInit : public Statement
{
  public:
    NumberLiteralInit(Token _tok) { this->start = _tok; }
    StatementType Type() { return StatementType::numberLiteralInit; }
    Value ToJson()
    {
        auto r = Object();
        r["type"] = "NumberLiteral";
        r["value"] = start.ToJson();
        r["start"] = start.ToPosStr();
        return r;
    }
};

class BoolLiteralInit : public Statement
{
  public:
    bool val;
    BoolLiteralInit(Token _tok)
    {
        this->start = _tok;
        val = _tok.Is(true_);
    }
    StatementType Type() { return StatementType::boolLiteralInit; }
    Value ToJson()
    {
        auto r = Object();
        r["type"] = "BoolLiteral";
        r["value"] = start.ToJson();
        r["start"] = start.ToPosStr();
        r["val"] = val;
        return r;
    }
};

class StringLiteralInit : public Statement
{
  public:
    StringLiteralInit(Token _tok) { this->start = _tok; }
    StatementType Type() { return StatementType::stringLiteralInit; }
    Value ToJson()
    {
        auto r = Object();
        r["type"] = "StringLiteral";
        r["value"] = start.ToJson();
        r["start"] = start.ToPosStr();
        return r;
    }
};

class NullLiteral : public Statement
{
  public:
    NullLiteral(Token _tok) { this->start = _tok; }
    StatementType Type() { return StatementType::nullLiteral; }
    Value ToJson()
    {
        auto r = Object();
        r["type"] = "NullLiteral";
        r["value"] = nullptr;
        r["start"] = start.ToPosStr();
        return r;
    }
};

class AssigmentStatement : public Statement
{
  public:
    StatPtr target;
    StatPtr value;

    StatementType Type() { return StatementType::assigmentStatement; }
    Value ToJson()
    {
        auto r = Object();
        r["target"] = target->ToJson();
        r["value"] = value->ToJson();
        r["type"] = "assigmentStatement";
        return r;
    }
};

class BlockStatment : public Statement
{
  public:
    Vector<StatPtr> stats;
    StatementType Type() { return StatementType::blockStatment; }
    Value ToJson()
    {
        auto r = Object();
        r["blocks"] = Array();
        for (auto&& i : stats)
        {
            r["blocks"].Arr().Src().push_back(i->ToJson());
        }

        r["type"] = "blockStatment";
        return r;
    }
};

class IfStatment : public Statement
{
  public:
    // [if,elif,elif,else]
    Vector<TokenFlowView> blocks;
    StatementType Type() { return StatementType::ifStatment; }
};

// 计算括号闭合的位置

TokenIter CalcEndBrackets(TokenFlowView tfv)
{
    auto s_tok = tfv.BeginIter();
    KW s_bracket = parenthesis_start_;
    KW e_bracket = parenthesis_end_;
    if (s_tok->Is(parenthesis_start_))
    {
    }
    else if (s_tok->Is(brackets_start_))
    {
        s_bracket = brackets_start_;
        e_bracket = brackets_end_;
    }
    else if (s_tok->Is(curly_brackets_start_))
    {
        s_bracket = curly_brackets_start_;
        e_bracket = curly_brackets_end_;
    }
    else
    {
        THROW_TOKEN(*s_tok);
    }
    std::vector<KW> parenthesis_stack{s_bracket};
    auto tok = tfv.BeginIter();
    while (true)
    {
        tok++;
        if (tok->Is(s_bracket))
        {
            if (parenthesis_stack.back() == s_bracket)
            {
                parenthesis_stack.push_back(s_bracket);
            }
            else
            {
                parenthesis_stack.pop_back();
            }
        }
        else if (tok->Is(e_bracket))
        {
            if (parenthesis_stack.back() == e_bracket)
            {
                parenthesis_stack.push_back(e_bracket);
            }
            else
            {
                parenthesis_stack.pop_back();
            }
        }
        if (parenthesis_stack.size() == 0)
        {
            return tok;
        }
    }
    THROW_MSG("找不到括号: {} 的结尾位置", s_tok->ToDebugStr());
}

class Compiler
{
    template <class T> using WithEnd = std::tuple<T, TokenIter>;
    using StatPtrWithEnd = WithEnd<StatPtr>;

  public:
    StatPtr CreateLiteralNode(const Token& tok)
    {
        if (tok.IsNumericLiteral())
        {
            return std::make_shared<NumberLiteralInit>(tok);
        }
        if (tok.IsBoolLiteral())
        {
            return std::make_shared<BoolLiteralInit>(tok);
        }
        if (tok.IsStringLiteral())
        {
            return std::make_shared<StringLiteralInit>(tok);
        }
        if (tok.IsNull())
        {
            return std::make_shared<NullLiteral>(tok);
        }
        THROW
    }

    int Skip(TokenIter& iter, KW kw = cr_)
    {
        int i = 0;
        while (iter->Is(kw))
        {
            iter++;
            i++;
        }
        return i;
    }

    // 处理声明语句，只支持使用let const进行单个声明
    WithEnd<std::shared_ptr<VariableDeclaration>> ResolveDeclearStatment(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        auto var_declear = std::make_shared<VariableDeclaration>(*iter);
        var_declear->start = *iter;
        auto id_iter = iter + 1;
        auto decl = std::make_shared<VariableDeclarator>(Identifier(*id_iter));
        decl->start = *id_iter;
        decl->type = *iter;
        var_declear->declarations.push_back(decl);
        auto assigment_or_end_iter = id_iter + 1;
        if (assigment_or_end_iter->IsLineEndToken() || tfv.IsEnd(assigment_or_end_iter)) // 没初始化
        {
            return {var_declear, assigment_or_end_iter};
        }
        else if (assigment_or_end_iter->Is(assigment_)) // 有初始化
        {
            decl->initialed = true;
            auto init_iter = assigment_or_end_iter + 1;
            auto [init_stat, end_iter] = ResolveExecutableStatment(init_iter);
            decl->init = init_stat;
            return {var_declear, end_iter};
        }
        THROW_TOKEN(*assigment_or_end_iter)
    }

    bool IsExpr(TokenIter iter) const { return iter->Include(expr_operator); }

    // x-1 令牌化 'x' '-1' 这边解析成 x + -1
    StatPtrWithEnd ResolveSubNumberLiteral(StatPtr pos1_stat, TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        auto [pos2_stat, pos2_end_iter] = ResolveExecutableStatment(iter);
        auto stat = std::make_shared<BinaryExpression>();
        stat->start = *iter;
        stat->left = pos1_stat;
        stat->right = pos2_stat;
        stat->op = Token('+');
        return {stat, pos2_end_iter};
    }

    StatPtrWithEnd ResolveIndex(StatPtr pos1_stat, TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        auto stat = std::make_shared<IndexStatement>();
        stat->start = *iter;
        stat->indexes.push_back(IndexNodeWrapper(pos1_stat));
        while (true)
        {
            if (iter->Is(dot_))
            {
                iter++;
                if (!iter->IsIdentifier())
                {
                    THROW_TOKEN(*iter)
                }
                auto id_stat = std::make_shared<Identifier>(*iter);
                id_stat->start = *iter;
                stat->indexes.push_back(IndexNodeWrapper(id_stat));
                iter++;
                continue;
            }
            else if (iter->Is(brackets_start_))
            {
                iter++;
                auto [id_stat, end_iter] = ResolveExecutableStatment(iter);
                stat->indexes.push_back(IndexNodeWrapper(id_stat, IndexType::index));
                iter = end_iter;
                iter->Expect(brackets_end_);
                iter++;
                continue;
            }
            else if (iter->Is(parenthesis_start_))
            {
                auto [fn_call, end_iter] = ResolveFuncCall(std::make_shared<Statement>(), iter, false);
                auto t = fn_call->Type();
                if (t == StatementType::functionCall)
                {
                    stat->indexes.push_back(IndexNodeWrapper(fn_call, IndexType::call));
                    iter = end_iter + 1;
                    continue;
                }
                THROW_MSG("undefined type {}", int(t))
            }
            break;
        }

        return {stat, iter};
    }

    TokenIter GetNextNotEmptyToken(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        while (iter->IsEmpty())
        {
            ASSERT_T(!iter->IsEnd())
            iter++;
        }
        return iter;
    }

    StatPtrWithEnd ResolveExpr(StatPtr pos1_stat, TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        auto probably_question_mask_or_unary = GetNextNotEmptyToken(iter);
        if (probably_question_mask_or_unary->Is(question_mask_)) // x ? y : z 三元
        {
            auto [pos2_stat, pos2_end_iter] = ResolveExecutableStatmentUntilNotEmpty(probably_question_mask_or_unary + 1);
            auto stat = std::make_shared<ConditionExpression>();
            stat->start = *iter;
            stat->cond = pos1_stat;
            stat->left = pos2_stat;
            auto colon_iter = GetNextNotEmptyToken(pos2_end_iter);
            colon_iter->Expect(colon_);
            auto [pos3_stat, pos3_end_iter] = ResolveExecutableStatmentUntilNotEmpty(colon_iter + 1);
            stat->right = pos3_stat;
            return {stat, pos3_end_iter};
        }
        // 一元运算符
        if (expr_operator_unary.Includes(probably_question_mask_or_unary->GetKwEnum()))
        {
            auto [pos2_stat, pos2_end_iter] = ResolveExecutableStatmentUntilNotEmpty(probably_question_mask_or_unary + 1);
            auto stat = std::make_shared<UnaryExpression>();
            stat->op = *probably_question_mask_or_unary;
            stat->stat = pos2_stat;
            return {stat, pos2_end_iter};
        }

        auto op_iter = iter;
        auto [pos2_stat, pos2_end_iter] = ResolveExecutableStatment(op_iter + 1);
        // x(+|-|*|/|++|+=|===。。。)y 二元
        auto stat = std::make_shared<BinaryExpression>();
        stat->start = *iter;
        stat->left = pos1_stat;
        stat->right = pos2_stat;
        stat->op = *op_iter;
        auto mul_div = Vector<KW>::From({mul_, div_});
        int priority = op_iter->Include(mul_div) ? 2 : 1;
        if (pos2_stat->Type() == StatementType::binaryExpression) // 调整运算的优先级
        {
            SRC_REF(pos2_bin_expr, BinaryExpression, pos2_stat);
            int left_priority = pos2_bin_expr.op.Include(mul_div) ? 2 : 1;
            if (left_priority < priority)
            {
                auto stat_adj = std::make_shared<BinaryExpression>();
                stat_adj->start = *iter;
                stat_adj->op = pos2_bin_expr.op;
                stat_adj->right = pos2_bin_expr.right;
                stat_adj->left = stat;
                stat->right = pos2_bin_expr.left;
                return {stat_adj, pos2_end_iter};
            }
        }
        return {stat, pos2_end_iter};
    }

    StatPtrWithEnd ResolveFunction(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        auto stat = std::make_shared<FunctionDeclaration>();
        stat->start = *iter;
        stat->start = *iter;
        bool is_omit_parenthesis = iter->IsIdentifier();
        if (is_omit_parenthesis)
        {
            FunctionArgument arg;
            arg.name = *iter;
            arg.initialed = false;
            stat->arguments.push_back(arg);
        }
        else
        {
            iter++;
            while (true)
            {
                FunctionArgument arg;
                if (iter->IsIdentifier())
                {
                    arg.name = *iter;
                    iter++;
                    if (iter->Is(assigment_)) // 带默认参数
                    {
                        iter++;
                        auto [val, end_iter] = ResolveExecutableStatment(iter);
                        arg.initialed = true;
                        arg.init = val;
                        iter = end_iter;
                    }
                    stat->arguments.push_back(arg);
                    if (iter->Is(comma_)) // 到下一个参数
                    {
                        iter++;
                        continue;
                    }
                }
                if (iter->Is(parenthesis_end_)) // 结束
                {
                    break;
                }
                THROW_TOKEN(*iter)
            }
        }
        iter++;
        iter->Expect(arrow_);
        iter++;
        auto is_block = iter->Is(curly_brackets_start_);
        if (is_block)
        {
            auto end_iter = CalcEndBrackets(iter);
            iter++;
            while (iter != end_iter)
            {
                auto [body, end_iter] = Dispatch(iter);
                iter = end_iter;
                if (body->Type() == StatementType::statement)
                {
                    continue;
                }
                stat->body.push_back(body);
            }
            return {stat, end_iter + 1};
        }
        auto [body, end_iter] = ResolveExecutableStatment(iter);
        body->start = *iter;
        stat->body.push_back(body);
        return {stat, end_iter};
    }

    //
    StatPtrWithEnd ResolveFuncCall(StatPtr func, TokenFlowView tfv, bool seek = true)
    {
        auto iter = tfv.BeginIter(); // (
        auto stat = std::make_shared<FunctionCall>();
        stat->start = *iter;
        stat->id = func;
        auto end_iter = CalcEndBrackets(iter); // )
        iter++;
        while (iter != end_iter)
        {
            auto [arg_stat, arg_end_iter] = ResolveExecutableStatment(iter);
            stat->arguments.push_back(arg_stat);
            iter = arg_end_iter;
            if (iter->IsComma())
            {
                iter++;
                continue;
            }
            if (iter == end_iter)
            {
                break;
            }
            THROW_TOKEN(*iter)
        }
        if (!seek)
        {
            return {stat, iter};
        }

        return SeekIfExpr(stat, iter);
    }

    StatPtrWithEnd ResolveArrayInit(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        iter->Expect(brackets_start_);
        iter++;
        auto arr = std::make_shared<ArrayInit>();
        arr->start = *iter;
        if (iter->Is(brackets_end_))
        {
            return {arr, iter + 1};
        }
        while (true)
        {
            auto [stat, end_iter] = ResolveExecutableStatment(iter);
            arr->src.push_back(stat);
            iter = end_iter;
            if (iter->Is(comma_))
            {
                iter++;
                continue;
            }
            if (iter->Is(brackets_end_))
            {
                iter++;
                break;
            }
            THROW_TOKEN(*iter)
        }
        return {arr, iter};
    }

    StatPtrWithEnd ResolveObjectInit(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        iter->Expect(curly_brackets_start_);
        iter++;
        auto obj = std::make_shared<ObjectInit>();
        obj->start = *iter;
        if (iter->Is(curly_brackets_end_))
        {
            return {obj, iter + 1};
        }
        while (true)
        {
            Skip(iter);
            if (!iter->IsIdentifier() && !iter->IsStringLiteral())
            {
                THROW_TOKEN(*iter)
            }
            auto kw = iter->IsIdentifier() ? iter->kw : iter->toStringContent(true);
            iter++;
            auto forward_offset = Skip(iter);
            if (iter->Is(comma_) || iter->Is(curly_brackets_end_))
            {
                auto iter_id = iter - 1 - forward_offset;
                auto [id_stat, _] = ResolveExecutableStatment(iter_id);
                obj->src[kw] = id_stat;
            }
            else
            {
                iter->Expect(colon_);
                iter++;
                auto [stat, end_iter] = ResolveExecutableStatment(iter);
                obj->src[kw] = stat;
                iter = end_iter;
            }

            Skip(iter);
            if (iter->Is(comma_))
            {
                iter++;
                continue;
            }
            if (iter->Is(curly_brackets_end_))
            {
                iter++;
                break;
            }
            THROW_TOKEN(*iter)
        }
        return {obj, iter};
    }

    StatPtrWithEnd ResolveExecutableStatmentUntilNotEmpty(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        auto res = ResolveExecutableStatment(iter);
        while (std::get<0>(res)->IsEmpty())
        {
            iter = std::get<1>(res);
            res = ResolveExecutableStatment(iter);
        }
        return res;
    }

    /**
     * 一行，或者括号
     *@return [句子，末尾迭代器)
     */
    StatPtrWithEnd ResolveExecutableStatment(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        if (iter->Is(parenthesis_start_))
        {
            auto parenthesis_end = CalcEndBrackets(iter);
            auto next_end_iter = parenthesis_end + 1; //括号的下一个位置
            if (next_end_iter->Is(arrow_))            // 括号后面跟着箭头是个函数
            {
                return ResolveFunction(iter);
            }
            auto [stat, _] = ResolveExecutableStatment(iter + 1);
            if (IsExpr(next_end_iter))
            {
                return ResolveExpr(stat, next_end_iter);
            }
            return {stat, next_end_iter};
        }
        if (iter->Is(at_))
        {
            return ResolveBlock(iter);
        }

        if (iter->Is(brackets_start_))
        {
            auto [left_stat, end_iter] = ResolveArrayInit(iter);
            return SeekIfExpr(left_stat, end_iter - 1);
        }

        if (iter->Is(curly_brackets_start_))
        {
            return ResolveObjectInit(iter);
        }

        if (iter->IsLiteral())
        {
            auto left_stat = CreateLiteralNode(*iter);
            return SeekIfExpr(left_stat, iter);
        }
        if (iter->IsIdentifier())
        {
            auto left_stat = std::make_shared<Identifier>(*iter);
            left_stat->start = *iter;
            auto next_end_iter = iter + 1; // id的下一个位置
            if (next_end_iter->Is(arrow_)) // id后面跟着箭头是个函数
            {
                return ResolveFunction(iter);
            }
            return SeekIfExpr(left_stat, iter);
        }
        if (iter->IsLineEndToken())
        {
            StatPtr s = std::make_shared<Statement>();
            return {s, iter + 1};
        }
        if (expr_operator_unary.Includes(iter->GetKwEnum()))
        {
           return ResolveExpr(std::make_shared<Statement>(), iter);
        }

        THROW_TOKEN(*iter)
    }

    // 块
    StatPtrWithEnd ResolveBlock(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        iter->Expect(at_);
        iter++;
        iter->Expect(curly_brackets_start_);
        auto end = CalcEndBrackets(iter);
        auto stat = std::make_shared<BlockStatment>();
        iter++;
        while (true)
        {
            if (iter == end)
            {
                break;
            }

            auto [r_stat, r_iter] = Dispatch(iter);
            iter = r_iter;
            if (r_stat->IsEmpty())
            {
                continue;
            }

            stat->stats.push_back(r_stat);
        }
        return {stat, end + 1};
    }

    StatPtrWithEnd SeekIfExpr(StatPtr left_stat, TokenIter iter)
    {
        // 判断如果后面跟随着一个运算符继续向下折叠
        auto next_iter = iter + 1;
        if (IsExpr(next_iter) || GetNextNotEmptyToken(next_iter)->Is(question_mask_))
        {
            return ResolveExpr(left_stat, next_iter);
        }
        if (next_iter->Is(parenthesis_start_) && !left_stat->IsLiteral())
        {
            return ResolveFuncCall(left_stat, next_iter);
        }
        if (next_iter->kw[0] == '-' && next_iter->IsNumericLiteral())
        {
            return ResolveSubNumberLiteral(left_stat, next_iter);
        }
        if (next_iter->Is(dot_) || next_iter->Is(brackets_start_))
        {
            return ResolveIndex(left_stat, next_iter);
        }
        return {left_stat, next_iter};
    };

    StatPtrWithEnd ResovleAssigmentOrIdentify(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        auto [probably_assign, assigment_or_other] = ResolveExecutableStatment(iter);
        auto assignable = probably_assign->Type() == StatementType::identifier || probably_assign->Type() == StatementType::indexStatement;
        if (assignable && assigment_or_other->Is(assigment_))
        {
            if (probably_assign->Type() == StatementType::indexStatement)
            {
                SRC_REF(idx, IndexStatement, probably_assign)
                auto last_property = *(idx.indexes.end() - 1);
                if (last_property.stat->Type() == StatementType::functionCall)
                {
                    THROW_MSG("assignment statements last property index cannot be a function call .\nposition:{}",
                              last_property.stat->start.ToPosStr())
                }
            }

            auto value_iter = assigment_or_other + 1;
            auto [stat_ptr, end_iter] = ResolveExecutableStatment(value_iter);
            auto assigment = std::make_shared<AssigmentStatement>();
            assigment->start = *iter;
            assigment->target = probably_assign;
            assigment->value = stat_ptr;
            return {assigment, end_iter};
        }
        return {probably_assign, assigment_or_other};
    }

    StatPtrWithEnd Dispatch(TokenFlowView tfv)
    {
        auto iter = tfv.BeginIter();
        if (iter->IsDeclear())
        {
            return ResolveDeclearStatment(iter);
        }
        else if (iter->IsIdentifier())
        {
            return ResovleAssigmentOrIdentify(iter);
        }
        return ResolveExecutableStatment(iter);
    }

    Program ConstructAST(Vector<Token>& token_flow)
    {
        Program p;
        Token end_tok;
        end_tok.SetViewEnd();
        token_flow.push_back(end_tok);
        TokenFlowView tfv(token_flow.cbegin(), token_flow.cend());
        auto iter = tfv.BeginIter();
        auto accept = [&](StatPtrWithEnd res) {
            auto [stat, end] = res;
            iter = end;
            if (stat->IsEmpty())
            {
                return;
            }
            p.body.push_back(stat);
        };
        while (!tfv.IsEnd(iter))
        {

            if (iter->IsLineEndToken())
            {
                iter++;
                continue;
            }
            accept(Dispatch(iter));
        }
        return p;
    }
};
} // namespace agumi
