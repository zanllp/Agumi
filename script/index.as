

const socket_push = (data) => {
    make_promise(resolve => {
        const resp = fetch_async('http://127.0.0.1:12345', {
            data: json.stringify(data),
            method: 'post',
            headers: { 'Content-Type': 'application/json' }
        }, resolve)
    })
}

socket_push("hell world").then(resp => {
    log(resp.data)
})

