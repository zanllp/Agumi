
define_member_function('array', {
    where: (this, predicate) => {
        const res = []
        this.select((v,i) => predicate(v,i) ? res.push(v) : null)
        res
    },
    range: (this, start, count) => {
        let r = []
        this.select((v,i, stop) => {
            if_exec(or(i < (start + count), count == -1), () => {
                r.push(v)
            })
        },start)
        r
    },
    join: (this, spec) => {
        let res = f(this[0])
        this.select((v,i) => {
            res = res + f(spec) + f(v)
        }, 1)
        res
    },
    find_index: (this, target) => {
        let res = -1
        this.select((v,i, stop) => {
            if_exec(v == target , () => {
                res=i
                stop()
            })
        })
        res
    },
    count: this => this.length(),
    empty: this => (this.count()) == 0,
    concat: (this, rhs) => {
        rhs.select(v => this.push(v))
        this
    },
    reverse: (this) => {
        const res = []
        const size = this.count()
        const dist = size - 1
        res.resize(size)
        this.select((v,i) => {
            res[dist - i] = v
        })
        res
    }
})
