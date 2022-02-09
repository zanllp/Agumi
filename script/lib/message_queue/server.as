include('lib/server/http_server')

const message_queue_server_handler = (req, resp, store) =>{
  const path = req.path
  (path == '/push') ? @{
    assert_t(req.header.get('content-type').includes('json'))
    store.queue.push(json.parse(req.data))
    resp.set_status(200).set_data('ok').end()
  } : (path == '/shift') ? @{
    const empty = store.queue.empty()
    let res = {}
    empty ? null : @{
      res = store.queue[0]
      store.queue = store.queue.range(1, -1)
    }
    resp.set_status(200).set_data(json.stringify(res, 0)).end()
  } : @{
    resp.set_status(404).set_data('<h1>404</h1>' ).end()
  }
}


const start_message_queue = (port) => {
  const store = {
    queue: []
  }
  make_http_server(port, {
    onInit: server => {
        log(f('消息队列启动 端口:{}', server.port))
    },
    onMessage: (req, resp) => {
      resp.header.set('Server', 'Agumi').set('Content-Type', 'application/json; charset=utf-8')
      message_queue_server_handler(req, resp, store)
    }
  })
}


