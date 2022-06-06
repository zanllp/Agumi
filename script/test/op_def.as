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