
    
// const f_log = f + log
const full_log = s + log

/*

log(log_color.blue('hello world'))
*/
const log_color = @{
    const color = get_io_mark_color()
    const end = color.end
    color.keys().aggregate({}, (p,mark) => {
        p[mark] = v => f("{}{}{}", color[mark], v, end)
        p
    })
}

const logc = (str, stack_offset = 2) => {
    const call_stack = get_call_stack_info()
    const len = call_stack.length()
    log(format('{} {}', log_color.green(call_stack[len - stack_offset].pos), str))
}


const f_log_factory = () => {
    // !TODO 使用block语法闭包会有问题
    // params PlaceHolder
    const _ = { '@F_LOG_PLACE_HOLDER': null }
    // f_log('hello {}', 'world')
    // f_log('{} {}!', 'hello', 'world')
    (tpl, a = _, b = _, c = _, d = _, f = _) => {
        log(apply(format, [tpl, a, b, c, d, f].where(v => !deep_compare(v, _))))
    }
}

const f_log = f_log_factory()