include('./stdafx.as')



const kv_store = make_ability('KV-Store')

define_member_function(kv_store, {
    exist: (this, key) => {
        const l = fs.exist(f(this.base_file_tpl, key))
        const r = this.get_item(key)
        to_bool(and_op(l, r))
    },
    set_item: (this, key, value) => {
        const file_name = f(this.base_file_tpl, key)
        (this.exist(key))? null : (this._mark(key))
        fs.write(file_name, s(value, 0))
    },
    get_item: (this, key) => {
        const file_name = f(this.base_file_tpl, key)
        const str = fs.read(file_name)
        const r = json.parse(str)
        (typeof(r) === 'null') ? null : r
    },
    remove_item: (this, key) => {
        fs.write(f(this.base_file_tpl, key), '')
        fs.write(this.all_keys_file, s(this.get_keys()))
    },
    _mark: (this, key) => {
        const next_keys = this.get_keys()
        next_keys.push(key)
        fs.write(this.all_keys_file, s(next_keys, 0))
    },
    get_keys: (this) => {
        const keys = this.get_item(this.all_keys)
        const res = (typeof(keys) == "array") ? keys : []
        res.where(v => this.exist(v))
    },
    clear: (this) => {
        const keys = this.get_keys()
        keys.select(key => this.remove_item(key) )
        fs.write(this.all_keys_file, s([]))
    }
})

const make_kv = (dir) => {
    const base_dir = path_calc(env().working_dir, dir)
    log(f('start kv base dir:{}', base_dir))
    const all_keys = '__all__keys__'
    const base_file_tpl  = path_calc(base_dir, '_kv_{}.json')
    const all_keys_file = f(base_file_tpl, all_keys)
    use_ability({
        base_dir, 
        base_file_tpl,
        all_keys_file,
        all_keys 
        }, kv_store)
}

const storage = make_kv(path_calc(env().curr_dir(),"../temp"))
