

const socket_push = (data) => {
    make_promise(resolve => {
        const resp = fetch_async('http://127.0.0.1:12345', {
            data: json.stringify(data,0),
            method: 'post',
            headers: { 'Content-Type': 'application/json' }
        }, resolve)
    })
}

socket_push([12344]).then(resp => {
    log(resp.data)
})

