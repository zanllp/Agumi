
const fp = @{
  const is_not = target => v => v != target
  {
    is_not,
    id: x => x,
    str: {
        not_null: is_not('null'),
        not_func: is_not('function'),
        not_str: is_not('string')
    }
  }
}
