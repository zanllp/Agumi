const macro = runInMacroQueue
const micro = runInMicroQueue
const strify = json.stringify
// const ee = v => log(v)
const call = (str,v) => {
    const fn1 = () => {
        () => str + v
    }
    const fn2 = () => "hello " + str + v
    [fn1 , fn2]
}
const fn = call("world","ee")
const fn0 = fn[0]
const fn1 = fn[1]
macro(() => log(fn1()))
