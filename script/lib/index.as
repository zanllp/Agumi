include('./stdafx')
include('./fp.as')
include('./operator_def')
include('./common')
include('./object')
include('./string')
include('./array')
include('./kv')
include('./promise')
include('./log')
include('./timer')

const shell = sys_call
const global = mem()

const range = (end, start = 0) => start..end


const for = (count, cb) => {
  range(count).select(cb)
  null
}

const to_num = (str) => json.parse(str)

const cowsay = msg => {
  log(f(`
  ____________
  < {} >
  ------------
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||
  `, msg))
}

