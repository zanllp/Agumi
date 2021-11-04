const f = format
const s = json.stringify
const f_log = f + log
const full_log = s + log
const const_file_name  = 'test/_kv_{}.json'
const all_keys = '__all__keys__'
const all_keys_file = f(const_file_name, all_keys)
const and_op = (a, b) => a ? b : a
const to_bool = a => a ? true: false


const storage = {
    exist: key => {
        const l = fs.exist(f(const_file_name, key))
        const r = storage.get_item(key)
        to_bool(and_op(l, r))
    },
    set_item: (key, value) => {
        const file_name = f(const_file_name, key)
        (storage.exist(key))? null : (storage._mark(key))
        fs.write(file_name, s(value, 0))
    },
    get_item: key => {
        const file_name = f(const_file_name, key)
        const str = fs.read(file_name)
        const r = json.parse(str)
        (typeof(r) === 'undefined') ? null : r
    },
    remove_item: key => {
        storage.set_item(key, null)
        fs.write(all_keys_file, s(storage.get_keys()))
    },
    _mark: (key) => {
        const next_keys = storage.get_keys()
        next_keys.push(key)
        fs.write(all_keys_file, s(next_keys, 0))
    },
    get_keys: () => {
        const keys = storage.get_item(all_keys)
        const res = (typeof(keys) == "array") ? keys : []
        res.where(storage.exist)
    },
    clear: () => {
        const keys = storage.get_keys()
        keys.select(key => storage.remove_item(key) )
        fs.write(all_keys_file, s([]))
    }
}
const ass = (a, b) => {
    const throw_err =() => {
        f_log(`assert error: a: {} b: {}`,a,b)
        assert(false)
    }
    (s(a) == s(b)) ? null  : throw_err()
}


const test = () => {
    storage.set_item('1111', 23333)
    ass(storage.get_item('1111'), 23333)
    storage.remove_item('1111')
    ass(storage.get_item('1111'), null)
    storage.set_item('hello', mem())
    storage.set_item('hello1', mem())
    storage.set_item('hello2', mem())
    let keys = storage.get_keys()
    ass(keys.length(), 3)
    storage.clear()
    keys = storage.get_keys()
    ass(keys.length(), 0)
    ass(storage.exist('hello'), false)
    ass(storage.get_item('hello'),null)
}
test()