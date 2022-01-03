#pragma once
#include "stdafx.h"

namespace agumi
{
    void ThrowError(const char *msg);
    void ThrowError(std::string msg);

    double StringSafe2Double(String str);
    template <typename T>
    void Assest(T p1, T p2, String msg = "")
    {
        if (p1 != p2)
        {
            throw std::logic_error(msg);
        }
    }
    bool IsNumber(const String &src);
    void Assest(bool condition, String msg = "");

// 断言宏
#define ASS(l, r) Assest(l == r, String::Format("断言错误 实际:{} 预期:{} 发生在: {}:{} ", l, r, __FILE__, __LINE__));
// 默认true的断言
#define ASS_T(l) ASS(l, true);
//针对一些String未实现转换的类型可以尝试使用这个
#define ASS_2UL(l, r) ASS((unsigned long)l, (unsigned long)r);
    Vector<String> CreateVecFromStartParams(int argc, char **argv);
// 可以单独加原因的ASS
#define ASS2(cond, s) Assest(cond, String::Format("断言错误:{} 发生在: {}:{}", s, __FILE__, __LINE__));

    String double2stringEarse0(double val);

    String LoadFile(String path);
    /**
     * 视图
     * T必须继承ViewEnd,并插入一个末尾的
     * @example
     * Token end_tok;
     * end_tok.SetViewEnd();
     * token_flow.push_back(end_tok);
     * TokenFlowView tfv(token_flow.cbegin(), token_flow.cend());
     *
     */
    template <class T>
    struct View
    {
        using Iter = typename std::vector<T>::const_iterator;
        View(Iter _begin)
        {
            begin = _begin;
        }

        View(Iter _begin, Iter _end)
        {
            begin = _begin;
            end = _end;
            is_init_end = true;
        }
        Iter begin;
        bool is_init_end = false;
        Iter end;
        Iter BeginIter() const
        {
            return begin;
        }

        bool IsEnd(Iter iter) const
        {
            if (is_init_end)
            {
                return iter == end || iter->IsEnd();
            }
            return iter->IsEnd();
        }
    };

    class ViewEnd
    {
    public:
        void SetViewEnd()
        {
            _is_view_end = true;
        }
        bool IsEnd() const
        {
            return _is_view_end;
        }

    private:
        bool _is_view_end = false;
    };

    String PathCalc(Vector<String> paths, String separate = '/');

    template <typename... Ts>
    String PathCalc(Ts... args)
    {
        return PathCalc({args...});
    }

    double get_thread_id();
}