include('script/stdafx.as')
include('script/kv.as')
include('script/promise.as')

const socket_push = (data) => {
    make_promise(resolve => {
        const resp = fetch('https://api.ioflow.link/socket/push?descriptor=IwPYC8kUSeUHDEdT', {
            data,
            method: 'post',
            headers: { 'Content-Type': 'application/json' }
        })
        resolve(resp)
    })
}

const resp = socket_push([])
resp.then(data => {
    storage.set_item('resp', data)
    log(s(storage.get_item('resp')))
})