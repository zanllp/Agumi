const resp_tpl = fs.read(path_calc(env().curr_dir(),'resp.http'))
const cr = utf8.from_code_point('0xd') // \r
const lf = utf8.from_code_point('0xa') // \n
const crlf = cr + lf

const parse_http_message_profile = (line) => {
    const profile = line.split(' ').select(item => item.trim())
    {
        method: profile[0],
        path: profile[1],
        protocolVersion: profile[2]
    }
}

const ability_set = []
const make_factory = (name, init_data_fn, mem_fn) => {
    const idx = ability_set.find_index(name)
    assert_t(idx == -1)
    const ability = make_ability(name)
    ability_set.push(name)
    define_member_function(ability, mem_fn)
    () => use_ability(init_data_fn(), ability)
}


const create_http_header =  make_factory('HttpHeader', () => ({ _raw_data: [] }), {
    get: (this, key) => {
        const idx = this._raw_data.select(v => v.k.byte_to_lowercase()).find_index(key)
        (idx!=-1) ? this._raw_data[idx].v : ''
    },
    set: (this, k, v) => {
        this._raw_data.push({k,v})
        this
    },
    all: this => this._raw_data
})

 
const parse_http_message = (http_raw_msg) => {
    const lines = http_raw_msg.split(crlf)
    const req = {
        header: create_http_header(),
        method:'',
        path: '',
        protocolVersion: '',
        is_end: false
    }
    
    const resolve_profile = (v) => {
        const profile = parse_http_message_profile(v)
        req.path = profile.path
        req.protocolVersion = profile.protocolVersion
        req.method = profile.method
    }
    const add_header = (v) => {
        const colon_idx = v.byte_find(':')
        assert_t(colon_idx > 0)
        req.header.set(v.substr(0, colon_idx).trim(), v.substr(colon_idx + 1).trim())
    }
    let data_end = -1
    const mark_end = (i, stop) => {
        stop()
        data_end = i
    }
    lines.select((v,i, stop) => {
        const len = v.length()
        const is_first = i == 0
        const is_header = not((v.byte_find(':')) == -1)
        is_first ? resolve_profile(v) : null
        and(not(is_first), is_header) ? add_header(v) : null
        and(not(is_first), not(is_header)) ? mark_end(i, stop) : null 
    })
    assert_t(data_end > -1)
    req.data = lines.range(data_end + 1, -1).join(crlf)
    req.is_end = (req.header.get('content-length')) == to_str(req.data.length())
    req
}

const HttpResponse = make_ability('HttpResponse')

define_member_function(HttpResponse, {
    set_data: (this, data) => {
        this.data = data
        this
    },
    set_status: (this, status) => {
        this.status = status
        this
    },
    end: (this) => {
        this.conn.send(this.build_resp()).close()
    },
    build_resp: (this) => {
        const line = []
        line.push([this.http_version(), this.status, this.status_str].join(' '))
        this.header.set('Content-Length', this.data.byte_len()).set('Connection', 'close')
        this.header.all().select(h => line.push(f(`{}: {}`, h.k, h.v)))
        line.push('')
        line.push(this.data)
        line.join(crlf)
    },
    http_version: () => 'HTTP/1.1'
})

const make_http_server = (port, cb) => {
    make_server({
        port,
        onInit: cb.onInit,
        onAccept: (conn) => {
            const buf = []
            const resp = {
                header: create_http_header(),
                conn,
                data: '',
                status: 200,
                status_str: 'OK'
            }
            use_ability(resp, HttpResponse)
            conn.onMessage = (conn) => {
                buf.push(conn.buf)
                const req = parse_http_message(buf.join(''))
                (req.is_end) ? cb.onMessage(req, resp) : null
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
    onMessage: (req, resp) => {
        log(req.header.get('cookie'))
        req.header.set('Server', 'Agumi').set('Content-Type', 'application/json')
        const data = '{"account":"m32@zasdcfs.cnew","password":"dsfcwcweq1q","remember":true}'
        resp.set_status(200).set_data(data).end()
    }
})

//include('script/http_server.as', true) 
//include('script/index.as', true) 
