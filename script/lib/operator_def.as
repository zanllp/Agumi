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
