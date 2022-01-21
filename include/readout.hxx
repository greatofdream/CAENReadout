#ifndef _READOUT_H
#define _READOUT_H
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "time.h"
#include "CAENVMElib.h"
#include "CAENVMEtypes.h"
#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"
#include "TTree.h"
#include "TFile.h"
#include "ReadoutType.h"
using namespace std;

#define ADDR_GLOBAL_TRG_MASK     0x810C
#define ADDR_TRG_OUT_MASK        0x8110
#define ADDR_FRONT_PANEL_IO_SET  0x811C
#define ADDR_ACQUISITION_MODE    0x8100
#define ADDR_EXT_TRG_INHIBIT     0x817C
#define ADDR_RUN_DELAY           0x8170
#define ADDR_FORCE_SYNC          0x813C
#define ADDR_RELOAD_PLL          0xEF34
#define ADDR_GROUP_TRG_MASK      0x10A8

// start on software command
#define RUN_START_ON_SOFTWARE_COMMAND     0xC
// start on S-IN level (logical high = run; logical low = stop)
#define RUN_START_ON_SIN_LEVEL            0xD
// start on first TRG-IN or Software Trigger
#define RUN_START_ON_TRGIN_RISING_EDGE    0xE
// start on LVDS I/O level
#define RUN_START_ON_LVDS_IO              0xF

#define CAEN_USE_DIGITIZERS
#define IGNORE_DPP_DEPRECATED

class ReadoutData{
    // 仅仅读取一个板子
public:
    int NSAMPLES;
    unsigned int VmeBaseAddress;
    int BoardId;
    int postTriggerRatio;
    int TriggerCh;
    vector<int> SampleCh;
    CAEN_DGTZ_ErrorCode ret;
    CAEN_DGTZ_BoardInfo_t BoardInfo;
    CAEN_DGTZ_EventInfo_t eventInfo;
    CAEN_DGTZ_UINT16_EVENT_t* Evt;
    CAEN_DGTZ_UINT16_EVENT_t* Event16;
    char* buffer;
    int handle;
    uint32_t Ns;
    uint32_t b, i, j;
    uint16_t b16, i16, j16;

    int c = 0;
    char * evtptr;
    uint32_t size;
    uint32_t bsize;
    double   TotalSize;
    uint32_t NTriggers;
    double   TotalWriteSize;
    uint32_t numEvents;

    
    short        TriggerMode;
    short        MultiThres;  // trigger when multiplicity >= threshold 
    unsigned int DcOffset[8];
    unsigned int Pedestal[8];
    unsigned int Threshold[8];
    unsigned int TriggerNo;

    int OverThreshold; /* A flag to tag if a channel is overthreshold */

    /* Time in second*/
    long long T0Sec, T0NanoSec, TNowSec, TNowNanoSec, TLastSec; 
    /* Write waveform to json file. Add by Guo Ziyi 2017/08/08 */
    double TLastSec_json;
    /* End */

    long long tadd;
    /* Trigger time delay */
    //double TrigTDelay[MAXNB] = {0, -48, -96, -144};
    //double TrigTDelay[MAXNB] = {0, -32, -64, -80};
    // double windowDelay[MAXNB] = {20, 20, 20, 20};
    // double TrigTDelay[MAXNB] = {0, 0, 0, 0};
    // double RunDelay[MAXNB] = {10, 6, 3, 0};
    //double TrigTDelay[MAXNB] = {0, -64, -96, -112};
    double TrigTDelay = 0;
    /* Output TFile and TTree */
    TFile* OutFile;
    Readout_t Readout;
    TTree* ReadoutTree;

    /* Screen output time interval */
    unsigned int TimeInt = 1;

    char Output[50];
    char LogFil[50];
    int RunNo;
    char RunType[50];

    ReadoutData(unsigned int vmebaseaddress, int boardid, int nsample, int posttriggerratio);
    void setTriggerMode(int triggermode, int triggerch=0, int multithres=1);
    void setSampleCh(vector<int> &samplech);
    void setPedestal(string config="config/pedestal.txt");
    int checkCommand();
    int readRunNo(string RunNoFN= "config/RunNo.txt");
    void sampleData();
};
#endif
