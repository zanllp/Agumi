include('lib/index')


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

socket_push([]).then(resp => {
    log(resp.data)
})

