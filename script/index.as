
const socket_push = (data) => {
  fetch_as_promsie('https://api.ioflow.link/socket/push?descriptor=IwPYC8kUSeUHDEdT', {
    data: json.stringify(data, 0),
    method: 'post',
    headers: { 'Content-Type': 'application/json' }
  })
}

const get_message_queue_accept_port = () => {
  const conf_file = fs.read(path_calc(env().curr_dir(), 'conf.json'))
  json.parse(conf_file).MessageQueueAccpetPort
}

const port = get_message_queue_accept_port()
