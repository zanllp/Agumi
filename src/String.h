#pragma once
#include "stdafx.h"
#include "Vector.h"
namespace agumi
{
    class Value;

    class String : public std::string
    {
    public:
        String();
        ~String();
        template <class T>
        String(T &&arg) : std::string(std::forward<T>(arg)) {}

        template <typename U>
        String(Vector<U> arg) : std::string(arg.Join()) {}

        String(Value arg);

        String(int arg);

        String(bool arg);

        String(unsigned long arg);

        String(double arg);

        String(char arg);

        // 使用字符串分割
        // flag 分割标志,返回的字符串向量会剔除,flag不要用char，会重载不明确
        // num 分割次数，默认-1                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            即分割到结束，例num=1,返回开头到flag,flag到结束size=2的字符串向量
        // skipEmpty 跳过空字符串，即不压入length==0的字符串
        Vector<String> Split(String flag, int num = -1, bool skipEmpty = true, bool utf8 = false) const;

        // 清除前后的字符
        // target 需要清除的字符默认空格
        String Trim(String empty_set = " \n\r\t") const;

        String Unescape() const;
        String Escape() const;
        String ToLowerCase() const;

        String ToUpperCase() const;
        // 返回搜索到的所有位置
        // flag 定位标志
        // num 搜索数量，默认直到结束
        Vector<int> FindAll(String flag, int num = -1) const;

        Vector<String> USpilt() const;
        size_t Ulength() const;
        String USubStr(int start = 0, int count = -1) const;

        // 字符串替换
        // oldStr 被替换的字符串
        // newStr 新换上的字符串
        // count 替换次数，默认1，大于0时替换到足够次数或找不到旧字符串为止，小于0时替换到结束
        String &Replace(String oldStr, String newStr, int count = 1, int start = 0, int step = 1);

        bool StartWith(String sub) const;

        bool Includes(String sub) const;

        static String Format(String str, std::vector<String> args);

        template <typename... Ts>
        static String Format(String str, Ts... args)
        {
            return String::Format(str, {args...});
        }

        static bool IncludeSym(const String &syms, char sym);

        static String FromCodePoint (String hex_str, int base = 16);

        static String FromUtf8EncodeStr (String str);

        String Repeat(int num) const;

        bool Test(String reg) const;
    };
}
