#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <functional>
#include <sstream>
#include <tuple>
#include <regex>
#include <fstream>
#include <chrono>
#include <cmath>
#include <array>
#include <queue>
#include <optional>
#include "String.h"
#include "Vector.h"
class String;
enum class JsType
{
    string,
    number,
    undefined,
    null,
    boolean,
    object,
    array,
    function
    //  bigint,
    // symbol
};
static std::vector<std::string> jstype_emun2str {
    "string",
    "number",
    "undefined",
    "null",
    "boolean",
    "JsObject",
    "array",
    "function"};
class JsValue;
using JsObjectMap = std::map<String, JsValue>;
using JsArrayVec = Vector<JsValue>;

#define THROW_MSG(msg, ...) throw logic_error(String::Format("{} \n\t发生在: {}:{} ", String::Format(msg, {__VA_ARGS__}), __FILE__, __LINE__));
