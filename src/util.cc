#include "util.h"

namespace agumi
{
    void Assest(bool condition, String msg)
    {
        if (!condition)
        {
            throw std::logic_error(msg);
        }
    }
    String double2stringEarse0(double val)
    {
        std::string s = std::to_string(val);
        int dot_pos = s.find('.');
        if (dot_pos == -1)
        {
            return s;
        }
        int continuous_zero_count = 0;
        for (auto i = dot_pos; i < s.length(); i++)
        {
            if (s[i] == '0')
            {
                continuous_zero_count++;
                if (continuous_zero_count == 3)
                {
                    int end_pos = i - continuous_zero_count;
                    return s.substr(0, dot_pos == end_pos ? end_pos : end_pos + 1);
                }
            }
            else
            {
                continuous_zero_count = 0;
            }
        }
        return s;
    }

    bool IsNumber(const String &src)
    {
        for (size_t i = 0; i < src.size(); i++)
        {
            char c = src[i];
            if (c >= '0' && c <= '9')
            {
            }
            else if (i == 0 && c == '-')
            {
            }
            else if (c == '.' && i != 0)
            {
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    Vector<String> CreateVecFromStartParams(int argc, char **argv)
    {
        return Vector<String>::From(std::vector<String>(argv, argv + argc));
    }

    double StringSafe2Double(String str)
    {
        if (IsNumber(str))
        {
            return stod(str);
        }
        throw std::range_error(String::Format("未知令牌：{}", str));
    }

    String LoadFile(String path)
    {
        std::fstream file(path, std::ios_base::in);
        std::string res;
        const int size = 1000;
        std::array<char, size> buf;
        buf.fill(0);
        while (file.good())
        {
            file.read(&buf.at(0), size - 1);
            res += &buf.at(0);
            buf.fill(0);
        }
        return res;
    }
}