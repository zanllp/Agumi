
define_member_function('string', {
    includes: (this, sub) => {
        (this.byte_find(sub)) != -1
    }
})

