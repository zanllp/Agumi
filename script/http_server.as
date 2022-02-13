include('lib/server/http_server')
include('lib/message_queue/client')

const error_404 = (resp) => {
    resp.set_status(404).set_data('<h1>404</h1>').end()
}

const ok_200 = (resp, path) => {
    resp.set_status(200).set_data(fs.read(path)).end()
}

const global_store = {
    buf: []
}

const message_queue = create_message_queue_client(10010)

start_timer(() => {
    message_queue.shift().then((data) => {
        (data != null) ? @{
            log('received a message', data)
            global_store.buf.push(data)
        }: null
    })
}, 1000)

make_http_server(8898, {
    onInit: server => {
        log(f('服务器启动等待连接 端口:{}', server.port))
    },
    onMessage: (req, resp) => {
        resp.header.set('Server', 'Agumi').set('Content-Type', 'text/html; charset=utf-8')
        const path_req = req.params.path
        const has_path = () => {
            const path = path_calc(env().curr_dir(), path_req)
            (fs.exist(path)) ? ok_200(resp, path) : error_404(resp)
        }
        path_req ? has_path() : error_404(resp)
    }
})

//include('script/http_server.as', true) 
//include('script/index.as', true) 
