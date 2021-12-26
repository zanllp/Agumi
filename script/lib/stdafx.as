const f = format
const s = json.stringify
const f_log = f + log
const full_log = s + log
const and_op = (a, b) => a ? b : a
const macro = runInMacroQueue
const micro = runInMicroQueue
const if_exec_else = (cond, fn, elfn) => cond ? fn() : elfn() 
const if_exec = (cond, fn) => cond ? fn() : null

const to_bool = a => {
    a ? true: false
}
