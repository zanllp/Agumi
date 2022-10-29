assert(!1, false)
assert(![], false)
assert(!"hello", false)
assert(!"", true)
assert(!0, true)
@{
    const arr = 1..100
    let v = 0
    arr.select(() => {
        v = v  + 1
    })
    assert(v, 99)
}

@{
    const state = { v: 1 }
    const effect = () => {
        state.v = 2
    }
    null && effect() // effect不执行
    assert(state.v, 1)
    null || effect() // 执行
    assert(state.v, 2)
    state.v = 1
    1 || effect() // 不执行
    assert(state.v, 1)
}