include('./stdafx.as')

const test_json_str = fs.read('test.json')
ass_t((test_json_str.length()) > 0)
const str = (fs.read('test.json')) + 'hello world'
fs.write('hello.txt', str)
ass(fs.read('hello.txt').length(), test_json_str.length() + 11)