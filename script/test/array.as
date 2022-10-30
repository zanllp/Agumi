include('../lib/index')

const arr_test = [1,2,43,4]
assert(arr_test.reverse(), [4,43,2,1])
assert([1,2,3].concat([4,5]), [1,2,3,4,5])
assert(['hello', ' ', 'world', '!'].join(''), 'hello world!')
assert([1,2,3,4].aggregate(0, (p,c) => p + c), 10)