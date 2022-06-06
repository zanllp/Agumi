const array_filter = (arr, predicate) => {
    const res = []
    arr.select((v,i) => predicate(v,i) ? res.push(v) : null)
    res
}  
const types = ['number','boolean','array','object','function','string', 'null']


define_operator('null', 'null', '==', () => true)


array_filter(types, fp.str.not_null).select(type => {
    define_operator('null', type, '==', () => false)
    define_operator(type, 'null', '==', () => false)
})

define_operator('string', 'number', '*', (l,r) => {
  const arr = []
  arr.resize(r)
  arr.select((_,i) => {
    arr[i] = l
  })
  arr.join('')
})

define_operator('string', 'number', '+', (l,r) => l + to_str(r))

array_filter(types, fp.str.not_func).select(type => {
    define_operator(type, 'string', '->', (l, r) => f(r,l))
})

define_operator('array', 'string', '->', (l, r) => apply(f, [r].concat(l)))
define_operator('function', 'function', '+', (l,r) => (a) =>  r(l(a)))

array_filter(types, fp.str.not_func).select(type => {
    define_operator(type, 'function', '->', (l, r) => r(l))
})
const func_pipe_mem = {}
define_operator('function', 'function', '->', (l,r) => {
    const key = to_str(l) + to_str(r)
    (func_pipe_mem.has(key)) ? @{
        func_pipe_mem[key]
    } :  @{
        const res = (a) => r(l(a))
        func_pipe_mem[key] = res
        res
    }
})

types.select(type => {
    define_operator(type, '!', v => v ? false : true)
})

define_operator('number', 'number', '..', (l,r) => {
    const len = r - l
    assert_t(len > 0)
    const res = []
    res.resize(len)
    res.select((_,i) => i + l)
})