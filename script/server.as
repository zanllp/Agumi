const ServerConnection = make_ability('ServerConnection')
const resp_tpl = fs.read(path_calc(env().curr_dir(),'resp.http'))
set_gc({ step:100000, enable: true })

define_member_function(ServerConnection, {
    send: (this, data) => {
        send_server_data(this, data)
        this
    } ,
    close: (this) => close_server_connection(this)
})

const Server = make_ability('Server')

define_member_function(Server, {
    close: this => close_server(this)
})

make_server({
    port: 12345,
    onInit: server => {
        log(f('服务器启动等待连接 端口:{}', server.port))
    },
    onAccept: (conn) => {
        log('建立连接')
        conn.onMessage = (conn) => {
            log(conn.buf)
            conn.send(resp_tpl).close()
        }
    },
    onClose: () => {

    }
})
//include('script/server.as', true) 
//include('script/index.as', true) 
