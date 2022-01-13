define_member_function('object', {
    entires: this => {
        object_entries(this)
    },
    values: (this) => {
        this.entires().select(v => v.v)
    },
    has: (this, key) => {
        (this.keys().find_index(key)) != -1
    },
    keys: (this) => {
        this.entires().select(item => item.k)
    },
    from_entires: (this, entires) => {
        entires.select(kv => {
            this[kv[0]] = kv[1]
        })
        this
    }
})
