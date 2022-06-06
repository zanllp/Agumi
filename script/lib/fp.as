
const __is_not = target => v => v != target
const fp = {
    is_not: __is_not,
    str: {
        not_null: __is_not('null'),
        not_func: __is_not('function'),
        not_str: __is_not('string')
    }
}