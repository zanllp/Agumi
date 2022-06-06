
define_member_function('array', {
    where: array_filter,
    range: (this, start, count) => {
        let r = []
        this.select((v,i, stop) => {
            or(i < (start + count), count == -1) ? @{
                 r.push(v)
            } : null
        }, start)
        r
    },
    join: (this, spec) => {
        // 设置值和闭包还是有问题
        const res = { v: (this.empty()) ? '' : to_str(this[0]) }
        this.select((v) => {
            res.v = ((res.v) + to_str(spec) + to_str(v))
        }, 1)
        res.v
    },
    find_index: (this, target) => {
        let res = {v:-1}
        this.select((v,i, stop) => {
            (v == target) ? @{
                res.v=i
                stop()
            } : null
        })
        res.v
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
