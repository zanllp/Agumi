include('lib/server/http_server')
include('lib/message_queue/client')
const port = to_num((env().process_arg.port) || 8080)
const url = f('http://127.0.0.1:{}', port)

const global_store = {
    buf: []
}

// const message_queue = create_message_queue_client(10010)


const blue_tag = log_color.blue('[log]')
const cache_loc = path_calc(env().curr_dir(), 'temp/ls.txt')

const is_dir = path => {
  const cmd = f('([ -d "{}" ] && echo "true" || echo "false") > {}', path, cache_loc)
  shell(cmd)
  fs.read(cache_loc).includes('true')
}


const get_files_html = (dir = '') => {
    dir ? @{
      const cmd = `
      cd {}
      ls > {}
      `
      shell(f(cmd, dir, cache_loc))
    } : @{
      const cmd = `ls > {}`
      shell(f(cmd, cache_loc))
    }
    const res = fs.read(cache_loc).split(lf).where(v => v).select(v => f('<a href="{}">{}</a>', f('{}?path={}',url,  dir ? (dir +'/' + v) : v), v))
}

const route = (params, resp) => {
  const curr_dir = env().curr_dir()
  const res = match (const path = params.path) {
    null: {
      data: f('<pre>{}</pre>', get_files_html().join(cr)),
      code: 200
    },
    is_dir(path): {
      data: f('<pre>{}</pre>', get_files_html(path).join(cr)),
      code: 200
    },
    fs.exist(path): {
      data: f('<pre>{}</pre>', fs.read(path)),
      code: 200
    },
    _: { code: 404, data: '<h1>404</h1>' }
  }
  resp.set_status(res.code).set_data(res.data).end()
}
// /*
make_http_server(port, {
    on_init: server => {
      // f_log('{} 服务器启动等待连接 url:{}', blue_tag, url)
      shell(f('open {}', url))
    },
    on_message: (req, resp) => {
      //  f_log('{} 请求进入 path:{}  params:{}', blue_tag, req.path, json.stringify(req.params, 4))
      const header = resp.header
      header.set('Server', 'Agumi')
      header.set('Content-Type', 'text/html; charset=utf-8')
      header.set('cache-control', 'max-age=0, no-cache, no-store')
      const path_req = req.params.path
      route(req.params, resp)
    }
})
// */
//include('script/http_server.as', true)
//include('script/index.as', true)
