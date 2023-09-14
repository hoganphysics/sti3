from stipy import *
from inspect import getframeinfo, stack

def g(c2):

    meas(c2, 600)


def g2():
    rawFrames = stack()
    return rawFrames
