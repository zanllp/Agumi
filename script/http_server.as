const resp_tpl = fs.read(path_calc(env().curr_dir(),'resp.http'))
const cr = `
`

const parse_http_message_profile = (line) => {
    const profile = line.split(' ').select(item => item.trim())
    {
        method: profile[0],
        path: profile[1],
        protocolVersion: profile[2]
    }
}
 
const parse_http_message = (msg) => {
    const lines = msg.split(cr)
    const res = {
        header: [],
        profile: {},
        is_end: false
    }
    const get_content_length = () => {
        const headers = res.header
        const key = 'content-length'
        const idx = headers.select(v => v.k.byte_to_lowercase()).find_index(key)
        (idx!=-1) ? headers[idx].v : ''
    }
    const resolve_profile = (v) => {
        res.profile = parse_http_message_profile(v)
    }
    const add_header = (v) => {
        const colon_idx = v.byte_find(':')
        assert_t(colon_idx > 0)
        res.header.push({ k: v.substr(0, colon_idx), v: v.substr(colon_idx + 1).trim() })
    }
    let data_end = -1
    const mark_end = (i, stop) => {
        stop()
        data_end = i
    }
    lines.select((v,i, stop) => {
        const trim_v = v.trim()
        const len = trim_v.length()
        const is_first = i == 0
        const is_header = not((v.byte_find(':')) == -1)
        is_first ? resolve_profile(trim_v) : null
        and(not(is_first), is_header) ? add_header(trim_v) : null
        and(not(is_first), not(is_header)) ? mark_end(i, stop) : null 
    })
    assert_t(data_end > -1)
    res.data = lines.range(data_end + 1, -1).join(cr)
    res.is_end = get_content_length() == to_str(res.data.length())
    res
}

const make_http_server = (port, cb) => {
    make_server({
        port,
        onInit: cb.onInit,
        onAccept: (conn) => {
            const buf = []
            conn.onMessage = (conn) => {
                buf.push(conn.buf)
                const msg = parse_http_message(buf.join(''))
                (msg.is_end) ? cb.onMessage(msg, conn) : null
            }
        },
        onClose: () => {

        }
    })
}

make_http_server(12345, {
    onInit: server => {
        log(f('服务器启动等待连接 端口:{}', server.port))
    },
    onMessage: (msg, conn) => {
        log(msg.data)
        conn.send(resp_tpl).close()
    }
})

//include('script/http_server.as', true) 
//include('script/index.as', true) 
