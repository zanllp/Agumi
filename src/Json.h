#pragma once

#include "stdafx.h"
#include "JsValue.h"
#include "String.h"

namespace agumi
{
    class Json
    {
    public:
        // 默认开启转义，性能下降
        static String Stringify(const Value &v, int indent = 4, bool escape = true);
        static bool error_if_circle_ref;

    private:
        static String StringifyInternal(const JsObject &next,
                                        std::set<const JsObjectMap *> &json_obj_rec, std::set<const JsArrayVec *> &json_arr_rec,
                                        int indent_step, int indent, bool escape);
        static String StringifyInternalArray(const JsArray &next,
                                             std::set<const JsObjectMap *> &json_obj_rec, std::set<const JsArrayVec *> &json_arr_rec,
                                             int indent_step, int indent, bool escape);
        static String SameLevelCompisition(std::vector<String> &src, int indent_step, int indent, std::tuple<String, String> start_end_symbol);
    };
}