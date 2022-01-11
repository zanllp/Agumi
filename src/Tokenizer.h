#pragma once
#include "stdafx.h"
#include "util.h"
#define THROW THROW_MSG("");
#define THROW_TOKEN(tok) THROW_MSG("未知令牌: {}", (tok).ToDebugStr());
#define SRC_REF(name, Type, sptr) auto &name = *static_cast<Type *>(sptr.get());

namespace agumi
{

    String ReadConstStr(String &src, char symbol, int &ptr, int &line, int &offset)
    {
        auto next_ptr = ptr + 1;
        bool need_unescape = false;
        auto len = src.length();
        while (true)
        {
            if (next_ptr == len)
            {
                THROW_MSG("找不到字符串符号 {} 的结束位置，行 {} 列 {}", symbol, line, offset)
            }
            char c = src[next_ptr];
            if (c == symbol)
            {
                if (src[next_ptr - 1] == '\\') // 对字符串符号进行转义
                {
                    need_unescape = true;
                }
                else // 结束
                {
                    break;
                }
            }
            else if (c == '\n')
            {
                if (symbol == '`') // 模板字符串支持多行
                {
                    line++;
                    offset = -1;
                }
                else
                {
                    THROW_MSG("找不到字符串符号 {} 的结束位置，行 {} 列 {}", symbol, line, offset)
                }
            }
            next_ptr++;
            offset++;
        }
        String res = src.substr(ptr, next_ptr - ptr + 1);
        ptr = next_ptr + 1;
        return need_unescape ? res.Unescape() : move(res);
    }

    enum KW
    {
        let_,
        const_,
        var_,
        class_,
        if_,
        else_,
        function_,
        true_,
        fasle_,
        // (
        parenthesis_start_,
        // )
        parenthesis_end_,
        //  [
        brackets_start_,
        // ]
        brackets_end_,
        // {
        curly_brackets_start_,
        // }
        curly_brackets_end_,
        // !
        negate_,
        // =
        assigment_,
        // ，
        comma_,
        // ==
        eqeq_,
        // ===
        eqeqeq_,
        // !=
        not_eq_,
        // !==
        not_eqeq_,
        // null
        null_,
        // :
        colon_,
        // ?
        question_mask_,
        // +
        add_,
        // -
        sub_,
        // *
        mul_,
        // /
        div_,
        // %
        mod_,
        // \n
        cr_,
        // >
        more_than_,
        // <
        less_than_,
        // >=
        more_than_equal_,
        // <=
        less_than_equal_,
        and_and_,
        or_or_,
        // =>
        arrow_,
        // .
        dot_,
        // +=
        add_equal_,
        // ++
        add_add_,
        // -=
        sub_equal_,
        // --
        sub_sub_,

    };
    // 表达式支持的运算符
    Vector<KW> expr_operator{
        question_mask_, add_, sub_, mul_, div_,
        mod_, eqeq_, eqeqeq_, not_eq_, not_eqeq_,
        more_than_, more_than_equal_, less_than_,
        less_than_equal_,
        add_equal_, sub_equal_,
        and_and_,
        or_or_};
    // 多字符的运算符
    Vector<KW> multi_char_operator{
        eqeq_,
        eqeqeq_,
        not_eq_,
        not_eqeq_,
        more_than_equal_,
        less_than_equal_,
        arrow_,
        add_equal_,
        add_add_,
        sub_equal_,
        sub_sub_,
        and_and_,
        or_or_
    };
    class Token : public ViewEnd
    {

    public:
        String kw;
        String file;
        int pos = -1;
        int line = -1;
        int offset = -1;
        bool kw_enum_inited = false;
        KW kw_enum;
        Token(String _kw = "", int _pos = -1, int _line = -1, int _offset = -1, String _file = "") : kw(_kw), pos(_pos), line(_line), offset(_offset), file(_file)
        {
        }
        Value ToJson() const
        {
            auto m = Object();
            m["src"] = kw;
            m["offset"] = offset;
            m["line"] = line;
            m["pos"] = pos;
            m["pos_str"] = ToPosStr();
            m["file"] = file;
            return m;
        }
        String UniqId()
        {
            return file + ":" + String(pos);
        }
        String ToPosStr() const
        {
            if (pos == -1)
            {
                return "[native func]";
            }
            return String::Format("{}:{}:{}", file, line + 1, offset + 1);
        }
        String ToDebugStr() const
        {
            auto _kw = kw;
            if (kw.length() == 1)
            {
                auto kw_char = kw[0];
                if (kw_char == '\n')
                {
                    _kw = "\\n";
                }
                else if (kw_char == '\r')
                {
                    _kw = "\\r";
                }
                else if (kw_char == '\t')
                {
                    _kw = "\\t";
                }
            }
            if (!IsStringLiteral())
            {
                _kw = String::Format("\"{}\"", _kw);
            }
            return String::Format("正文:{} [{}:{}:{}] {}", _kw, file, line + 1, offset + 1, IsEnd() ? " 末尾结束不可访问" : "");
        }

