# control the PiLas Laser
# https://github.com/Lab5015/Lab5015Utils/blob/main/Lab5015_utils.py
import pyvisa
from pyvisa import constants
import argparse
## pyvisa-info check the backend status
class PiLas():
    """Instrument class for PiLas laser
    Args:
        * portname (str): port name
    """
    def __init__(self, portname='ASRL/dev/pilas::INSTR'):
        self.instr = pyvisa.ResourceManager().open_resource(portname)
        self.instr.baud_rate = 19200
        self.instr.read_termination = '\n'
        self.instr.write_termination = '\n'
    def read_tune(self):
        """Return the laser tune [%]"""
        print(self.instr.query("tune?"))

    def read_freq(self):
        """Return the laser frequency [Hz]"""
        print(self.instr.query("f?"))

    def check_state(self):
        """Return the laser state (0: OFF, 1: RUNNING)"""
        print(self.instr.query("ld?"))

    def set_state(self, value):
        """Set the laser state (0: OFF, 1: RUNNING)"""
        print(self.instr.query("ld="+str(value)))

    def set_trigger(self, value):
        """Set the laser trigger (0: int, 1: ext adj., 2: TTL)"""
        print(self.instr.query("ts="+str(value)))

    def set_tune(self, value):
        """Set the laser tune [%]"""
        print(self.instr.query("tune="+str(value)))

    def set_freq(self, value):
        """Set the laser frequency [Hz]"""
        print(self.instr.query("f="+str(value)))
if __name__=="__main__":
    psr = argparse.ArgumentParser()
    psr.add_argument('--on', default=False, action='store_true')
    args = psr.parse_args()
    # rm = pyvisa.ResourceManager()
    # print(rm.list_resources())
    laser = PiLas('ASRL/dev/ttyUSB0::INSTR')
    # laser.read_tune()
    laser.read_freq()
    laser.check_state()
    if args.on:
        laser.set_state(1)
    else:
        laser.set_state(0)
