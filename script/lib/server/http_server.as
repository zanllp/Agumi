include('./tcp_server')

const cr = utf8.from_code_point('0xd') // \r
const lf = utf8.from_code_point('0xa') // \n
const crlf = cr + lf
const http_status_map_file = fs.read(path_calc(env().curr_dir(), 'http_status_map.csv'))
const http_status_map = {} 
http_status_map.from_entires(http_status_map_file.split(lf).select(line => line.split(',').select(v => v.trim())))



const parse_url_params = (url ,idx, res) => {
    res.path = url.substr(0, idx)
    res.params.from_entires(url.substr(idx + 1).split('&').select(kv => kv.split('=')))
}

const parse_url = (url) => {
    const res = { path: '', params: {} }
    const query_idx = url.byte_find('?')
    const set_path = () => {
        res.path = url
    }
    (query_idx == -1) ? set_path() : parse_url_params(url, query_idx, res) 
    res
}

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
    has: (this, key) => {
        const idx = this._raw_data.select(v => v.k.byte_to_lowercase()).find_index(key)
        idx!=-1
    },
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
        params: {},
        protocolVersion: '',
        data: '',
        is_end: false
    }
    
    const resolve_profile = (v) => {
        const profile = parse_http_message_profile(v)
        const url_parsed = parse_url(profile.path)
        req.path = url_parsed.path
        req.params = url_parsed.params
        req.protocolVersion = profile.protocolVersion
        req.method = profile.method
    }
    const add_header = (v) => {
        const colon_idx = v.byte_find(':')
        assert_t(colon_idx > 0)
        req.header.set(v.substr(0, colon_idx).trim(), v.substr(colon_idx + 1).trim())
    }
    const mark_end = (i, stop) => {
        stop()
        req.data = lines.range(i + 1, -1).join(crlf)
        const has_data_end = () => {
            req.is_end = (req.header.get('content-length')) == to_str(req.data.length())
        }
        const no_data_end = () => {
            req.is_end = true
        }
        (req.header.has('content-length')) ? has_data_end() : no_data_end()

    }
    lines.select((v,i, stop) => {
        const len = v.length()
        const is_first = i == 0
        const is_header = not((v.byte_find(':')) == -1)
        is_first ? resolve_profile(v) : null
        and(not(is_first), is_header) ? add_header(v) : null
        and(not(is_first), not(is_header)) ? mark_end(i, stop) : null 
    })
    req
}


const HttpResponse = make_ability('HttpResponse')

define_member_function(HttpResponse, {
    set_data: (this, data) => {
        this.data = data
        this
    },
    set_status: (this, status) => {
        const status_str = status.to_string()
        assert_t(http_status_map.has(status_str))
        this.status = status
        this.status_desc = http_status_map[status_str]
        this
    },
    end: (this) => {
        this.conn.send(this.build_resp()).close()
    },
    build_resp: (this) => {
        const line = []
        line.push([this.http_version(), this.status, this.status_desc].join(' '))
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
                status_desc: 'OK'
            }
            use_ability(resp, HttpResponse)
            conn.onMessage = (conn) => {
                buf.push(conn.buf)
                const buf_join = buf.join('')
                const req = parse_http_message(buf_join)
                (req.is_end) ? cb.onMessage(req, resp, buf_join) : null
            }
        },
        onClose: () => {

        }
    })
}
