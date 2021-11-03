const macro = runInMacroQueue
const micro = runInMicroQueue
const f = format
const path = name => 'test/' + name + '.as'
const s = json.stringify
const file = fs.read(path('promise'))

// log(s(parse_agumi_script(file), 4))

const test = path + (v => {
    log(f('-- start test file:{}', v))
    v
}) + (fs.read) + eval

['promise','closure'].select(test)

const resp = fetch('https://api.ioflow.link/socket/push?descriptor=IwPYC8kUSeUHDEdT', {
    method: 'post',
    headers: {
        'Content-Type': 'application/json'
    },
    data: []
})
const keys = v => {
    const arr = object_entries(v)
    arr.select(v => v.v)
}
log(s(resp), s(keys(resp)))