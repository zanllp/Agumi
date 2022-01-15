const block_test_0 = @{
    log('hello')
    111 + 222
}
assert(block_test_0, 333)

const block_test_1 = 0 ? @{
    123 + 2
} : 1 ? @{
    456 *2
} : 2334

assert(block_test_1, 912)