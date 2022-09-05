'''
处理csv格式的数据
+ 获取高压信息
+ 写入RUNINFO.csv和TESTINFO.csv
python3 CSVData.py --run 697 --origincsv runinfo/697.csv --runcsv runinfo/RUNINFO.csv --pmtcsv runinfo/PMTINFO.csv --testcsv runinfo/TESTINFO.csv
'''
import pandas as pd
import numpy as np
import argparse
import datetime
import json
class CSVReader(object):
    def __init__(self, filename):
        self.csv = pd.read_csv(filename)
        self.filename = filename

class PMTINFO(CSVReader):
    # pmt信息获取: PMT,HV_r
    def __init__(self, filename):
        super(PMTINFO, self).__init__(filename)
        self.pmtinfo = self.csv.set_index('PMT')
    def getPMTInfo(self, pmt):
        return self.pmtinfo.loc[pmt]
class OriginINFO(CSVReader):
    # 输入的run信息获取:CHANNEL,BOXID,PMT,TRIGGER,MODE
    def __init__(self, filename):
        super(OriginINFO, self).__init__(filename)
    def getPMT(self):
        return self.csv['PMT']
    def getMode(self):
        return self.csv.iloc[0]['MODE']
class RUNINFO(CSVReader):
    # RUN信息设置: RUNNO,DATE,ISTRIGGER
    def __init__(self, filename):
        super(RUNINFO, self).__init__(filename)
        self.runinfo = self.csv.set_index('RUNNO')
    def updateAppend(self, runno, date, mode):
        # 更新或新增某一个run
        self.runinfo.loc[runno] = (date, mode)
    def save(self):
        self.runinfo.reset_index().to_csv(self.filename, index=False)
class TESTINFO(CSVReader):
    # test信息设置: RUNNO,CHANNEL,BOXID,HV,PMT
    def __init__(self, filename):
        super(TESTINFO, self).__init__(filename)
    def appendRun(self, runno, origininfo, HV):
        origininfo['RUNNO'] = runno
        origininfo['HV'] = HV.astype('float64').values
        self.csv = pd.concat([self.csv, origininfo], join="inner")
    def save(self):
        self.csv.to_csv(self.filename, index=False)
if __name__=="__main__":
    psr = argparse.ArgumentParser()
    psr.add_argument('-o', dest='opt', help='output config json')
    psr.add_argument('--origincsv', help='origin csv file')
    psr.add_argument('--runcsv', help='run csv file')
    psr.add_argument('--pmtcsv', help='pmt csv file')
    psr.add_argument('--testcsv', help='test csv file')
    psr.add_argument('--run', type=int, help='run no')
    args = psr.parse_args()

    pmtinfo = PMTINFO(args.pmtcsv)
    origininfo = OriginINFO(args.origincsv)
    runinfo = RUNINFO(args.runcsv)
    testinfo = TESTINFO(args.testcsv)

    pmtids = origininfo.getPMT()
    selectpmtinfo = pmtinfo.getPMTInfo(pmtids)
    x = datetime.datetime.now()
    mode = origininfo.getMode()
    runinfo.updateAppend(args.run, x.strftime("%Y-%m-%d-%H:%M"), mode)
    runinfo.save()
    testinfo.appendRun(args.run, origininfo.csv, selectpmtinfo['HV_r'])
    testinfo.save()
    # store config run info
    if mode==0:
        configjson = 'config/DCRconfig.json'
    else:
        configjson = 'config/APconfig.json'
    with open(configjson, 'r') as ipt:
        jsondata = json.load(ipt)
    triggerch = origininfo.csv['TRIGGER'].values[0]
    jsondata['triggerch'] = int(triggerch)
    if mode==0:
        jsondata['samplech'] = [int(i) for i in origininfo.csv['CHANNEL'].values]
    else:
        jsondata['samplech'] = [int(i) for i in np.insert(origininfo.csv['CHANNEL'].values, 0, triggerch)]
    print(jsondata)
    with open(args.opt, 'w') as opt:
        json.dump(jsondata, opt)


