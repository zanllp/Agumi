#pragma once
#include "stdafx.h"

namespace agumi
{
    template <typename T>
    class Vector : public std::vector<T>
    {
    private:
        /* data */
    public:
        Vector()
        {
        }

        Vector(std::vector<T> vec) : std::vector<T>(vec)
        {
        }
        Vector(std::initializer_list<T> vec) : std::vector<T>(vec)
        {
        }

        ~Vector()
        {
        }

        Vector<T> Copy()
        {

            return {};
        }

        Vector<T> Filter(std::function<bool(T)> cond) const
        {
            Vector<T> res;
            for (const auto &i : *this)
            {
                if (cond(i))
                {
                    res.push_back(i);
                }
            }
            return res;
        }

        Vector<T> Foreach(std::function<void(T)> fn)
        {
            for (const auto &i : *this)
            {
                fn(i);
            }
            return *this;
        }

        Vector<T> Sort(std::function<bool(T, T)> fn)
        {
            std::sort(this->begin(), this->end(), fn);
            return *this;
        }

        Vector<T> Sort()
        {
            std::sort(this->begin(), this->end());
            return *this;
        }

        template <typename U>
        Vector<U> Map(std::function<U(T)> fn) const
        {
            Vector<U> res;
            for (const auto &i : *this)
            {
                res.push_back(fn(i));
            }
            return res;
        }

        std::string Join(std::string separator = ",") const
        {
            std::stringstream ss;
            size_t count = 0;
            for (const auto &i : *this)
            {
                ss << i << (++count == this->size() ? "" : separator);
            }
            return ss.str();
        }

        bool Includes(const T &target) const
        {
            return find(this->begin(), this->end(), target) != this->end();
        }

        T GetOrDefault(size_t idx)
        {
            if (idx < this->size())
            {
                return (*this)[idx];
            }
            return T();
        }

        T GetOr(size_t idx, T backup)
        {
            if (idx < this->size())
            {
                return (*this)[idx];
            }
            return backup;
        }

        Vector<T> Slice(size_t start, size_t end = -1)
        {
            Vector<T> res;
            for (size_t i = start; i != end; i++)
            {
                if (this->size() == i && end == -1)
                {
                    break;
                }
                res.push_back((*this)[i]);
            }
            return res;
        }

        static Vector<T> From(std::vector<T> vec)
        {
            return vec;
        }
    };
}