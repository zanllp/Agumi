include('lib/server/http_server')

make_http_server(8890, {
    onMessage: (req, resp) => {
      resp.set_data('hello world').end()
    }
})
