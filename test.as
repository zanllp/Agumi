const macro = runInMacroQueue
const micro = runInMicroQueue
const f = format
const create_closure = str => {
    const vvv = f(1)
    tt => {
        ee => f('{} {} {} {}',str ,vvv ,tt, ee)
    }
}
const fn = create_closure('hello')
log(fn('22')(1))
log(fn('33')(2))

const while = (s,e,fn) => {
    const cb = () => {
        fn(s)
        micro(() => while(s+1, e, fn))
    }
    (s < e) ? cb() : null
}

while(0, 5, log)
const call = (fn, arg) => fn(arg)

const call_api = (path, cb) => macro(() => {
    const url = 'https://api.ioflow.link'+path
    const data = fetch(url)
    cb(data)
})


call_api('/message', v => log(v.data))