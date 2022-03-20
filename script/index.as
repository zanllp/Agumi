const socket_push = (data) => {
  fetch_as_promsie('https://api.ioflow.link/socket/push?descriptor=IwPYC8kUSeUHDEdT', {
    data: json.stringify(data, 0),
    method: 'post',
    headers: { 'Content-Type': 'application/json' }
  })
}

const global_s = {
  i: 0
}
const ttfn = () => {
  global_s.i = (global_s.i) + 1
  const i2 = global_s.i
   ['hello world']
  ((i2 % 10000) == 0) ? @{
    log('dd',i2)
  } : null
  (i2 > 2000000) ? @{
    gc()
  } : @{
    micro(ttfn)
  }
}

ttfn()


start_timer(() => {
  log(';111111')
}, 10000000000)
