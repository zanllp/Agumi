define_operator('null', 'null', '==', () => true)
['number','boolean','array','object','function','string'].select(type => {
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

['number','boolean','object','function','string', 'null'].select(type => {
    define_operator(type, 'string', '->', (l, r) => f(r,l))
})

define_operator('array', 'string', '->', (l, r) => apply(f, [r].concat(l)))
define_operator('function', 'function', '+', (l,r) => (a) =>  r(l(a)))

['array','number','boolean','object','string', 'null'].select(type => {
    define_operator(type, 'function', '->', (l, r) => r(l))
})
define_operator('function', 'function', '->', (l,r) => (a) =>  r(l(a)))