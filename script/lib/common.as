
const f_log = f + log
const full_log = s + log
const throw = raise_native_exception
const assert = (a, b) => (s(a) == s(b)) ? null  : throw(f(`assert error: a: {} b: {}`,a,b))
const assert_t = (a) => assert(a, true)


const types = ['number','boolean','array','object','function','string']
types.select(type => {
    define_member_function(type, {
        to_string: this => to_str(this),
        to_bool: this => to_bool(this),
        type_str: this => typeof(this),
        is : (this,type) => (this.type_str()) == type
    })
})