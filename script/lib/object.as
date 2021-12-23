define_member_function('object', {
    entires: this => {
        object_entries(this)
    },
    values: (this) => {
        const entires = this.entires()
        entires.select(v => v.v)
    },
    has: (this, key) => {
        const keys = this.keys()
        (keys.find_index(key)) != -1
    },
    keys: (this) => {
        const entires = this.entires()
        entires.select(item => item.k)
    }
})
