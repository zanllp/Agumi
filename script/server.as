const ServerConnection = make_ability('ServerConnection')
const resp_tpl = `HTTP/1.1 200 OK
Date: Mon, 03 Jan 2022 15:09:13 GMT
Server: Apache
Last-Modified: Sun, 09 Apr 2006 00:47:00 GMT
ETag: "8934-410f4d3e66900-gzip"
Accept-Ranges: bytes
Vary: Accept-Encoding
Content-Encoding: gzip
Content-Length: 0
Keep-Alive: timeout=5, max=100
Connection: close
Content-Type: text/html


`

define_member_function(ServerConnection, {
    send: (this, data) => {
        send_server_data(this, data)
        this
    } ,
    close: (this) => close_server_connection(this)
})

const server = make_server(12345,  conn => {
    full_log(conn.buf)
    conn.send(resp_tpl).close()
})

//include('script/server.as', true) 
//include('script/index.as', true) 
