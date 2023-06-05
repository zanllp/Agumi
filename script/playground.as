/*const dd = () => {
    const obj = { id: start_timer(() => log('call'), 30) }
    make_promise(cb => {
      fetch('http://127.0.0.1:8890', { method: 'get' }).then(v => {
        log(v.err_msg)
        remove_timer(obj.id)
        cb(v)
      })
    })
}
dd().then(v => log(f('end {}', v.keys().join(','))))


const delayExec = (fn, timeout = 1000) => start_timer(fn,timeout, true)
*/

const json_str = fs.read("/Users/wuqinchuan/Desktop/project/agumi/canada.json")

const j = json.parse(json_str)
@{
  const fn = (node) => {
    const t = typeof(node)
    (t == "object") ? @{
      node.values().select(fn)
    } : (t == "array") ? @{
      node.select(fn)
    } : log(node)

  }
  // fn(j)
}

const t = time()
for(5, () => {
  // json.parse(json_str)
})
log((time() - t)/5)


const match = (params) => {
  assert(typeof(params), "object")
  full_log(params.values())
}

match({
  "hello": 1,
  world: 2
})
