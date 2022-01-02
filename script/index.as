

const socket_push = (data) => {
    make_promise(resolve => {
        const resp = fetch_async('https://api.ioflow.link/socket/push?descriptor=IwPYC8kUSeUHDEdT', {
            data,
            method: 'post',
            headers: { 'Content-Type': 'application/json' }
        }, resolve)
    })
}

socket_push([]).then(resp => {
    log(resp.data)
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

log(utf8.decode('\u6d4b12\u8bd51234\u27ab'))
