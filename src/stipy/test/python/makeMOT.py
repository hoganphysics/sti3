from stipy import *

from channels import *

def f():
    event(c1, 64.6, 89)
    event(c1, 164.6, -5)
    event(c1, 264.6, 4)

    setvar("x", 34)

    settag("MOT")


    event(c1, 564.6, 2*var("x")+6)
