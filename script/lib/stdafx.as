const f = format
const s = json.stringify
const f_log = f + log
const full_log = s + log
const and = (a, b) => a ? b : a
const not = (a) => a ? false : true
const macro = runInMacroQueue
const micro = runInMicroQueue
const if_exec_else = (cond, fn, elfn) => cond ? fn() : elfn() 
const if_exec = (cond, fn) => cond ? fn() : null

const to_bool = a => {
    a ? true: false
}
