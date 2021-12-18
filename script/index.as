include('./stdafx.as')
include('./kv.as')
include('./promise.as')
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

defineMemberFunction('array','range', (this, start, count) => {
    let res = []
    this.select((v,i) => {
        if_exec(and_op(i>=start, i < (start + count)), () => {
            res.push(v)
        })
    })
    res
})
defineMemberFunction('array','join', (this, spec) => {
    let res = f(this[0])
    const arr = this.range(1,(this.length()) - 1)
    arr.select((v,i) => {
        res = res + f(spec) + f(v)
    })
    res
})
defineMemberFunction('array','find_index', (this, target) => {
    let res = -1
    this.select((v,i) => if_exec(v == target , () => {
        res=i
    }))
    res
})
const arr = [1,2,3,4]
log(arr.join(','))
log(arr.find_index(3))