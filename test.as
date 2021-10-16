const macro = runInMacroQueue
const micro = runInMicroQueue
const strify = json.stringify
// const ee = v => log(v)
const call = (str) => macro(() => log(str))
call('hello')