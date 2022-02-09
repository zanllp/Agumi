const create_message_queue_client = (port) => {
  const push = (data) => {
    fetch_as_promsie(f('http://127.0.0.1:{}/push', port), {
      data: json.stringify(data, 0),
      method: 'post',
      headers: { 'Content-Type': 'application/json' }
    })
  }
  const shift = () => {
    fetch_as_promsie(f('http://127.0.0.1:{}/shift', port, 2), {
      headers: { 'Content-Type': 'application/json' },
      method: 'get'
    })
  }
  { push, port, shift }
}
