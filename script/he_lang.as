define_operator('number', 'number' , '|' , (l,r) => [l, r])
define_operator('number', 'array' , '|' , (l,r) => [l].concat(r))

const keys = 0..84

const powerCon = (target, val) => keys.select(v => (target.exist(v)) ? val : 0)
const res = powerCon(1 | 2 | 6 | 7 | 11 | 52 | 57 | 57 | 65, 10)

log(json.stringify(res, 0))
