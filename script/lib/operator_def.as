define_operator('null', 'null', '==', () => true)
['number','boolean','array','object','function','string'].select(type => {
    define_operator('null', type, '==', () => false)
    define_operator(type, 'null', '==', () => false)
})