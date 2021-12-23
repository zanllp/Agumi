
const throw = (msg) => assert(false, msg)
const ass = (a, b) => (s(a) == s(b)) ? null  : throw(f(`assert error: a: {} b: {}`,a,b))
const ass_t = (a) => ass(a, true)


const types = ['number','boolean','array','object','function','string']
types.select(type => {
    define_member_function(type, {
        to_string: this => f(this)
    })
})
