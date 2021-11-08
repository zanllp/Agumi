include('test/stdafx.as')

const base_dir = 'test'
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
const ass = (a, b) => (s(a) == s(b)) ? null  : throw(f(`assert error: a: {} b: {}`,a,b))


const test = () => {
    storage.clear()
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