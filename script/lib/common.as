
const throw = raise_native_exception
const assert = (a, b) => {
    const stra = json.stringify(a)
    const strb = json.stringify(b)
    (stra != strb) && throw(f(`assert error: a: {} b: {}`,a,b))
}
const assert_t = (a) => assert(a, true)

array_filter(types, fp.str.not_null).select(type => {
    define_member_function(type, {
        to_string: this => to_str(this),
        to_bool: this => to_bool(this),
        type_str: this => typeof(this),
        is : (this,type) => (this.type_str()) == type
    })
})