#pragma once

#include "String.h"
#include "Value.h"
#include "stdafx.h"

namespace agumi
{
class JsonStringify
{
  public:
    // 默认开启转义，性能下降
    static String Call(const Value& v, int indent = 4, bool escape = true);
    static bool error_if_circle_ref;

  private:
    static String StringifyInternal(const Object& next, std::set<long> json_ptr_rec, int indent_step, int indent, bool escape);
    static String StringifyInternalArray(const Array& next, std::set<long> json_ptr_rec, int indent_step, int indent, bool escape);
    static String SameLevelCompisition(std::vector<String>& src, int indent_step, int indent, std::tuple<String, String> start_end_symbol);
};
} // namespace agumi
