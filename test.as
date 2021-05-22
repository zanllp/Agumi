
const fn = (i) => {
    [1,2,3,4,i, "hello world"]
}
const c = fn(0)
log(json.stringify(c, 0))