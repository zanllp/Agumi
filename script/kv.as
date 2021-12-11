include('script/stdafx.as')

const base_dir = 'script'
const base_file_tpl  = base_dir + '/_kv_{}.json'
const all_keys = '__all__keys__'
const all_keys_file = f(base_file_tpl, all_keys)

const storage = {
    exist: key => {
        const l = fs.exist(f(base_file_tpl, key))
        const r = storage.get_item(key)
        to_bool(and_op(l, r))
    },
    set_item: (key, value) => {
        const file_name = f(base_file_tpl, key)
        (storage.exist(key))? null : (storage._mark(key))
        fs.write(file_name, s(value, 0))
    },
    get_item: key => {
        const file_name = f(base_file_tpl, key)
        const str = fs.read(file_name)
        const r = json.parse(str)
        (typeof(r) === 'null') ? null : r
    },
    remove_item: key => {
        fs.write(f(base_file_tpl, key), '')
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

