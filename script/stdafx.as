const f = format
const s = json.stringify
const f_log = f + log
const full_log = s + log
const and_op = (a, b) => a ? b : a
const to_bool = a => a ? true: false
const macro = runInMacroQueue
const micro = runInMicroQueue
const throw = (msg) => assert(false, msg)
const ass = (a, b) => (s(a) == s(b)) ? null  : throw(f(`assert error: a: {} b: {}`,a,b))
const ass_t = (a) => ass(a, true)
