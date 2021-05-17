#include "String.h"
#include "stdafx.h"
#include "JsValue.h"

namespace agumi
{
    String::String() = default;
    String::~String()
    {
    }

    String::String(JsValue arg) : std::string(arg.ToString()) {}

    String::String(int arg) : std::string(std::to_string(arg)) {}

    String::String(unsigned long arg) : std::string(std::to_string(arg)) {}

    String::String(double arg) : std::string(double2stringEarse0(arg)) {}

    String::String(bool arg)
    {
        (*this) = arg ? "true" : "false";
    }

    String::String(char arg)
    {
        (*this) = " ";
        (*this)[0] = arg;
    }

    // 使用字符串分割
    // flag 分割标志,返回的字符串向量会剔除,flag不要用char，会重载不明确
    // num 分割次数，默认-1即分割到结束，例num=1,返回开头到flag,flag到结束size=2的字符串向量
    // skipEmpty 跳过空字符串，即不压入length==0的字符串
    Vector<String> String::Split(String flag, int num, bool skipEmpty) const
    {
        std::vector<String> dataSet;
        auto PushData = [&](String line)
        {
            if (line.length() != 0 || !skipEmpty)
            {
                dataSet.push_back(line);
            }
        };
        auto Pos = FindAll(flag, num);
        if (Pos.size() == 0)
        {
            return std::vector<String>({*this});
        }
        for (auto i = 0; i < Pos.size() + 1; i++)
        {
            if (dataSet.size() == num && Pos.size() > num && num != -1)
            { // 满足数量直接截到结束
                PushData(substr(Pos[dataSet.size()] + flag.size()));
                break;
            }
            if (i == 0)
            { // 第一个数的位置不是0的话补上
                PushData(substr(0, Pos[0]));
            }
            else if (i != Pos.size())
            {
                int Left = Pos[i - 1] + flag.length();
                int Right = Pos[i] - Left;
                PushData(substr(Left, Right));
            }
            else
            { // 最后一个标志到结束
                PushData(substr(*(--Pos.end()) + flag.size()));
            }
        }
        return dataSet;
    }

    // 清除前后的字符
    // target 需要清除的字符默认空格
    String String::Trim(String empty_set) const
    {
        int len = length();
        int left = 0;
        while (left < len && IncludeSym(empty_set, (*this)[left]))
        {
            left++;
        }
        if (left >= len)
        {
            return *this;
        }
        int right = len - 1;
        while (right > 0 && IncludeSym(empty_set, (*this)[right]))
        {
            right--;
        }
        return substr(left, right - left + 1);
    }

    String String::ToLowerCase() const
    {
        String s = *this;
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        return s;
    }

    String String::ToUpperCase() const
    {
        String s = *this;
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                       { return std::toupper(c); });
        return s;
    }
    // 返回搜索到的所有位置
    // flag 定位标志
    // num 搜索数量，默认直到结束
    Vector<int> String::FindAll(String flag, int num) const
    {
        std::vector<int> Result;
        auto Pos = find(flag);
        auto flag_offset = flag.length() == 0 ? 1 : flag.length();
        while (Pos != -1 && Result.size() != num)
        {
            Result.push_back(Pos);
            Pos = find(flag, *(--Result.end()) + flag_offset);
        }
        return Result;
    }

    // 字符串替换
    // oldStr 被替换的字符串
    // newStr 新换上的字符串
    // count 替换次数，默认1，大于0时替换到足够次数或找不到旧字符串为止，小于0时替换到结束
    String &String::Replace(String oldStr, String newStr, int count, int start, int step)
    {
        if (count == 0)
        {
            return *this;
        }
        int pos = find(oldStr, start);
        if (pos == std::string::npos)
        {
            return *this;
        }
        replace(pos, oldStr.length(), newStr);
        return Replace(oldStr, newStr, count < 0 ? -1 : count - 1, pos + step, step);
    }

    String String::Repeat(int num) const
    {
        std::stringstream res;
        for (auto i = num - 1; i >= 0; i--)
        {
            res << *this;
        }
        return res.str();
    }

    String String::Format(String str, std::vector<String> args)
    {
        std::stringstream res;
        auto fragment = str.Split("{}", -1, false);
        for (auto &i : args)
        {
            if (fragment.size() == 0 || (fragment.size() == 1 && fragment[0] == ""))
            {
                break;
            }
            auto front = fragment.front();
            fragment.erase(fragment.begin());
            res << front << i;
        }
        res << fragment.Join("{}");
        return res.str();
    }
    bool String::IncludeSym(const String &syms, char sym)
    {
        for (auto i : syms)
        {
            if (i == sym)
            {
                return true;
            }
        }
        return false;
    }
    bool String::StartWith(String sub)
    {
        return this->find(sub) == 0;
    }

    bool String::Test(String reg) const
    {
        std::smatch m;
        std::regex test_reg(reg);
        std::regex_match(*this, m, test_reg);
        return !m.empty();
    }

    String String::Unescape() const
    {
        return Split("\\\"", -1, false).Join("\"");
    }

    String String::Escape() const
    {
        auto next = *this;
        return next.Replace("\"", "\\\"", -1, 0, 2);
    }
}