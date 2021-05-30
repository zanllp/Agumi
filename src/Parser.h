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
        boolLiteralInit,
        ifStatment,
        conditionExpression,
        binaryExpression,
        assigmentStatement,
        indexStatement,
        arrayInit
    };

    class Statement
    {
    public:
        virtual StatementType Type()
        {
            return StatementType::statement;
        }
        virtual bool IsLiteral()
        {
            auto t = Type();
            return t == StatementType::boolLiteralInit ||
                   t == StatementType::numberLiteralInit ||
                   t == StatementType::stringLiteralInit;
        }
        virtual Value ToJson()
        {
            return "base statement";
        }
        virtual ~Statement(){};
    };

    using StatPtr = std::shared_ptr<Statement>;

    class Identifier : public Statement
    {
    public:
        Token tok;
        Identifier(Token _tok) : tok(_tok) {}

        StatementType Type()
        {
            return StatementType::identifier;
        }
        Value ToJson()
        {
            auto r = JsObject();
            r["type"] = "Identifier";
            r["name"] = tok.ToJson();
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
            Value res = JsArray();
            for (auto &i : body)
            {
                res.Array().Src().push_back(i->ToJson());
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
        StatementType Type()
        {
            return StatementType::variableDeclarator;
        }
        Value ToJson()
        {
            Value r = JsObject();
            r["type"] = "VariableDeclarator";
            r["id"] = id.ToJson();
            r["initialed"] = initialed;
            r["type"] = type.ToJson();
            r["init"] = init->ToJson();
            return r;
        }
    };

    class VariableDeclaration : public Statement
    {
    public:
        Token kind;
        Vector<std::shared_ptr<VariableDeclarator>> declarations = {};
        VariableDeclaration() = default;
        VariableDeclaration(Token _kind)
            : kind(_kind)
        {
        }

        StatementType Type()
        {
            return StatementType::variableDeclaration;
        }

        Value ToJson()
        {
            auto decls = JsArray();
            for (auto &i : declarations)
            {
                decls.Src().push_back(i->ToJson());
            }
            auto res = JsObject();
            res["type"] = "VariableDeclaration";
            res["kind"] = kind.ToJson();
            res["declarations"] = decls;
            return res;
        }
    };

    class ArrayInit : public Statement
    {
    public:
        Vector<std::shared_ptr<Statement>> src;
        ArrayInit(){};
        StatementType Type()
        {
            return StatementType::arrayInit;
        }
        Value ToJson()
        {
            auto arr = JsArray();
            for (auto &i : src)
            {
                arr.Src().push_back(i->ToJson());
            }
            auto res = JsObject();
            res["src"] = arr;
            res["type"] = "arrayInit";
            return res;
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
        StatementType Type()
        {
            return StatementType::conditionExpression;
        }

        Value ToJson()
        {
            Value r = JsObject();
            r["type"] = "ConditionExpression";
            r["cond"] = cond->ToJson();
            r["left"] = left->ToJson();
            r["right"] = right->ToJson();
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
        StatementType Type()
        {
            return StatementType::binaryExpression;
        }
        Value ToJson()
        {
            Value r = JsObject();
            r["type"] = "BinaryExpression";
            r["op"] = op.ToJson();
            r["left"] = left->ToJson();
            r["right"] = right->ToJson();
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
        Token start;
        StatementType Type()
        {
            return StatementType::functionDeclaration;
        }
        Value ToJson()
        {
            Value r = JsObject();
            r["type"] = "FunctionDeclaration";
            JsArray args;
            for (auto i : arguments)
            {
                Value a = JsObject();
                a["name"] = i.name.ToJson();
                a["initialed"] = i.initialed;
                if (i.initialed)
                {
                    a["init"] = i.init->ToJson();
                }
                args.Src().push_back(a);
            }
            r["arguments"] = args;

            JsArray body_stat;
            for (auto &i : body)
            {
                body_stat.Src().push_back(i->ToJson());
            }
            r["body"] = body_stat;
            return r;
        }
    };

    class FunctionCall : public Statement
    {
    public:
        Vector<StatPtr> arguments;
        StatPtr id;
        StatementType Type()
        {
            return StatementType::functionCall;
        }
        Value ToJson()
        {
            Value r = JsObject();
            r["type"] = "FunctionCall";
            JsArray args;
            for (auto &i : arguments)
            {
                args.Src().push_back(i->ToJson());
            }
            r["id"] = id->ToJson();
            r["args"] = args;
            return r;
        }
    };

    class IndexStatement : public Statement
    {
    public:
        std::shared_ptr<Identifier> object;
        StatPtr property;
        StatementType Type()
        {
            return StatementType::indexStatement;
        }
        Value ToJson()
        {
            Value r = JsObject();
            r["type"] = "IndexStatement";
            r["propetry"] = property->ToJson();
            r["object"] = object->ToJson();
            return r;
        }
    };

    class NumberLiteralInit : public Statement
    {
    public:
        Token tok;
        NumberLiteralInit(Token _tok) : tok(_tok)
        {
        }
        StatementType Type()
        {
            return StatementType::numberLiteralInit;
        }
        Value ToJson()
        {
            return JsObject({{"type", "NumberLiteral"},
                             {"value", tok.ToJson()}});
        }
    };

    class BoolLiteralInit : public Statement
    {
    public:
        Token tok;
        BoolLiteralInit(Token _tok) : tok(_tok)
        {
        }
        StatementType Type()
        {
            return StatementType::boolLiteralInit;
        }
        Value ToJson()
        {
            return JsObject({{"type", "BoolLiteral"},
                             {"value", tok.ToJson()}});
        }
    };

    class StringLiteralInit : public Statement
    {
    public:
        Token tok;
        StringLiteralInit(Token _tok) : tok(_tok)
        {
        }
        StatementType Type()
        {
            return StatementType::stringLiteralInit;
        }
        Value ToJson()
        {
            return JsObject({{"type", "StringLiteral"},
                             {"value", tok.ToJson()}});
        }
    };

    class AssigmentStatement : public Statement
    {
    public:
        Token id;
        StatPtr value;

        StatementType Type()
        {
            return StatementType::assigmentStatement;
        }
        Value ToJson()
        {
            auto r = JsObject();
            r["id"] = id.ToJson();
            r["value"] = value->ToJson();
            r["type"] = "assigmentStatement";
            return r;
        }
    };

    class IfStatment : public Statement
    {
    public:
        // [if,elif,elif,else]
        Vector<TokenFlowView> blocks;
        StatementType Type()
        {
            return StatementType::ifStatment;
        }
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
        template <class T>
        using WithEnd = std::tuple<T, TokenIter>;
        using StatPtrWithEnd = WithEnd<StatPtr>;

    public:
        StatPtr CreateLiteralNode(const Token &tok)
        {
            if (tok.IsNumericLiteral())
            {
                return std::make_shared<NumberLiteralInit>(tok);
            }
            else if (tok.IsBoolLiteral())
            {
                return std::make_shared<BoolLiteralInit>(tok);
            }
            else if (tok.IsStringLiteral())
            {
                return std::make_shared<StringLiteralInit>(tok);
            }
            THROW
        }

        TokenIter Skip(TokenIter iter, KW kw = cr_)
        {
            while (iter->Is(kw))
            {
                iter++;
            }
            return iter;
        }

        // 处理声明语句，只支持使用let const进行单个声明
        WithEnd<std::shared_ptr<VariableDeclaration>> ResolveDeclearStatment(TokenFlowView tfv)
        {
            auto iter = tfv.BeginIter();
            auto var_declear = std::make_shared<VariableDeclaration>(*iter);
            auto id_iter = iter + 1;
            auto decl = std::make_shared<VariableDeclarator>(Identifier(*id_iter));
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

        bool IsExpr(TokenIter iter) const
        {
            return iter->Include(expr_operator);
        }

        // x-1 令牌化 'x' '-1' 这边解析成 x + -1
        StatPtrWithEnd ResolveSubNumberLiteral(StatPtr pos1_stat, TokenFlowView tfv)
        {
            auto iter = tfv.BeginIter();
            auto [pos2_stat, pos2_end_iter] = ResolveExecutableStatment(iter);
            auto stat = std::make_shared<BinaryExpression>();
            stat->left = pos1_stat;
            stat->right = pos2_stat;
            stat->op = Token('+');
            return {stat, pos2_end_iter};
        }

        StatPtrWithEnd ResolveIndex(StatPtr pos1_stat, TokenFlowView tfv)
        {
            auto iter = tfv.BeginIter();
            iter++;
            auto [pos2_stat, pos2_end_iter] = ResolveExecutableStatment(iter);
            auto stat = std::make_shared<IndexStatement>();
            stat->object = std::static_pointer_cast<Identifier>(pos1_stat);
            stat->property = pos2_stat;
            return {stat, pos2_end_iter};
        }

        StatPtrWithEnd ResolveExpr(StatPtr pos1_stat, TokenFlowView tfv)
        {
            auto iter = tfv.BeginIter();
            auto op_iter = iter;
            auto [pos2_stat, pos2_end_iter] = ResolveExecutableStatment(op_iter + 1);
            if (op_iter->Is(question_mask_)) // x ? y : z 三元
            {
                auto stat = std::make_shared<ConditionExpression>();
                stat->cond = pos1_stat;
                stat->left = pos2_stat;
                pos2_end_iter->Expect(colon_);
                auto [pos3_stat, pos3_end_iter] = ResolveExecutableStatment(pos2_end_iter + 1);
                stat->right = pos3_stat;
                return {stat, pos3_end_iter};
            }
            // x(+|-|*|/|++|+=|===。。。)y 二元
            auto stat = std::make_shared<BinaryExpression>();
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
                    stat->body.push_back(body);
                }
                return {stat, end_iter + 1};
            }
            auto [body, end_iter] = ResolveExecutableStatment(iter);
            stat->body.push_back(body);
            return {stat, end_iter};
        }

        //
        StatPtrWithEnd ResolveFuncCall(StatPtr func, TokenFlowView tfv)
        {
            auto iter = tfv.BeginIter(); // (
            auto stat = std::make_shared<FunctionCall>();
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
            return SeekIfExpr(stat, iter);
        }

        StatPtrWithEnd ResolveArrayInit(TokenFlowView tfv)
        {
            auto iter = tfv.BeginIter();
            iter->Expect(brackets_start_);
            iter++;
            auto arr = std::make_shared<ArrayInit>();

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

            if (iter->Is(brackets_start_))
            {
                auto [left_stat, end_iter] = ResolveArrayInit(iter);
                return SeekIfExpr(left_stat, end_iter - 1);
            }

            if (iter->IsLiteral())
            {
                auto left_stat = CreateLiteralNode(*iter);
                return SeekIfExpr(left_stat, iter);
            }
            else if (iter->IsIdentifier())
            {
                auto left_stat = std::make_shared<Identifier>(*iter);
                return SeekIfExpr(left_stat, iter);
            }
            THROW_TOKEN(*iter)
        }

        // 块
        Vector<StatPtr> ResolveBlock(TokenFlowView tfv)
        {
            Vector<StatPtr> res;
            while (true)
            {
            }
            return res;
        }

        WithEnd<IfStatment> ResolveIfStatment(TokenFlowView tfv)
        {
            auto iter = tfv.BeginIter();
            IfStatment res;
            while (true)
            {
                auto block_num = res.blocks.size();
                if (block_num == 0)
                {
                    iter->Expect(if_);
                    auto parenthesis_start = iter + 1;
                    parenthesis_start->Expect(parenthesis_start_);
                    auto parenthesis_end = CalcEndBrackets(parenthesis_start);
                    auto block_start = parenthesis_end + 1;
                    block_start->Expect(curly_brackets_start_);
                    auto block_end = CalcEndBrackets(block_start);
                    res.blocks.push_back(TokenFlowView(block_start, block_end));
                    iter = block_end + 1;
                    if (iter->IsLineEndToken())
                    {
                        break;
                    }
                }
                else
                {
                    iter->Expect(else_);
                    auto iter_next = iter + 1;
                    if (iter_next->Is(if_))
                    {
                        auto parenthesis_start = iter_next + 1;
                        parenthesis_start->Expect(parenthesis_start_);
                        auto parenthesis_end = CalcEndBrackets(parenthesis_start);
                        auto block_start = parenthesis_end + 1;
                        block_start->Expect(curly_brackets_start_);
                        auto block_end = CalcEndBrackets(block_start);
                        res.blocks.push_back(TokenFlowView(block_start, block_end));
                        iter = block_end + 1;
                        if (iter->IsLineEndToken())
                        {
                            break;
                        }
                    }
                    else
                    {
                        auto block_start = iter_next;
                        block_start->Expect(curly_brackets_start_);
                        auto block_end = CalcEndBrackets(block_start);
                        res.blocks.push_back(TokenFlowView(block_start, block_end));
                        iter = block_end + 1;
                        break;
                    }
                };
            }
            return {res, iter};
        }

        StatPtrWithEnd SeekIfExpr(StatPtr left_stat, TokenIter iter)
        {
            // 判断如果后面跟随着一个运算符继续向下折叠
            auto next_iter = iter + 1;
            if (IsExpr(next_iter))
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
            if (next_iter->Is(dot_))
            {
                return ResolveIndex(left_stat, next_iter);
            }
            return {left_stat, next_iter};
        };

        StatPtrWithEnd ResovleAssigmentOrIdentify(TokenFlowView tfv)
        {
            auto iter = tfv.BeginIter();
            auto assigment_or_other = iter + 1;
            if (assigment_or_other->Is(dot_))
            {
            }

            if (assigment_or_other->Is(assigment_))
            {
                auto value_iter = assigment_or_other + 1;
                auto [stat_ptr, end_iter] = ResolveExecutableStatment(value_iter);
                auto assigment = std::make_shared<AssigmentStatement>();
                assigment->id = *iter;
                assigment->value = stat_ptr;
                return {assigment, end_iter};
            }
            return ResolveExecutableStatment(iter);
        }

        StatPtrWithEnd Dispatch(TokenFlowView tfv)
        {
            auto iter = tfv.BeginIter();
            if (iter->IsDeclear())
            {
                return ResolveDeclearStatment(iter);
            }
            else if (iter->Is(if_))
            {
                ResolveIfStatment(iter);
            }
            else if (iter->IsIdentifier())
            {
                return ResovleAssigmentOrIdentify(iter);
            }
            else
            {
                return ResolveExecutableStatment(iter);
            }
            THROW
        }

        Program ConstructAST(Vector<Token> &token_flow)
        {
            Program p;
            Token end_tok;
            end_tok.SetViewEnd();
            token_flow.push_back(end_tok);
            TokenFlowView tfv(token_flow.cbegin(), token_flow.cend());
            auto iter = tfv.BeginIter();
            auto accept = [&](StatPtrWithEnd res)
            {
                auto [stat, end] = res;
                p.body.push_back(stat);
                iter = end;
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
}