include('../lib/index')

const fs_test_read_path = path_calc(env().curr_dir(),'./test.json')
const fs_test_write_path = path_calc(env().curr_dir(),'./hello.txt')
ass_t(fs.exist(fs_test_read_path))
const test_json_str = fs.read(fs_test_read_path)
ass_t((test_json_str.byte_len()) > 0)
const str = (fs.read(fs_test_read_path)) + 'hello world'
fs.write(fs_test_write_path, str)
const hello_text = fs.read(fs_test_write_path)
const test_str_size= test_json_str.byte_len()
ass(hello_text.byte_len(), test_str_size + 11)