#pragma once
#include "stdafx.h"
#include "JsValue.h"
#include "String.h"

class Json
{
public:
    static JsValue Parse(const String &str);
    // 默认开启转义，性能下降
    static String Stringify(const JsValue &v, int ident = 4, bool escape = true);
    static bool error_if_circle_ref;

private:
    static String StringifyInternal(const JsObject &next,
                                    set<const JsObjectMap *> &json_obj_rec, set<const JsArrayVec *> &json_arr_rec,
                                    int ident_step, int ident, bool escape);
    static String StringifyInternalArray(const JsArray &next,
                                         set<const JsObjectMap *> &json_obj_rec, set<const JsArrayVec *> &json_arr_rec,
                                         int ident_step, int ident, bool escape);
    static String SameLevelCompisition(vector<String> &src, int ident_step, int ident, std::tuple<String, String> start_end_symbol);
};
