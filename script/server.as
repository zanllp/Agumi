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

const server = make_server(12345,  conn => {
    // full_log(conn.buf)
    conn.send(resp_tpl).close()
})

//include('script/server.as', true) 
//include('script/index.as', true) 
