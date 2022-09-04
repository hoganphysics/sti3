from stipy.bin.stipy import STIPyShot


def var(self, fullVarName) :  
    return self.group().var(fullVarName)


setattr(STIPyShot, 'var', var)
