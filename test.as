const macro = runInMacroQueue
const micro = runInMicroQueue
const strify = json.stringify
// const ee = v => log(v)
const call = (str) => {
    () => {
        str
    }
}
const f = format
const fn = call('hello')()
log(fn)
log(f('hello {}', 'world'))