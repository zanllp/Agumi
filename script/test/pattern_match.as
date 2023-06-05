

@{
  const val = match(let num = 0) {
    0: 1,
    1: 2,
    _: 0
  }
  assert(val, 1)
}

@{
  const val = match(let x = 'hello') {
    is_str(x) : x + " world",
    _: ''
  }
  assert(val, "hello world")
}

@{
  const val = match(let x = 123) {
    is_str(x)  : x + "world",
    is_bool(x) : false,
    is_obj(x)  : { h: '1111' },
    _: ''
  }
  assert(val, '')
}

@{
  const fib = n => match(n) {
    0: 0,
    1: 1,
    _: fib(n-1) + fib(n-2)
  }
  assert(fib(10), 55)
}

@{
  const val = { type: 'abc', aaa: 222, aaa: 222 }

  match(val) {
    { type: 'abc' }: @{},
    _: assert_t(false)
  }
}

@{
  const val = [1,2,3,5,6]

  match(val) {
    [1,2,3]: @{},
    _: assert_t(false)
  }
}
