# 获取当前run对应PMT的高压并对应至相应的高压通道；模拟鼠标点击控制高压软件
# https://iseg-hv.com/download/SOFTWARE/isegSNMPcontrol/current/Manual_MPOD_HV_2.5.7.pdf
# from easysnmp import Session
## require libsnmp-dev snmp-mibs-downloader
import subprocess
from CSVData import PMTINFO, OriginINFO
import time
import argparse
class snmpSessionBaseClass(object):
    """A Base Class For a SNMP Session"""
    def __init__(self,
                version=2,
                host="localhost",
                community="public"):
        if version==2:
            self.version = '2c'
        else:
            self.version = version
        self.host = host
        self.community = community

    def Get(self, oid):
        """Creates SNMP query session"""
        process = subprocess.Popen('snmpget -Oqs -v {}  -m WIENER-CRATE-MIB -c {} {} {}'.format(self.version, self.community, self.host, oid), shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        while process.poll() is None:
            line = process.stdout.readline()
            line = line.strip()
            if line:
                print(line.decode("utf8", 'ignore'))
    def Set(self, oid, value):
        """Creates SNMP query session"""
        process = subprocess.Popen('snmpset -Oqs -v {}  -m WIENER-CRATE-MIB -c {} {} {} {}'.format(self.version, 'guru', self.host, oid, value), shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        while process.poll() is None:
            line = process.stdout.readline()
            line = line.strip()
            if line:
                print(line.decode("utf8", 'ignore'))
class Session(snmpSessionBaseClass):
    def __init__(self,
                version=2,
                host="localhost",
                community="public"):
        super(Session, self).__init__(version, host, community)
    def getVoltage(self, boardid, channel):
        self.Get('outputVoltage.u{}'.format(boardid*100 + channel))
    def getVoltageSense(self, boardid, channel):
        self.Get('outputMeasurementSenseVoltage.u{}'.format(boardid*100 + channel))
    def setVoltage(self, boardid, channel, value):
        self.Set('outputVoltage.u{}'.format(boardid*100 + channel), 'F {}'.format(value))
    def onCh(self, boardid, channel):
        self.Set('outputSwitch.u{}'.format(boardid*100 + channel), 'i 1')
    def offCh(self, boardid, channel):
        self.Set('outputSwitch.u{} i {}'.format(boardid*100 + channel), 'i 0')
device_ip = '192.168.1.15'
board_i = 4
if __name__ == '__main__':
    psr = argparse.ArgumentParser()
    psr.add_argument('--origincsv', help='origin csv file')
    psr.add_argument('--pmtcsv', help='pmt csv file')
    psr.add_argument('--on', default=False, action='store_true')
    args = psr.parse_args()
    # Get the channel and HV
    pmtinfo = PMTINFO(args.pmtcsv)
    origininfo = OriginINFO(args.origincsv)
    pmtids = origininfo.getPMT()
    selectpmtinfo = pmtinfo.getPMTInfo(pmtids)
    HVs, channels = selectpmtinfo['HV_r'].values, origininfo.csv['HVCHANNEL'].values
    if args.on:
        # set the HV
        session = Session(host=device_ip, community='public', version=2)
        for HV, channel in zip(HVs, channels):
            session.getVoltage(board_i, channel)
            # session.setVoltage(board_i, channel, HV)
            # open the HV
            session.onCh(board_i, channel)
        # wait for the HV on
        time.sleep(30)
        # get the HV
        for HV, channel in zip(HVs, channels):
            session.getVoltage(board_i, channel)
    else:
        # close the HV
        session = Session(host=device_ip, community='public', version=2)
        for HV, channel in zip(HVs, channels):
            session.getVoltage(board_i, channel)
            # close the HV
            session.offCh(board_i, channel)
        # wait for the HV off
        time.sleep(30)
        # get the HV
        for HV, channel in zip(HVs, channels):
            session.getVoltage(board_i, channel)