const macro = runInMacroQueue
const micro = runInMicroQueue
const strify = json.stringify
// const ee = v => log(v)
const call = (str) => {
    const vvv = f(1)
    (tt) => {
        log(f('{} aaa', str, vvv))
        () => str + vvv + tt
    }
}
const f = format
const fn = call('hello')('ee')()
log(fn)
log(f('hello {}', 'world'))