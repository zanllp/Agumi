include('test/stdafx.as')

const make_promise_internal = (resolve) => {
    const onreslove_callbacks = []
    resolve(res => onreslove_callbacks.select(fn => fn(res)))
    {
        then: cb => make_promise_internal(reslove => {
            onreslove_callbacks.push(cb + reslove)
        })
    }
}

const make_promise = cb => make_promise_internal(resolve => micro(() => cb(resolve)))
/*
const call_api_promise = (path) => {
    make_promise(resolve => {
        const url = 'https://api.ioflow.link'+path
        const resp = fetch(url)
        resolve({ resp, url })
    })
} 

const p = call_api_promise('/message').then(v => v.resp.data)
p.then(_ => log('twice callback'))
p.then(v => log(v, 23333))
*/