        bool IsLineEndToken() const
        {
            return Include({';', '\n', '\r'});
        }

        bool Is(String sym) const
        {
            return kw == sym;
        }

        bool IsComma() const
        {
            return Is(comma_);
        }

        bool IsEmpty() const
        {
            auto c = kw[0];
            return c == '\n' || c == ' ' || c == '\t';
        }

        // 数字字面量
        bool IsNumericLiteral() const
        {
            return IsNumber(kw);
        }

        bool IsStringLiteral() const
        {
            char begin_char = kw[0];
            char end_char = kw[kw.length() - 1];
            if (begin_char == end_char && String::IncludeSym("\"'`", begin_char))
            {
                return true;
            }
            return false;
        }

        bool IsBoolLiteral() const
        {
            return Include({"true", "false"});
        }

        bool IsNull() const
        {
            return kw == "null";
        }

        bool IsLiteral() const
        {
            return IsStringLiteral() || IsBoolLiteral() || IsNumericLiteral() || IsNull();
        }

        bool Is(KW kw) const
        {
            return Is(Kw2Str(kw));
        }

        //  可用的标识
        bool IsIdentifier() const
        {
            if (IsKeyWord())
            {
                return false;
            }
            return kw.Test(R"(^[_a-zA-Z$][_$\w]*$)");
        }

        KW ToKwEnum()
        {
            if (!kw_enum_inited)
            {
                kw_enum = Token::Str2kw(kw);
                kw_enum_inited = true;
            }
            return kw_enum;
        }

        // 去掉两边的’”·
        String toStringContent(bool ignore_check = false) const
        {
            if (ignore_check || IsStringLiteral())
            {
                return kw.substr(1, kw.size() - 2);
            }
            THROW_MSG("仅允许字符串字面量令牌调用 src:{}", kw);
        }

        double toNumber(bool ignore_check = false) const
        {
            return ignore_check ? std::stod(kw) : StringSafe2Double(kw);
        }

        void Expect(String expect_kw) const
        {
            if (!Is(expect_kw))
            {
                THROW_MSG("未知令牌:\n\t{} \t预期: {}", ToDebugStr(), expect_kw)
            }
        }

        void Expect(KW expect_kw) const
        {
            if (!Is(expect_kw))
            {
                THROW_MSG("未知令牌:\n\t{} 预期:\"{}\"", ToDebugStr(), KeyWordMap.at(expect_kw))
            }
        }

        void Expect(const Vector<KW> &syms) const
        {
            auto kws = syms.Map<String>(Kw2Str);
            if (!kws.Includes(kw))
            {
                THROW_MSG("未知令牌:\n\t{} \t预期: {} 其中任意一个", ToDebugStr(), kws.Join())
            }
        }

        // 包含
        bool Include(const Vector<String> &syms) const
        {
            return syms.Includes(kw);
        }

        // 包含
        bool Include(const Vector<KW> &syms) const
        {
            return syms.Map<String>(Kw2Str).Includes(kw);
        }

        bool IsKeyWord() const
        {
            return KeyWordMap.Includes(kw);
        }

        bool IsDeclear() const
        {
            return Is(let_) || Is(const_); // Is(var_) || 不允许var
        }

        bool operator==(const Token &rhs) const
        {
            return pos == rhs.pos && kw == rhs.kw && line == rhs.line && offset == rhs.offset;
        }

        static Vector<String> KeyWordMap;

        static String Kw2Str(KW kw)
        {
            auto res = KeyWordMap[kw];
            if (res.length() == 0)
            {
                throw std::range_error(String::Format("未定义关键字：{}", (int)kw));
            }

            return res;
        }

        static KW Str2kw(String kw)
        {
            for (size_t i = 0; i < KeyWordMap.size(); i++)
            {
                if (KeyWordMap[i] == kw)
                {
                    return static_cast<KW>(i);
                }
            }
            THROW
        }

