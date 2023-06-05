
const fp = @{
  const is_not = target => v => v != target
  {
    is_not,
    id: x => x,
    str: {
        not_null: is_not('null'),
        not_func: is_not('function'),
        not_str: is_not('string')
    },
    type: (t) => {
      (val) => typeof(val) == t
    }
  }
}
const is_str = fp.type("string")
const is_num = fp.type("number")
const is_null = fp.type("null")
const is_func = fp.type("function")
const is_bool = fp.type("boolean")
const is_arr = fp.type("array")
const is_obj = fp.type("object")
