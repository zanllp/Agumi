

const socket_push = (data) => {
    make_promise(resolve => {
        const resp = fetch_async('https://api.ioflow.link/socket/push?descriptor=IwPYC8kUSeUHDEdT', {
            data: json.stringify(data),
            method: 'post',
            headers: { 'Content-Type': 'application/json' }
        }, resolve)
    })
}

start_timer(() => {
    socket_push("hell world").then(resp => {
        log(resp.data)
    })
}, 1000)
