
const fn = (i) => {
    [1,2,3,4,i, "hello world"]
}
const c = fn(0)
log(json.stringify(c, 0))
const createFunc = () => {
    const arr = [1,2,3,4, fn(1)]
    () => arr
}
const fn1 = createFunc()
assert((fn1().get(1)) == 2)
assert(fn1() == fn1())
assert([] != [])
const arr = [1,2,3,4, [1,2,3]]
assert((arr[0]) == 1)
assert((arr[2+2][0]) == 1)
assert((b['dd']['o3']['hello']['d']) == 'hello world')