log(fs.exist('README.md'))
log(fs.read('test.json'))
const str = (fs.read('test.json')) + 'hello world'
log(str)
fs.write('hello.txt', str)