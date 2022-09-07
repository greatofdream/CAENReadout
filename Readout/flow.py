coolingTime = 12 * 60 * 60 # 静止12h
import argparse
from CSVData import OriginINFO
import json
import datetime
import threading
import subprocess
def Logger(process):
    while process.poll() is None:
        line = process.stdout.readline()
        line = line.strip()
        if line:
            print(line.decode("utf8", 'ignore'))
def startSample():
    print('start at {}'.format(datetime.datetime.now()))
    process = subprocess.Popen('make begin', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    Logger(process)
def HVon():
    print('HV on')
    process = subprocess.Popen('make HVon', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    Logger(process)
def HVoff():
    print('HV off')
    process = subprocess.Popen('make HVoff',
        shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    Logger(process)
def endSample():
    print('end at {}'.format(datetime.datetime.now()))
    subprocess.Popen('make end', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    process = subprocess.Popen('make copy', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    Logger(process)
psr = argparse.ArgumentParser()
psr.add_argument('-i', dest='ipt', help='config json')
psr.add_argument('--cool', type=int, default=coolingTime, help='coolingTime')
args = psr.parse_args()
with open(args.ipt, 'r') as ipt:
    jsondata = json.load(ipt)
timeLength = jsondata['time']
coolingTime = args.cool

beginTimer = threading.Timer(coolingTime, startSample)
endTimer = threading.Timer(coolingTime + timeLength * 60, endSample)

print('start cooling at {}'.format(datetime.datetime.now()))
print('1 load sampler: +{}s'.format(coolingTime))
beginTimer.start()
print('2 load end sampler: +{}s'.format(coolingTime + timeLength * 60))
endTimer.start()