@{
    const fn = (d = 1) => {
        d
    }
    assert(fn(), 1)
    assert(fn(2), 2)
}
@{
    const fn = (a,b = 1 + 1) => {
        a + b
    }
    assert(fn(1), 3)
    assert(fn(2,3), 5)
}
@{
    const tri = 3
    const fn = (a, b=1, c=2, d=tri) => {
        a + b + c + d
    }
    assert(fn(0), 6)
    assert(fn(1,2,3), 9)
}
