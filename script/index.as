
const socket_push = (data) => {
  fetch_as_promsie('https://api.ioflow.link/socket/push?descriptor=IwPYC8kUSeUHDEdT', {
    data: json.stringify(data, 0),
    method: 'post',
    headers: { 'Content-Type': 'application/json' }
  })
}