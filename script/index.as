include('./stdafx.as')
include('./kv.as')
include('./promise.as')
include('./index.spec.as')
const socket_push = (data) => {
    make_promise(resolve => {
        const resp = fetch('https://api.ioflow.link/socket/push?descriptor=IwPYC8kUSeUHDEdT', {
            data,
            method: 'post',
            headers: { 'Content-Type': 'application/json' }
        })
        resolve(resp)
    })
}

const resp = socket_push([])
resp.then(data => {
    storage.set_item('resp', data)
    log(s(storage.get_item('resp')))
})

define_member_function('array','range', (this, start, count) => {
    let res = []
    this.select((v,i) => {
        if_exec(and_op(i>=start, i < (start + count)), () => {
            res.push(v)
        })
    })
    res
})
define_member_function('array','join', (this, spec) => {
    let res = f(this[0])
    const arr = this.range(1,(this.length()) - 1)
    arr.select((v,i) => {
        res = res + f(spec) + f(v)
    })
    res
})
define_member_function('array','find_index', (this, target) => {
    let res = -1
    this.select((v,i) => if_exec(v == target , () => {
        res=i
    }))
    res
})
const arr = [1,2,3,4]
full_log(arr.range(1, 2))
full_log(arr.range(0, 3))
log(arr.join(','), arr.join('_'))
log(arr.find_index(3))

define_member_function('object','entires', (this) => object_entries(this))
define_member_function('object','values', (this) => {
    const entires = this.entires()
    entires.select(v => v.v)
})
define_member_function('object','keys',  (this) => {
    const entires = this.entires()
    entires.select(item => item.k)
})
define_member_function('object', 'has', (this,key) => {
    const keys = this.keys()
    (keys.find_index(key)) != -1
})
const aaa = mem().b
full_log(aaa)
log(s(aaa.keys()))
log(aaa.has('typeof'))

const debug = make_ability('Debug')

define_member_function(debug, {
    print: (this) => log(s(this)),
    equal: (this, o) =>  {
        s(this) == s(o)
    }
})

const foo = { hello: 'ciallo' }
use_ability(foo, debug)

foo.print()
log(foo.equal({}))
