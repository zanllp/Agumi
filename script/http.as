include('lib/server/http_server')

make_http_server(8890, {
    on_message: (req, resp) => {
      resp.set_data('hello world').end()
    }
})
