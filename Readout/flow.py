coolingTime = 12 * 60 * 60 # 静止12h
import argparse
from CSVData import OriginINFO
import json
import datetime
import threading
import subprocess
def startSample():
    print('start at {}'.format(datetime.datetime.now()))
    process = subprocess.Popen('make begin', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    while process.poll() is None:
        line = process.stdout.readline()
        line = line.strip()
        if line:
            print(line.decode("utf8", 'ignore'))
def endSample():
    print('end at {}'.format(datetime.datetime.now()))
    subprocess.Popen('make end', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    subprocess.Popen('make copy', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
psr = argparse.ArgumentParser()
psr.add_argument('-i', dest='ipt', help='config json')
args = psr.parse_args()
with open(args.ipt, 'r') as ipt:
    jsondata = json.load(ipt)
timeLength = jsondata['time']
beginTimer = threading.Timer(coolingTime, startSample)
endTimer = threading.Timer(coolingTime + timeLength * 60, endSample)
print('start cooling at {}'.format(datetime.datetime.now()))
print('wait for begin sample')
beginTimer.start()
print('wait for end sample')
endTimer.start()