include('lib/server/tcp_server')

set_gc({ step:100000, enable: true, log: true })

const resp_tpl = fs.read(path_calc(env().curr_dir(),'resp.http'))
make_server({
    port: 12345,
    onInit: server => {
        log(f('服务器启动等待连接 端口:{}', server.port))
    },
    onAccept: (conn) => {
        log('建立连接')
        conn.onMessage = (conn) => {
            log('接收到数据',conn.buf)
            conn.send('echo: ' + conn.buf).close()
        }
    },
    onClose: () => {

    }
})
//include('script/server.as', true) 
//include('script/index.as', true) 
