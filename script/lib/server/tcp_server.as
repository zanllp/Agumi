const ServerConnection = make_ability('ServerConnection')

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
