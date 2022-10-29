
    
// const f_log = f + log
const full_log = s + log

const log_color = @{
    const color = get_io_mark_color()
    const end = color.end
    color.keys().aggregate({}, (p,mark) => {
        p[mark] = v => f("{}{}{}", color[mark], v, end)
        p
    })
}

/*

log(log_color.blue('hello world'))
*/