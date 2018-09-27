import gpiozero as gp
class Valve(object):
    """description of class"""
    def __init__(self,name,index):
        self.name = name
        self.pin = gp.OutputDevice(index)
        self.state = 0


    def On(self):
        self.pin.on()
        self.state = 1
    def Off(self):
        self.pin.off()
        self.state = 0