        static void Init();
    };

    using TokenFlowView = View<Token>;
    using TokenIter = TokenFlowView::Iter;

    void ThrowLogicError(String msg)
    {
        throw std::logic_error(msg);
    }

    Vector<String> Token::KeyWordMap = {};
    //

    class GeneralTokenizer
    {
        friend Token;

    public:
        enum TokenType
        {
            json,
            agumi
        };
        GeneralTokenizer(const String &src_, TokenType type_ = json, String file_ = GeneralTokenizer::ReplFileName())
        {
            src = src_;
            type = type_;
            file_name = file_;
        }
        Vector<Token> res;
        Vector<Token> &Start()
        {
            Dispatch();
            return res;
        }
        static Vector<Token> Agumi(const String &script, String file = GeneralTokenizer::ReplFileName())
        {

            return GeneralTokenizer(script, GeneralTokenizer::agumi, file)
                .Start()
                .Filter([](Token tok)
                        { return tok.kw != " "; });
        }

        static String ReplFileName()
        {
            return String::Format("repl_{}", ++GeneralTokenizer::uniq_id);
        }

    private:
        Token CreateToken(String src)
        {
            return Token(src, ptr, line, offset, file_name);
        }
        String file_name;
        static const String operator_set;
        static const String empty_set;
        static const String single_char_set;
        static const String const_str_set;
        static int uniq_id;
        void Dispatch()
        {
            while (ptr < src.size())
            {
                char c = src[ptr];
                if (String::IncludeSym(operator_set, c))
                {
                    Operator(c);
                }
                else if ('0' <= c && c <= '9')
                {
                    Number(c);
                }
                else if (String::IncludeSym(empty_set, c))
                {
                    Empty(c);
                }
                else if (String::IncludeSym(single_char_set, c))
                {
                    SingleChar(c);
                }
                else if (String::IncludeSym(const_str_set, c))
                {
                    ConstString(c);
                }
                else
                {
                    Identify(c);
                }
            }
        }
        bool NotLast()
        {
            return ptr < src.size() - 1;
        }
        char Next()
        {
            return src[ptr + 1];
        }
        void Operator(char c)
        {
            if (c == '-' && NotLast()) // 负号开头但实际是个负数
            {
                char cn = Next();
                if ('0' <= cn && cn <= '9')
                {
                    return Number(c);
                }
            }
            else if (c == '/' && NotLast())
            {
                char cn = Next();
                if ('/' == cn || cn == '*')
                {
                    return Comment(c, cn);
                }
            }
            auto tok = CreateToken(c);
            if (type == agumi) // agumi才贪婪匹配
            {
                auto filter_fn = [&](String s)
                {
                    for (size_t i = 0; i < s.size(); i++)
                    {
                        if (src[ptr + i] != s[i])
                        {
                            return false;
                        }
                    }
                    return true;
                };
                auto match = multi_token_set.Filter(filter_fn).Sort(CompareStrLen);
                if (match.size())
                {
                    auto match_kw = match[0];
                    tok.kw = match_kw;
                    auto compensate = match_kw.size() - 1;
                    ptr += compensate;
                    offset += compensate;
                }
            }
            ptr++;
            offset++;
            res.push_back(tok);
        }
        void Number(char c)
        {
            auto tok = CreateToken(c);
            auto ptr_next = ptr + 1;
            auto len = src.size();
            while (ptr_next < len)
            {
                char c_next = src[ptr_next];
                if (!(
                        ('0' <= c_next && c_next <= '9') ||
                        c_next == '.'))
                {
                    break;
                }
                ptr_next++;
                offset++;
            }
            tok.kw = src.substr(ptr, ptr_next - ptr);
            ptr = ptr_next;
            res.push_back(tok);
        }
        void Comment(char c, char cn)
        {
            auto tok = CreateToken(c); // 暂时用不到注释的令牌
            auto is_single_line = cn == '/';
            auto push_token_if_needed = [&]
            {
                return;
                if (true) // 暂时不需要注释的信息
                {
                    return;
                }
                // tok.kw = src.substr(tok.pos, ptr - tok.pos);
                // res.push_back(tok);
            };
            for (size_t i = ptr; i < src.size(); i++)
            {
                char c = src[i];
                if (c == '\n')
                {
                    if (is_single_line)
                    {
                        ptr = i; // 移到 \n
                        return push_token_if_needed();
                    }
                    line++;
                    offset = 0;
                }
                else if (c == '*' && src[i + 1] == '/' && !is_single_line)
                {
                    ptr = i + 2; // 移到 */ 的下一位
                    return push_token_if_needed();
                }
                else
                {
                    offset++;
                }
            }
            if (is_single_line) // 最后一行是注释
            {
                ptr = src.size() - 1; // 移到 \n
                return push_token_if_needed();
            }
            THROW_MSG("找不到注释的结束符号: {}", tok.ToDebugStr());
        }
        void Identify(char c)
        {
            auto tok = CreateToken(c);
            auto ptr_next = ptr + 1;
            offset++;
            auto len = src.size();
            while (ptr_next < len)
            {
                char c_next = src[ptr_next];
                if (!(
                        ('a' <= c_next && c_next <= 'z') ||
                        ('A' <= c_next && c_next <= 'Z') ||
                        ('0' <= c_next && c_next <= '9') ||
                        c_next == '_' ||
                        c_next == '$'))
                {
                    break;
                }
                ptr_next++;
                offset++;
            }
            tok.kw = src.substr(ptr, ptr_next - ptr);
            ptr = ptr_next;
            res.push_back(tok);
        }
        void Empty(char c)
        {
            if (type != json)
            {
                auto tok = CreateToken(c);
                res.push_back(tok);
            }
            ptr++;
            if (c == '\n')
            {
                line++;
                offset = 0;
            }
            else
            {
                offset++;
            }
        }
        void SingleChar(char c)
        {
            auto tok = CreateToken(c);
            res.push_back(tok);
            ptr++;
            offset++;
        }
        void ConstString(char c)
        {
            auto kw = ReadConstStr(src, c, ptr, line, offset);
            auto tok = CreateToken(kw);
            res.push_back(tok);
        }
        int ptr = 0;
        int line = 0;
        int offset = 0;

