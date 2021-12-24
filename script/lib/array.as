
define_member_function('array', {
    range: (this, start, count) => {
        let res = []
        this.select((v,i) => {
            if_exec(and_op(i>=start, i < (start + count)), () => {
                res.push(v)
            })
        })
        res
    },
    join: (this, spec) => {
        let res = f(this[0])
        const arr = this.range(1,(this.length()) - 1)
        arr.select((v,i) => {
            res = res + f(spec) + f(v)
        })
        res
    },
    find_index: (this, target) => {
        let res = -1
        this.select((v,i) => if_exec(v == target , () => {
            res=i
        }))
        res
    }
})
