const macro = runInMacroQueue
const micro = runInMicroQueue
const f = format
const path = name => 'test/' + name + '.as'
const s = json.stringify
const file = loadFile(path('promise'))

// log(s(parse_agumi_script(file), 4))

const test = path + (v => {
    log(f('-- start test file:{}', v))
    v
}) + loadFile + eval

['promise','closure'].select(test)

fetch('https://api.ioflow.link/message', {
    method: 'post',
    headers: {
        tt: 'cccc'
    },
    data: ['hello world']
})
