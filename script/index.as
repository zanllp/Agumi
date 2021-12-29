include('lib/index')


const socket_push = (data) => {
    make_promise(resolve => {
        const resp = fetch('http://api.ioflow.link/socket/push?descriptor=IwPYC8kUSeUHDEdT', {
            data,
            method: 'post',
            headers: { 'Content-Type': 'application/json' }
        })
        resolve(resp)
    })
}

socket_push([]).then(resp => {
    // log(resp.data)
})


const hh = "😄"
const h1 = "w"
const h2 = "好"
const dd = () => [ hh, hh.byte_len(), h1.byte_len(), h2.byte_len()]
const dd2 = () => [ hh, hh.length(), h1.length(), h2.length()]
const emojis = "🐶🍐🍎🏡💀🐜".substr(0)

// log(s(dd()), s(dd2()))
log(emojis.length(), emojis.substr(2))

emojis.split().select(log)
