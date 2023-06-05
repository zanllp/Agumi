
define_member_function('string', {
    includes: (this, sub) => {
        (this.byte_find(sub)) != -1
    },
    to_utf8: this => utf8.decode(this),
    format: (this, args) => {
      args = (typeof(args) == "array") ? args : [args]
      apply(f, [this].concat(args))
    }
})

