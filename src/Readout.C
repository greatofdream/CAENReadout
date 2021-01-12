/*
   Jinping Neutrino Experiment
   One Ton Prototype 
   Online Readout Program

   Author: Zhe Wang
   Date:   July 23, 2016

   Single V1751 board, majority trigger
   Ziyi Guo, Zhe Wang, 2019, 2020

 */

#include <stdio.h>
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
#include <iomanip>

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

#define NBOARDS  1  /* Number of connected boards */
#define MAXNB    8

//#define NSAMPLES 2000  /* Size of each waveform in samples, i.e. ns */ 
#define NSAMPLES 400
int checkCommand() {
    int c = 0;

    std::ifstream infile( "config/end" );

    if( infile.is_open() ) {
        c=1; // exit command
    }
    return c;
}

int main(int argc, char* argv[])
{
    cout<<endl;
    cout<<"    Jinping Neutrino Experiment   "<<endl;
    cout<<"         One Ton Prototype        "<<endl; 
    cout<<"      Online Readout Program      "<<endl;
    cout<<endl;
    /* The following variable is the type returned from most of CAENDigitizer
       library functions and is used to check if there was an error in function
       execution. For example:
       ret = CAEN_DGTZ_some_function(some_args);
       if(ret) printf("Some error"); */
    CAEN_DGTZ_ErrorCode ret;

    /* The following variable will be used to get an handler for the digitizer. The
       handler will be used for most of CAENDigitizer functions to identify the board */
    int	handle[NBOARDS];

    CAEN_DGTZ_BoardInfo_t BoardInfo;
    CAEN_DGTZ_EventInfo_t eventInfo[NBOARDS];
    CAEN_DGTZ_UINT16_EVENT_t *Evt = NULL;
    char* buffer[NBOARDS] = {};
    CAEN_DGTZ_UINT16_EVENT_t  *Event16[NBOARDS] = {};
    uint32_t Ns[NBOARDS];

    uint32_t b, i, j;
    uint16_t b16, i16, j16;

    int c = 0;
    char * evtptr = NULL;
    uint32_t size,bsize[NBOARDS];
    double   TotalSize = 0;
    uint32_t NTriggers = 0;
    double   TotalWriteSize = 0;
    uint32_t numEvents;

    unsigned int VmeBaseAddress[MAXNB] = {0x00090000, 0x00030000, 0x00090000, 0x000c0000};

    short        TriggerMode = 0;
    short        MultiThres = 0;  // trigger when multiplicity >= threshold 
    unsigned int DcOffset[MAXNB][8];
    unsigned int Pedestal[MAXNB][8];
    unsigned int Threshold[MAXNB][8];
    unsigned int TriggerNo = 0;

    int OverThreshold; /* A flag to tag if a channel is overthreshold */

    /* Time in second*/
    long long T0Sec, T0NanoSec, TNowSec, TNowNanoSec, TLastSec; 
    /* Write waveform to json file. Add by Guo Ziyi 2017/08/08 */
    double TLastSec_json;
    /* End */

    long long tadd[NBOARDS];
    /* Trigger time delay */
    //double TrigTDelay[MAXNB] = {0, -48, -96, -144};
    //double TrigTDelay[MAXNB] = {0, -32, -64, -80};
    double windowDelay[MAXNB] = {20, 20, 20, 20};
    double TrigTDelay[MAXNB] = {0, 0, 0, 0};
    double RunDelay[MAXNB] = {10, 6, 3, 0};
    //double TrigTDelay[MAXNB] = {0, -64, -96, -112};

    /* Output TFile and TTree */
    TFile* OutFile;
    Readout_t Readout;
    TTree* ReadoutTree=0;

    /* Screen output time interval */
    unsigned int TimeInt = 1;

    char Output[50];
    char LogFil[50];
    int RunNo;
    char RunType[50]; 

    /********************************************/
    /* Read in parameters of run configuration  */
    /********************************************/
    {
        std::string config = "config/config.txt";
        std::ifstream infile( config.c_str() );

        if( !(infile.is_open()) ) {
            cout<<"Run configuration "<<config.c_str()<<" cannot be open."<<endl;
            exit(0);
        }
        std::string line;

        /* Trigger Mode */
        char f;
        while (std::getline(infile, line))
        {
            f=line[0];
            if( f!='#' ) {
                std::istringstream iss(line);
                iss>> TriggerMode;
                break;
            }
        }

        /* Multiplicity trigger threshold */
        while (std::getline(infile, line))
        {
            f=line[0];
            if( f!='#' ) {
                std::istringstream iss(line);
                iss>> MultiThres;
                break;
            }
        }
        if( TriggerMode == 0 ) {
            MultiThres = 1000;   /* 1000 unphysical, to turn off physics trigger */
        }

    }


    /********************************************/
    /* Read in parameters of pedestals          */
    /********************************************/
    {
        std::string config = "config/pedestal.txt";
        std::ifstream infile( config.c_str() );

        if( !(infile.is_open()) ) {
            cout<<"Pedestal constants "<<config.c_str()<<" cannot be open."<<endl;
            exit(0);
        }

        std::string line;
        char f;
        while (std::getline(infile, line))
        {	  
            f=line[0];	  
            if( f=='#' ) continue;
            std::istringstream iss(line);
            iss>> b >> c;
            iss>> DcOffset[b][c] >> Pedestal[b][c] >> Threshold[b][c];
	    Threshold[b][c] = Pedestal[b][c]-10;
        }
    }

    /* config.txt printout */
    
    for(b=0;b<NBOARDS;b++)  {
        for(c=0;c<8;c++)  {
	  cout<<b<<"\t"<<c<<"\t"<< DcOffset[b][c]<<"\t"<<Pedestal[b][c]<<"\t"<<Threshold[b][c]<<endl;
	}
    }
    if( TriggerMode==0 ) cout<<"Pedestal run"<<endl;
    if( TriggerMode==1 ) cout<<"Physics run"<<endl;

    /********************************************/
    /*      Date, RunNo, Output, Log file name  */
    /********************************************/
    { 
        time_t tm;
        struct tm *ptm;
        time(&tm);
        ptm = localtime(&tm);

        std::string RunNoFN = "config/RunNo.txt";
        std::fstream RunNoFile( RunNoFN.c_str() );

        if( !(RunNoFile.is_open()) ) {
            cout<<"Run number file "<<RunNoFN.c_str()<<" cannot be open."<<endl;
            exit(0);
        }
        std::string line;

        /* Run type */
        if( TriggerMode==0 )  // Pedestal run
        { strcpy(RunType, "Ped"); }
        else
        { strcpy(RunType, "Phy"); } 

        /* Run No file */
        char f;
        while (std::getline(RunNoFile, line))
        {
            f=line[0];
            if( f!='#' ) {
                std::istringstream iss(line);
                iss>> RunNo;
                break;
            }
        }

        sprintf(Output,"data/Jinping_1ton_%s_%.4d%.2d%.2d_%.8d.root",RunType,ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,RunNo);
        sprintf(LogFil,"log/Jinping_1ton_%s_%.4d%.2d%.2d_%.8d.log",RunType,ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,RunNo);
        cout<<"Data written to "<<Output<<endl;
        cout<<"Log file written to "<<LogFil<<endl;

        // RunNo += 1, save back to the same file.
        RunNoFile.close();
        std::ofstream RunNoUpdate( RunNoFN.c_str() );
        unsigned int NewRunNo=RunNo+1;
        RunNoUpdate << NewRunNo <<endl;
    }

    /* Prepare log file */
    std::ofstream logfile( LogFil );


    /********************************************/
    /* Configure V1751 digitizers               */
    /********************************************/
    ret = CAEN_DGTZ_Success;
    for(b=0; b<NBOARDS; b++){
        ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_PCIE_OpticalLink, 0, 0, VmeBaseAddress[b], &handle[b]);
	    //cout<<"1751 handle "<<handle[b]<<endl;
        if(ret != CAEN_DGTZ_Success) {
            cout<<"Can't open digitizer"<<endl;
            logfile<<"Can't open digitizer"<<endl;
            goto QuitProgram;
        }
        /* Once we have the handler to the digitizer, we use it to call the other functions */
        ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);
        cout<<endl<<"Connected to CAEN Digitizer Model "<<BoardInfo.ModelName<<", recognized as board "<<b<<endl;
        cout<<"ROC FPGA Release is "<<BoardInfo.ROC_FirmwareRel<<"; \tAMC FPGA Release is "<<BoardInfo.AMC_FirmwareRel<<endl; 
        logfile<<endl<<"Connected to CAEN Digitizer Model "<<BoardInfo.ModelName<<", recognized as board "<<b<<endl;
        logfile<<"ROC FPGA Release is "<<BoardInfo.ROC_FirmwareRel<<"; \tAMC FPGA Release is "<<BoardInfo.AMC_FirmwareRel<<endl;	
    //    ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);            /* Get Board Info */
        ret = CAEN_DGTZ_Reset(handle[b]);                          /* Reset Digitizer */
        ret = CAEN_DGTZ_SetRecordLength(handle[b],NSAMPLES);       /* Set the lenght of each waveform (in samples) */
        ret = CAEN_DGTZ_SetChannelEnableMask(handle[b],0xFF);      /* Enable channel 0-7 */
        //ret = CAEN_DGTZ_SetInterruptConfig(handle[b],CAEN_DGTZ_DISABLE,1,1,1,CAEN_DGTZ_IRQ_MODE_ROAK);
        ret = CAEN_DGTZ_SetPostTriggerSize(handle[b], 2);   /* Post trigger size in percentage */

        for( c=0; c<8; c++ )  {
            ret = CAEN_DGTZ_SetChannelDCOffset(handle[b], c, DcOffset[b][c]);              /* Set channel DC offsect */
	    ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle[b], c, Threshold[b][c]);     /* Set selfTrigger threshold */
            ret = CAEN_DGTZ_SetTriggerPolarity(handle[b], c, CAEN_DGTZ_TriggerOnFallingEdge);  /* Set falling edge trigger for channel 0 */
        }

        // >>>> zaq chy
        c = 0;
            ret = CAEN_DGTZ_SetTriggerPolarity(handle[b], c, CAEN_DGTZ_TriggerOnRisingEdge);  /* Set falling edge trigger for channel 0 */
        // <<<<<<

        ret = CAEN_DGTZ_SetChannelSelfTrigger(handle[b], CAEN_DGTZ_TRGMODE_EXTOUT_ONLY, 0xFF);  /* Set trigger on channel 0-7 to be EXTOUT_ONLY */

	//cout<<hex<<data<<dec<<endl;
	if( TriggerMode == 1 )  {
      unsigned int data;
	  ret = CAEN_DGTZ_ReadRegister(handle[b], ADDR_GLOBAL_TRG_MASK, &data);
      // >>>>>>>>>>> zaq,chy
	  data = data  | 0x0900001;   /* Majority>3, Coincidence window A, channel 0-4 */
	  ret = CAEN_DGTZ_WriteRegister(handle[b], ADDR_GLOBAL_TRG_MASK, data);  //  Majority trigger 4 on channel 0-4
	  cout<<hex<<data<<dec<<endl;
	} 

	//if( TriggerMode == 1 )  {
	//ret = CAEN_DGTZ_SetChannelSelfTrigger(handle[b], CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0xFF);
	//ret = CAEN_DGTZ_WriteRegister(handle[b], ADDR_GLOBAL_TRG_MASK, 0xC0000040);  // up
	//ret = CAEN_DGTZ_WriteRegister(handle[b], ADDR_GLOBAL_TRG_MASK, 0xC0000080);  // down
	//}

	/*
	uint32_t nc = 0;
	for(nc=0; nc<8; nc++)
	  {
	    CAEN_DGTZ_TriggerMode_t mode;
	    CAEN_DGTZ_GetChannelSelfTrigger(handle[b], nc, &mode);
	    printf("Get self trigger nc%d: %d\n", nc, mode);
	  }
	*/

        ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle[b],1);                             /* Set the max number of events to transfer in a sigle readout */
        // >>>>>>>>>>>>>>> zaq chy
        ret = CAEN_DGTZ_SetAcquisitionMode(handle[b],CAEN_DGTZ_SW_CONTROLLED);   /* Set the acquisition mode */
        //ret = CAEN_DGTZ_SetAcquisitionMode(handle[b],CAEN_DGTZ_FIRST_TRG_CONTROLLED);   /* Set the acquisition mode */

        if(ret != CAEN_DGTZ_Success) {
            cout<<"Errors during Digitizer Configuration."<<endl;
            logfile<<"Errors during Digitizer Configuration."<<endl;
            goto QuitProgram;
        }
    }

    /* Malloc Readout Buffer.
    NOTE1: The mallocs must be done AFTER digitizer's configuration!
    NOTE2: In this example we use the same buffer, for every board. We
    use the first board to allocate the buffer, so if the configuration
    is different for different boards (or you use different board models), may be
    that the size to allocate must be different for each one. */
    for(b=0; b<NBOARDS; b++)  {
        ret = CAEN_DGTZ_MallocReadoutBuffer(handle[b],&buffer[b],&size);
        ret = CAEN_DGTZ_AllocateEvent(handle[b], (void**)&Event16[b]);
    }

    /********************************************/
    /*           Output Root Tree               */
    /********************************************/
    /* filename: Jinping_1ton_Phy_20170401_00i000001.root */
    OutFile = new TFile(Output,"recreate");

    ReadoutTree = new TTree("Readout", "TriggerReadout");
    ReadoutTree->SetMaxTreeSize(200000000);  // 200M, otherwise split into a new file
    ReadoutTree->Branch("RunNo",      &Readout.RunNo);
    ReadoutTree->Branch("TriggerNo",  &Readout.TriggerNo);
    ReadoutTree->Branch("TriggerType",&Readout.TriggerType);
    ReadoutTree->Branch("TriggerTag", &Readout.TriggerTag);
    ReadoutTree->Branch("DetectorID", &Readout.DetectorID);
    ReadoutTree->Branch("Sec",        &Readout.Sec);
    ReadoutTree->Branch("NanoSec",    &Readout.NanoSec);
    ReadoutTree->Branch("ChannelId",  &Readout.ChannelId);
    ReadoutTree->Branch("Waveform",   &(Readout.Waveform));

    /********************************************/
    /*           Start Run Operation            */
    /********************************************/
    usleep(300000);
    for(b=0; b<NBOARDS; b++)  {
        ret = CAEN_DGTZ_ClearData(handle[b]);

        ret = CAEN_DGTZ_SWStartAcquisition(handle[b]);
    }
    //cout<<"CAEN_DGTZ_SWStartAcquisition"<<endl;
    //cout<<"Data taking started"<<endl;
    logfile<<"Data taking started"<<endl;

    /********************************************/
    /*          Start acquisition loop          */
    /********************************************/
    struct timeval tv;
    gettimeofday(&tv, 0);
    T0Sec = tv.tv_sec; T0NanoSec = tv.tv_usec*1000;
    TLastSec = T0Sec;

    /* Write waveform to json file. Add by Guo Ziyi 2017/08/08 */
    TLastSec_json = tv.tv_sec+tv.tv_usec*1e-6;
    /* End */

    uint32_t FADCdataReady[NBOARDS];
    int Trigger;
    uint32_t nEvts[NBOARDS];
    uint32_t Counter[NBOARDS];
    long long TimeTag[NBOARDS];
    long long PrevTimeTag[NBOARDS];
    long long TimeTagSec[NBOARDS], TimeTagNanoSec[NBOARDS];
    uint32_t NRoll[NBOARDS];
    for(b=0; b<NBOARDS; b++) {
        nEvts[b]=0;
        Counter[b]=0;
        NRoll[b]=0;
        PrevTimeTag[b] = 0;
    }
    
    unsigned int offset, thres;
    for(b=0; b<NBOARDS; b++) {
      for(int chl=0; chl<8; chl++) {
	CAEN_DGTZ_GetChannelDCOffset( handle[0], chl, &offset );
	CAEN_DGTZ_GetChannelTriggerThreshold( handle[0], chl, &thres );
	//cout<<offset<<" "<<thres<<"  ";
      }
      //cout<<endl;
    }


    while(1) {
       

       if( TriggerMode == 0 ) {
          usleep(1e4);
	      ret = CAEN_DGTZ_SendSWtrigger(handle[0]);
	    }

        /* All boards must have triggers at the same time, otherwise wait. */
        Trigger=1;
        for(b=0; b<NBOARDS; b++)  {
            // Check vme status register to get the data ready bit
            ret = CAEN_DGTZ_ReadRegister(handle[b], 0xEF04, &FADCdataReady[b]); //vme status
            FADCdataReady[b] = FADCdataReady[b] & 0x1;
          //  cout<<"Data Ready : "<<FADCdataReady[b]<<endl;
            Trigger = Trigger & FADCdataReady[b];
        }
        long long dtWaveform[NBOARDS];

    //    cout<<FADCdataReady[b]<<endl;
        /* Decode data */
        if( Trigger ) {
            NTriggers++;
            TriggerNo++;
            Readout.ChannelId.clear();
            Readout.Waveform.clear();

            for(b=0; b<NBOARDS; b++)  {
                ret = CAEN_DGTZ_ReadData(handle[b],CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,buffer[b],&bsize[b]);  /* Read the buffer from the digitizer */

                /* decoding */
                ret = CAEN_DGTZ_GetNumEvents(handle[0], buffer[b], bsize[b], &nEvts[b]);
                ret = CAEN_DGTZ_GetEventInfo(handle[b],buffer[b],bsize[b],0,&eventInfo[b],&evtptr);
                ret = CAEN_DGTZ_DecodeEvent(handle[b],evtptr,(void**)&Event16[b]);

                /* Keep some record */	 
                TotalSize += bsize[b]/1.0e6; // convert to MB 

                Counter[b] = eventInfo[b].EventCounter;
                TimeTag[b] = (long long)(eventInfo[b].TriggerTimeTag & 0x7FFFFFFF) * 8;
                //cout<<"Board "<<b<<" TimeTag[b]-TimeTag[0]: "<<TimeTag[b]-TimeTag[0]<<endl;

                /* The following calculation require event rate in 1/s level */
                //cout<<"TimeTag::"<<TimeTag[b]<<"  Previous TimeTag::"<<PrevTimeTag[b]<<endl;
                if(TimeTag[b]-PrevTimeTag[b]<0)  {
                    NRoll[b]++;
                }
                PrevTimeTag[b] = TimeTag[b];
                
                TimeTag[b] += (eventInfo[b].Pattern >> 16) & 0x7;
                TimeTagSec[b] = TimeTag[b]/(long long)1e9;
                TimeTagNanoSec[b] = TimeTag[b]%(long long)1e9;
                
                tadd[b] = (long long)NRoll[b]*(8*(((long long)1<<31)-1));  /* in nanosecond */
                //cout<<"NRoll::"<<NRoll[b]<<"  tadd::"<<tadd[b]/1e9<<endl;

                /* Fill tree contents */
                Ns[b] = Event16[b]->ChSize[0];

                /* Add in each channel and sample */
                dtWaveform[b] = (tadd[b]-tadd[0]) + (TimeTagNanoSec[b] - TimeTagNanoSec[0])
                    +(TimeTagSec[b]-TimeTagSec[0])*(long long)1e9 - TrigTDelay[b];
				//if(TriggerNo==1 || TriggerNo%500==0) 
				//cout<<b<<":\t"<<dtWaveform[b]+TrigTDelay[b]<<endl;
            }

            for(b=0; b<NBOARDS; b++)  {
                for(i=0; i<8; i++)  {
                    OverThreshold = 0;
                    Readout.ChannelId.push_back(b*8+i);
                    //cout<<"Ns["<<b<<"]: "<<Ns[b]<<endl;
		    /*
		    if( b==0 && i==0 )  {
		      for(int kk=0; kk<10; kk++)  {
			cout<< Event16[b]->DataChannel[i][kk] <<" ";
		      }
		      cout<<endl;
		    }
		    */

                    for(j=0; j<Ns[b]; j++)  {

                        int idx = j;
                        Readout.Waveform.push_back( Event16[b]->DataChannel[i][idx] );

                        /* Do a sample-by-sample comparison with threshold */
                        if( OverThreshold == 0 )  {
                            if( Event16[b]->DataChannel[i][idx] <= Threshold[b][i] )  {
                                OverThreshold = 1;
                            }
                        }
                    }

                    /* Discard trival channel information if not software trigger, zero suppression */
		    /*
                    if(TriggerMode == 1 && OverThreshold == 0) {
		        Readout.ChannelId.pop_back();	      
		        Readout.Waveform.erase( Readout.Waveform.end()-Ns[b], Readout.Waveform.end() );
                    }
		    */
		    
                }
                Readout.RunNo = RunNo;
                Readout.DetectorID = 1;
                Readout.TriggerNo = TriggerNo;
                Readout.TriggerType = MultiThres;
		Readout.TriggerTag = TriggerMode;
            }
            TotalWriteSize += ((Readout.Waveform.size()+Readout.ChannelId.size())/1e6*4); /* in MB */

            /* Fill some per-trigger information */
            //cout<<setiosflags(ios::fixed);
            //cout<<"T0+tadd[0]+TimeTag[0] "<<T0<<" "<<tadd[0]<<" "<<TimeTag[0]<<endl;
            long long tempSec = T0Sec+TimeTagSec[0];
            long long tempNanoSec = T0NanoSec+tadd[0]+TimeTagNanoSec[0];
            tempSec += tempNanoSec/(long long)1e9;
            Readout.Sec = tempSec;
            Readout.NanoSec = tempNanoSec%(long long)1e9;


            gettimeofday(&tv, 0);
            TNowSec = tv.tv_sec;
            TNowNanoSec = tv.tv_usec*1000;
            logfile<<"The data is ready!!!"<<endl;
            logfile<<"Time comapre :"<<endl;
            logfile<<"Readout.Sec : "<<tempSec<<endl;
            logfile<<"Unix time : "<<TNowSec<<endl;;
            /* One output summary per second */
            if( TNowSec-TLastSec > TimeInt )  {  
                TLastSec = TNowSec;
                double dt1 = TNowSec-T0Sec+(TNowNanoSec-T0NanoSec)*1e-9;
                cout<<" Trigger rate "<<NTriggers/dt1<<"/s "<<TotalSize/dt1<<" Mbyte/s"<<endl;
                cout<<"Disk writting rate is "<<TotalWriteSize/dt1<<" Mbyte/s (Before zipping)"<<endl<<endl;
                logfile<<" Trigger rate "<<NTriggers/dt1<<"/s "<<TotalSize/dt1<<" Mbyte/s"<<endl;
                logfile<<"Disk writting rate is "<<TotalWriteSize/dt1<<" Mbyte/s (Before zipping)"<<endl<<endl;

                TimeInt += sqrt(TimeInt);
            }	  

            /* Fill the tree */
            ReadoutTree->Fill();

        }

        c = checkCommand();
        if (c == 1) goto Continue;
    }   

Continue:
    for(b=0; b<NBOARDS; b++)  {
        cout<<"\nBoard "<<b<<": Retrieved "<<Counter[b]+1<<" Events\n"<<endl;
        logfile<<"\nBoard "<<b<<": Retrieved "<<Counter[b]+1<<" Events\n"<<endl;
    }
    goto QuitProgram;

    /********************************************/
    /*          Quit program routine            */
    /********************************************/
QuitProgram:
    // Free the buffers and close the digitizers
    for(b=0; b<NBOARDS; b++)  {
        //ret = CAEN_DGTZ_ClearData(handle[b]);
        ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer[b]);
        ret = CAEN_DGTZ_SWStopAcquisition(handle[b]);
        ret = CAEN_DGTZ_CloseDigitizer(handle[b]);   
    }

    if(ReadoutTree)  {
        ReadoutTree->Write();
        // Pay attention to the last line. It is necessary to file splitting.
        // http://root.cern.ch/root/htmldoc/TTree.html#TTree:ChangeFile
        OutFile = ReadoutTree->GetCurrentFile(); //to get the pointer to the current file
        OutFile->Close();
    }

    return 0;
}
