include('../lib/index')

const create_closure = str => {
    const vvv = f(1)
    tt => {
        ee => f('{} {} {} {}',str ,vvv ,tt, ee)
    }
}
const closure_fn = create_closure('hello')
assert(closure_fn('22')(1), 'hello 1 22 1')
assert(closure_fn('33')(2), 'hello 1 33 2')

const create_obj = () => {
    { hello: 'world' }
}

assert(create_obj().hello, 'world')
 
const while = (s,e,fn) => {
    const foo_r = () => {
        fn(s)
        micro(() => while(s+1, e, fn))
    }
    (s < e) ? foo_r() : null
}

while(0, 5, log)


const foo = () => {
    const bar = { cc: {} }
    [bar, () => {
        bar.cc[0]=1
    }]
}

const foo_r = foo()
foo_r[0].cc = [11,2,3]
foo_r[1]()
assert(foo_r[0].cc, [1,2,3])

const multi_chain_test = [0, () => { { f: 'HELLO WORLD' } }]

assert(multi_chain_test[1]().f, 'HELLO WORLD')