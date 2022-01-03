const ServerConnection = make_ability('ServerConnection')

define_member_function(ServerConnection, {
    send: (this, data) => send_server_data(this.socket, data) ,
    close: () => close_server_connection()
})

const server = make_server(12345,  e => {
    full_log(e.buf)
})