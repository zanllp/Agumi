@{
    const s = { v: null }
    const effect = (v) => {
        s.v = v
    }
    @{
        const call_effect = () => {
            return 0
            effect(0)
        }
        call_effect()
        assert(s.v, null)
    }
    @{
        const call_effect = () => {
            1 ? @{
                return 1
            } : effect(0)
        }
        assert(call_effect(), 1)
        assert(s.v, null)
    }
    @{
        const call_effect = () => {
            0 ? @{
                return 2
            } : effect(0)
        }
        assert(call_effect(), 0)
        assert(s.v, 0)
    }
}