#pragma once
#include "Parser.h"
namespace agumi
{
    class JsonNext
    {
        // tfv
        void ResolveObject(const TokenFlowView &tfv, JsObject &obj)
        {
            auto iter = tfv.BeginIter(); // {
            iter++;
            if (tfv.IsEnd(iter))
            {
                return;
            }
            while (true)
            {
                if (!iter->IsStringLiteral())
                {
                    THROW_TOKEN(*iter);
                }
                auto key = iter->toStringContent(true);
                iter++; // 移到冒号
                iter->Expect(colon_);
                iter++; // 移动到值
                auto val = ResolveValue(iter);
                obj[key] = std::get<0>(val);
                iter = std::get<1>(val) + 1; // } 或者 ,
                if (tfv.IsEnd(iter))
                {
                    break;
                }
                else
                {
                    iter->Expect(comma_);
                    iter++;
                }
            }
        }

        void ResolveArray(const TokenFlowView &tfv, Array &arr)
        {
            auto iter = tfv.BeginIter();
            iter++;
            if (tfv.IsEnd(iter))
            {
                return;
            }
            while (true)
            {
                auto [val, end_iter] = ResolveValue(iter);
                arr.Src().push_back(val);
                iter = end_iter + 1; // } 或者 ,
                if (tfv.IsEnd(iter))
                {
                    break;
                }
                else
                {
                    iter->Expect(comma_);
                    iter++;
                }
            }
        }

        std::tuple<Value, TokenIter> ResolveValue(const TokenFlowView &tfv)
        {
            Value res;
            auto f = tfv.BeginIter();
            auto end = f;
            if (f->Is(brackets_start_))
            {
                end = CalcEndBrackets(f);
                Array arr;
                res = arr;
                TokenFlowView tfv_n(f, end);
                ResolveArray(tfv_n, arr);
            }
            else if (f->Is(curly_brackets_start_))
            {
                end = CalcEndBrackets(f);
                JsObject obj;
                res = obj;
                TokenFlowView tfv_n(f, end);
                ResolveObject(tfv_n, obj);
            }
            else if (f->Is(null_))
            {
                res = nullptr;
            }
            else if (f->IsStringLiteral() && f->kw[0] == '"')
            {
                res = f->toStringContent(true);
            }
            else if (f->IsNumericLiteral())
            {
                res = f->toNumber(true);
            }
            else if (f->IsBoolLiteral())
            {
                res = f->kw == "true" ? true : false;
            }
            else
            {
                THROW_TOKEN(*f);
            }
            return {res, end};
        }

    public:
        Value JsonParse(const String &src)
        {
            Value res;
            const auto token_flow = GeneralTokenizer(src).Start();
            if (token_flow.size() == 0)
            {
                return res;
            }
            auto f = token_flow.begin();
            auto root_value = ResolveValue(f);
            res = std::get<0>(root_value);
            auto root_type = res.Type();
            if (token_flow.size() != 1 && root_type != JsType::array && root_type != JsType::object)
            {
                THROW_TOKEN(token_flow[1]);
            }
            return res;
        }
    };
}