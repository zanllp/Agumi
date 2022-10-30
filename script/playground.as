const dd = () => {
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
