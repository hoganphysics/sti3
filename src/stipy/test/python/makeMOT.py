from stipy import *

from channels import *

def f():
    event(c1, 64.6, 89)
    event(c1, 164.6, -5)
    event(c1, 264.6, 4)
    event(c1, 269.6, [1,4,6])

    setvar("x", 34)

    settag("MOT")


    event(c1, 564.6, 2*var("x")+6)

    event(c1, 900, [[6, 8], 4, "Hello", True])

    mv = MixedValue()
    mv.addValue([6, 8])
    mv.addValue(True)
    mv2 = MixedValue()
    mv2.addValue(mv)
    mv2.addValue(4)
    mv2.addValue("Hello")
    mv2.addValue(False)

    event(c1, 1900, mv2)
