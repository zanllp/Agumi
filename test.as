
const fn = (i) => {
    [1,2,3,4,i, "hello world"]
}
const c = fn(0)
log(json.stringify(c, 0))
const createFunc = (arr) => () => arr
const fn1 = createFunc([1,2,3,4,5])
log(fn1() == fn1())
log([] == [])