include('../lib/index')

const ee =[1,2,3,[1,[2]]]

ass(ee[3][1][0], 2)

const obj = {
    dd: {
        agumi: () => {
                {
                foo: () => 'ciallo'
            }
        }
    }
}

ass(obj.dd.agumi().foo(), 'ciallo')

const arr = [1,2,3, 4, 5]
ass(arr.select(v => v*3).where(v => v>3).range(0, 2)[1], 9)