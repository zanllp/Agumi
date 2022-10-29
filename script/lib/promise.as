
const Promise = make_ability('Promise')

const make_promise = (resolver) => {
  const this = { ok_cbs: [], err_cbs: [], res:null, state: 'pending' }
  resolver((res) => {
    this.res = res
    this.state = 'ok'
    this.ok_cbs.select(fn => fn(res))
  }, (errmsg) => {
    this.state = 'err'
    this.err_cbs.select(fn => fn(res))
  })
  use_ability(this, Promise)
}

define_member_function(Promise, {
    then: (this, cb) => {
      ((this.state) == 'ok') ? @{
        make_promise((resolve) => resolve(cb(this.res)))
      } : @{
        make_promise((resolve) => {
          this.ok_cbs.push(res => {
            resolve(cb(res))
          })
        })
      }
    }
})


const fetch_as_promsie = (url, params) => {
  make_promise((resolve, err) => {
     fetch_async(url, params, resolve)
  })
}


@{
  const global = mem()
  const _sync_fetch = fetch
  global.fetch = fetch_as_promsie
  global.fetch_sync = _sync_fetch
}

/*
const dd = () => make_promise(cb => cb('1'),null).then(() => log(2)).then(() => log(3))
micro(dd)
const ddc = () => make_promise(cb => macro(() => cb('xxxx')),null).then((x) => log(x, 21)).then(() => log(31))
micro(ddc)
const dc = () => fetch('https://github.com/orgs/xiachufang/dashboard',{ method: 'get' } ).then(data => {
  log(json.stringify(data))
  fs.write('dashboard.html', data.data)
})
micro(dc)

const call_api_promise = (path) => {
  const url = 'https://api.ioflow.link'+path
  fetch(url, { method: 'get' })
}

const p = call_api_promise('/message').then(v => v.data)
p.then(_ => log('twice callback'))
p.then(v => log(v, 23333))

*/