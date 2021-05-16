#include "Json.h"
#include "JsValue.h"
#include "JsObject.h"
#include "util.h"

bool Json::error_if_circle_ref = false;
String Json::StringifyInternalArray(const JsArray &next,
                                    set<const JsObjectMap *> &json_obj_rec, set<const JsArrayVec *> &json_arr_rec,
                                    int ident_step, int ident, bool escape)
{
    json_arr_rec.insert(next.Ptr());
    vector<String> res_vec;
    auto handle_circle_ref = [&] {
        if (error_if_circle_ref)
        {
            THROW_MSG("循环引用");
        }
        res_vec.push_back("\"circle ref\"");
    };
    int next_ident = ident + ident_step;
    for (auto &i : next.SrcC())
    {
        auto type = i.Type();
        if (type == JsType::object)
        {
            if (json_obj_rec.find(i.ObjectC().Ptr()) == json_obj_rec.end())
            {
                auto next_v = Json::StringifyInternal(i.ObjectC(), json_obj_rec, json_arr_rec, ident_step, next_ident, escape);
                res_vec.push_back(next_v);
            }
            else
            {
                handle_circle_ref();
            }
        }
        else if (type == JsType::array)
        {
            if (json_arr_rec.find(i.ArrayC().Ptr()) == json_arr_rec.end())
            {
                auto next_v = Json::StringifyInternalArray(i.ArrayC(), json_obj_rec, json_arr_rec, ident_step, next_ident, escape);
                res_vec.push_back(next_v);
            }
            else
            {
                handle_circle_ref();
            }
        }
        else
        {
            auto v_src = type == JsType::undefined ? "null" : i.ToString();
            auto str = escape ? v_src.Escape() : v_src;
            auto value_str = i.Type() == JsType::string ? String::Format("\"{}\"", str) : str;
            res_vec.push_back(value_str);
        }
    }
    return SameLevelCompisition(res_vec, ident_step, next_ident, {"[", "]"});
}
String Json::StringifyInternal(const JsObject &next,
                               set<const JsObjectMap *> &json_obj_rec, set<const JsArrayVec *> &json_arr_rec,
                               int ident_step, int ident, bool escape)
{
    json_obj_rec.insert(next.Ptr());
    vector<String> res_vec;
    auto handle_circle_ref = [&](String key) {
        if (error_if_circle_ref)
        {
            THROW_MSG("key:{} 循环引用", key);
        }
        res_vec.push_back(String::Format("\"{}\": \"circle ref\"", key));
    };
    int next_ident = ident + ident_step;
    for (auto &i : next.SrcC())
    {
        auto key = escape ? i.first.Escape() : i.first;
        auto type = i.second.Type();
        if (type == JsType::object)
        {
            if (json_obj_rec.find(i.second.ObjectC().Ptr()) == json_obj_rec.end())
            {
                auto next_v = Json::StringifyInternal(i.second.ObjectC(), json_obj_rec, json_arr_rec, ident_step, next_ident, escape);
                auto str = String::Format("\"{}\": {}", key, next_v);
                res_vec.push_back(str);
            }
            else
            {
                handle_circle_ref(key);
            }
        }
        else if (type == JsType::array)
        {
            if (json_arr_rec.find(i.second.ArrayC().Ptr()) == json_arr_rec.end())
            {
                auto next_v = Json::StringifyInternalArray(i.second.ArrayC(), json_obj_rec, json_arr_rec, ident_step, next_ident, escape);
                auto str = String::Format("\"{}\": {}", key, next_v);
                res_vec.push_back(str);
            }
            else
            {
                handle_circle_ref(key);
            }
        }
        else
        {
            String v_src = (type == JsType::undefined) ? "null" : i.second.ToString();
            auto str = escape ? v_src.Escape() : v_src;
            auto value_str = i.second.Type() == JsType::string ? String::Format("\"{}\"", str.Replace('\n',"\\n", -1)) : str;
            res_vec.push_back(String::Format("\"{}\": {}", key, value_str));
        }
    }
    return SameLevelCompisition(res_vec, ident_step, next_ident, {"{", "}"});
}

String Json::SameLevelCompisition(vector<String> &src_vec, int ident_step, int ident, std::tuple<String, String> start_end_symbol)
{
    bool zero_ident = ident_step == 0;
    bool is_empty = src_vec.size() == 0;
    String cr_if_not_zero_ident = (zero_ident || is_empty) ? "" : "\n";
    stringstream res;
    res << get<0>(start_end_symbol) << cr_if_not_zero_ident;
    int vec_size = src_vec.size();
    for (size_t i = 0; i < vec_size; i++)
    {
        res << (zero_ident ? "" : String(" ").Repeat(ident));
        res << src_vec[i];
        if (i != src_vec.size() - 1)
        {
            res << ",";
        }
        res << cr_if_not_zero_ident;
    }
    res << ((zero_ident || is_empty) ? "" : String(" ").Repeat(ident - ident_step));
    res << get<1>(start_end_symbol);
    return res.str();
}

JsValue Json::Parse(const String &str_src)
{
    return nullptr;
}

String Json::Stringify(const JsValue &v, int ident, bool escape)
{
    set<const JsObjectMap *> json_obj_rec;
    set<const JsArrayVec *> json_arr_rec;
    switch (v.Type())
    {
    case JsType::object:
        return Json::StringifyInternal(v.ObjectC(), json_obj_rec, json_arr_rec, ident, 0, escape);
    case JsType::array:
        return Json::StringifyInternalArray(v.ArrayC(), json_obj_rec, json_arr_rec, ident, 0, escape);
    case JsType::string:
        return String::Format("\"{}\"", v.ToString());
    default:
        return v.ToString();
    }
}