        static bool CompareStrLen(String s1, String s2)
        {
            return s1.length() > s2.length();
        }

        String src;
        TokenType type;
        static Vector<String> multi_token_set;
    };

    Vector<String> GeneralTokenizer::multi_token_set;
    const String GeneralTokenizer::operator_set = "!+-*/(){}[]=><%&|";
    const String GeneralTokenizer::empty_set = " \n\t";
    const String GeneralTokenizer::single_char_set = ";,:.?";
    const String GeneralTokenizer::const_str_set = R"("'`)";
    int GeneralTokenizer::uniq_id = 0;
    void Token::Init()
    {
        auto &m = KeyWordMap;
        m.resize(64);
        m[let_] = "let";
        m[var_] = "var";
        m[const_] = "const";
        m[class_] = "class";
        m[if_] = "if";
        m[else_] = "else";
        m[function_] = "function";
        m[true_] = "true";
        m[fasle_] = "false";
        m[parenthesis_start_] = "(";
        m[and_and_] = "&&";
        m[or_or_] = "||";
        m[parenthesis_end_] = ")";
        m[brackets_start_] = "[";
        m[brackets_end_] = "]";
        m[curly_brackets_start_] = "{";
        m[curly_brackets_end_] = "}";
        m[comma_] = ",";
        m[assigment_] = '=';
        m[eqeq_] = "==";
        m[eqeqeq_] = "===";
        m[not_eq_] = "!=";
        m[not_eqeq_] = "!==";
        m[negate_] = "!";
        m[null_] = "null";
        m[colon_] = ':';
        m[question_mask_] = '?';
        m[add_] = '+';
        m[sub_] = '-';
        m[mul_] = '*';
        m[div_] = '/';
        m[cr_] = '\n';
        m[mod_] = '%';
        m[more_than_] = '>';
        m[less_than_] = '<';
        m[more_than_equal_] = ">=";
        m[less_than_equal_] = "<=";
        m[arrow_] = "=>";
        m[dot_] = '.';
        m[add_add_] = "++";
        m[add_equal_] = "+=";
        m[sub_sub_] = "--";
        m[sub_equal_] = "-=";
        for (int i = m.size() - 1; i >= 0; i--)
        {
            if (m[i] != "")
            {
                m.resize(i + 1);
                break;
            }
        }
        // 连续的符号
        for (auto &i : multi_char_operator)
        {
            GeneralTokenizer::multi_token_set.push_back(m[i]);
        }
    }
}