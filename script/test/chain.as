include('../lib/index')

const ee =[1,2,3,[1,[2]]]

assert(ee[3][1][0], 2)

const obj = {
    dd: {
        agumi: () => {
                {
                foo: () => 'ciallo'
            }
        }
    }
}

assert(obj.dd.agumi().foo(), 'ciallo')

const arr = [1,2,3, 4, 5]
assert(arr.select(v => v*3).where(v => v>3).range(0, 2)[1], 9)

const inx_test =  {
    foo: () => () => [1,2,42]
}
assert(inx_test.foo()()[2], 42)