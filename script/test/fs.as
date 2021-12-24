include('../lib/index')

const test_json_str = fs.read('test.json')
ass_t((test_json_str.length()) > 0)
const str = (fs.read('test.json')) + 'hello world'
fs.write('hello.txt', str)
const hello_text = fs.read('hello.txt')
const test_str_size= test_json_str.length()
ass(hello_text.length(), test_str_size + 11)