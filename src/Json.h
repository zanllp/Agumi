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
        static String Stringify(const JsValue &v, int ident = 4, bool escape = true);
        static bool error_if_circle_ref;

    private:
        static String StringifyInternal(const JsObject &next,
                                        std::set<const JsObjectMap *> &json_obj_rec, std::set<const JsArrayVec *> &json_arr_rec,
                                        int ident_step, int ident, bool escape);
        static String StringifyInternalArray(const JsArray &next,
                                             std::set<const JsObjectMap *> &json_obj_rec, std::set<const JsArrayVec *> &json_arr_rec,
                                             int ident_step, int ident, bool escape);
        static String SameLevelCompisition(std::vector<String> &src, int ident_step, int ident, std::tuple<String, String> start_end_symbol);
    };
}