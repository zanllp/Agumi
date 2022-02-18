const create_message_queue_client = (port) => {
  const push = (data) => {
    fetch_as_promsie(f('http://127.0.0.1:{}/push', port), {
      data: json.stringify(data, 0),
      method: 'post',
      headers: { 'Content-Type': 'application/json' }
    })
  }
  const shift = () => {
    const p = fetch_as_promsie(f('http://127.0.0.1:{}/shift', port), {
      headers: { 'Content-Type': 'application/json' },
      method: 'get'
    })
    p.then(resp => json.parse(resp.data))
  }
  { push, port, shift }
}
