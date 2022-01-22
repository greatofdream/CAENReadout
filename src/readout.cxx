#include "readout.hxx"
ReadoutData::ReadoutData(unsigned int vmebaseaddress, int boardid, int nsample, int posttriggerratio):TotalSize(0),NTriggers(0),TotalWriteSize(0),TriggerNo(0),TriggerMode(0),MultiThres(0){
        this->BoardId = boardid;
        this->NSAMPLES = nsample;
        this->VmeBaseAddress = vmebaseaddress;
        this->postTriggerRatio = posttriggerratio;
    }
void ReadoutData::setTriggerMode(int triggermode, int triggerch, int multithres){
        this->TriggerMode = triggermode;
        this->MultiThres = multithres;
        this->TriggerCh = triggerch;
        cout<<"Trigger Channel:"<< TriggerCh<<endl;
    }
void ReadoutData::setSampleCh(vector<int> &samplech){
        SampleCh = samplech;
    }
void ReadoutData::setDevice(){
    /********************************************/
        /* Configure V1751 digitizers               */
        /********************************************/
        ret = CAEN_DGTZ_Success;

        ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_PCIE_OpticalLink, 0, 0, VmeBaseAddress, &handle);
	    //cout<<"1751 handle "<<handle<<endl;
        if(ret != CAEN_DGTZ_Success) {
            cout<<"Can't open digitizer"<<endl;
            cout<<"Can't open digitizer"<<endl;
            goto QuitProgram;
        }
        /* Once we have the handler to the digitizer, we use it to call the other functions */
        ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
        cout<<endl<<"Connected to CAEN Digitizer Model "<<BoardInfo.ModelName<<", recognized as board "<<b<<endl;
        cout<<"ROC FPGA Release is "<<BoardInfo.ROC_FirmwareRel<<"; \tAMC FPGA Release is "<<BoardInfo.AMC_FirmwareRel<<endl; 
        cout<<endl<<"Connected to CAEN Digitizer Model "<<BoardInfo.ModelName<<", recognized as board "<<b<<endl;
        cout<<"ROC FPGA Release is "<<BoardInfo.ROC_FirmwareRel<<"; \tAMC FPGA Release is "<<BoardInfo.AMC_FirmwareRel<<endl;	
        //    ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);            /* Get Board Info */
        ret = CAEN_DGTZ_Reset(handle);                          /* Reset Digitizer */
        ret = CAEN_DGTZ_SetRecordLength(handle,NSAMPLES);       /* Set the lenght of each waveform (in samples) */
        ret = CAEN_DGTZ_SetChannelEnableMask(handle,0xFF);      /* Enable channel 0-7 */
        //ret = CAEN_DGTZ_SetInterruptConfig(handle,CAEN_DGTZ_DISABLE,1,1,1,CAEN_DGTZ_IRQ_MODE_ROAK);
        ret = CAEN_DGTZ_SetPostTriggerSize(handle, postTriggerRatio);   /* Post trigger size in percentage */

        for( c=0; c<8; c++ )  {
            ret = CAEN_DGTZ_SetChannelDCOffset(handle, c, DcOffset[c]);              /* Set channel DC offsect */
            ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, c, Threshold[c]);     /* Set selfTrigger threshold */
            ret = CAEN_DGTZ_SetTriggerPolarity(handle, c, CAEN_DGTZ_TriggerOnFallingEdge);  /* Set falling edge trigger for channel 0 */
        }

        // >>>> zaq chy
        if(TriggerMode==2){//外部触发
            ret = CAEN_DGTZ_SetTriggerPolarity(handle, TriggerCh, CAEN_DGTZ_TriggerOnRisingEdge);  /* Set falling edge trigger for channel 0 */
        }
        // <<<<<<

        ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_EXTOUT_ONLY, 0xFF);  /* Set trigger on channel 0-7 to be EXTOUT_ONLY */

        //cout<<hex<<data<<dec<<endl;
        if( TriggerMode == 1 || TriggerMode==2)  {
            unsigned int data;
            ret = CAEN_DGTZ_ReadRegister(handle, ADDR_GLOBAL_TRG_MASK, &data);
            // >>>>>>>>>>> zaq,chy
            data = data  | (0x0900000+(1<<TriggerCh));   /* Majority>3, Coincidence window A, channel 0-4 */
            ret = CAEN_DGTZ_WriteRegister(handle, ADDR_GLOBAL_TRG_MASK, data);  //  Majority trigger 4 on channel 0-4
            cout<<hex<<data<<dec<<endl;
        }
        ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle,1);                             /* Set the max number of events to transfer in a sigle readout */
        // >>>>>>>>>>>>>>> zaq chy
        ret = CAEN_DGTZ_SetAcquisitionMode(handle,CAEN_DGTZ_SW_CONTROLLED);   /* Set the acquisition mode */
        //ret = CAEN_DGTZ_SetAcquisitionMode(handle,CAEN_DGTZ_FIRST_TRG_CONTROLLED);   /* Set the acquisition mode */

        if(ret != CAEN_DGTZ_Success) {
            cout<<"Errors during Digitizer Configuration."<<endl;
            cout<<"Errors during Digitizer Configuration."<<endl;
            goto QuitProgram;
        }
        ret = CAEN_DGTZ_MallocReadoutBuffer(handle,&buffer,&size);
        ret = CAEN_DGTZ_AllocateEvent(handle, (void**)&Event16);
        ret = CAEN_DGTZ_ClearData(handle);

        ret = CAEN_DGTZ_SWStartAcquisition(handle);
        unsigned int offset, thres;
        for(int chl=0; chl<8; chl++) {
            CAEN_DGTZ_GetChannelDCOffset( handle, chl, &offset );
            CAEN_DGTZ_GetChannelTriggerThreshold( handle, chl, &thres );
        //cout<<offset<<" "<<thres<<"  ";
        }
    QuitProgram:
        // Free the buffers and close the digitizers
            //ret = CAEN_DGTZ_ClearData(handle);
            ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);
            ret = CAEN_DGTZ_SWStopAcquisition(handle);
            ret = CAEN_DGTZ_CloseDigitizer(handle);   

}
void ReadoutData::SampleOne(){
    while(1) {
        int Trigger = 1;
        uint32_t nEvts = 0;
        uint32_t FADCdataReady;
        // Check vme status register to get the data ready bit
        ret = CAEN_DGTZ_ReadRegister(handle, 0xEF04, &FADCdataReady); //vme status
        FADCdataReady = FADCdataReady & 0x1;
        //  cout<<"Data Ready : "<<FADCdataReady<<endl;
        Trigger = Trigger & FADCdataReady;
        long long dtWaveform;
        if( Trigger ) {
            Readout.ChannelId.clear();
            Readout.Waveform.clear();
            ret = CAEN_DGTZ_ReadData(handle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,buffer,&bsize);  /* Read the buffer from the digitizer */

            /* decoding */
            ret = CAEN_DGTZ_GetNumEvents(handle, buffer, bsize, &nEvts);
            ret = CAEN_DGTZ_GetEventInfo(handle,buffer,bsize,0,&eventInfo,&evtptr);
            ret = CAEN_DGTZ_DecodeEvent(handle,evtptr,(void**)&Event16);
            Ns = Event16->ChSize[0];
            for(i=0; i<SampleCh.size(); i++)  {
                    OverThreshold = 0;
                    Readout.ChannelId.push_back(SampleCh[i]);
                    
                    for(j=0; j<Ns; j++)  {

                        int idx = j;
                        Readout.Waveform.push_back( Event16->DataChannel[SampleCh[i]][idx] );

                        /* Do a sample-by-sample comparison with threshold */
                        if( OverThreshold == 0 )  {
                            if( Event16->DataChannel[SampleCh[i]][idx] <= Threshold[SampleCh[i]] )  {
                                OverThreshold = 1;
                            }
                        }
                    }
            }
        break;
        }
    }
    return;
}
void ReadoutData::setPedestal(string config){
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
            if(b==BoardId){
                iss>> DcOffset[c] >> Pedestal[c] >> Threshold[c];
                // Threshold[c] = Pedestal[c]-10;
            }
        }
        /* config.txt printout */
        for(c=0;c<8;c++)  {
            cout<<BoardId<<"\t"<<c<<"\t"<< DcOffset[c]<<"\t"<<Pedestal[c]<<"\t"<<Threshold[c]<<endl;
        }
        if( TriggerMode==0 ) cout<<"Pedestal run"<<endl;
        else if( TriggerMode==1 ) cout<<"Physics run"<<endl;
        else if(TriggerMode==2) cout<<"External Trigger"<<endl;
    }
int ReadoutData::checkCommand() {
        int c = 0;
        std::ifstream infile( "config/end" );
        if( infile.is_open() ) {
            c=1; // exit command
        }
        return c;
    }
int ReadoutData::readRunNo(string RunNoFN){
        std::fstream RunNoFile( RunNoFN.c_str() );

        if( !(RunNoFile.is_open()) ) {
            cout<<"Run number file "<<RunNoFN.c_str()<<" cannot be open."<<endl;
            exit(0);
        }
        /* Run No file */
        char f;
        std::string line;
        while (std::getline(RunNoFile, line))
        {
            f=line[0];
            if( f!='#' ) {
                std::istringstream iss(line);
                iss>> RunNo;
                break;
            }
        }
        // RunNo += 1, save back to the same file.
        RunNoFile.close();
        std::ofstream RunNoUpdate( RunNoFN.c_str() );
        unsigned int NewRunNo=RunNo+1;
        RunNoUpdate << NewRunNo <<endl;
    }
void ReadoutData::sampleData(){
        time_t tm;
        struct tm *ptm;
        time(&tm);
        ptm = localtime(&tm);

        /* Run type */
        if( TriggerMode==0 )  // Pedestal run
        { strcpy(RunType, "Ped"); }
        else if( TriggerMode==1 )
        { strcpy(RunType, "Phy"); } 
        else{
            strcpy(RunType, "Ext");
        }

        sprintf(Output,"data/Jinping_1ton_%s_%.4d%.2d%.2d_%.8d.root",RunType,ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,RunNo);
        sprintf(LogFil,"log/Jinping_1ton_%s_%.4d%.2d%.2d_%.8d.log",RunType,ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,RunNo);
        cout<<"Data written to "<<Output<<endl;
        cout<<"Log file written to "<<LogFil<<endl;
        std::ofstream logfile( LogFil );


        /********************************************/
        /* Configure V1751 digitizers               */
        /********************************************/
        ret = CAEN_DGTZ_Success;

        ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_PCIE_OpticalLink, 0, 0, VmeBaseAddress, &handle);
	    //cout<<"1751 handle "<<handle<<endl;
        if(ret != CAEN_DGTZ_Success) {
            cout<<"Can't open digitizer"<<endl;
            logfile<<"Can't open digitizer"<<endl;
            goto QuitProgram;
        }
        /* Once we have the handler to the digitizer, we use it to call the other functions */
        ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
        cout<<endl<<"Connected to CAEN Digitizer Model "<<BoardInfo.ModelName<<", recognized as board "<<b<<endl;
        cout<<"ROC FPGA Release is "<<BoardInfo.ROC_FirmwareRel<<"; \tAMC FPGA Release is "<<BoardInfo.AMC_FirmwareRel<<endl; 
        logfile<<endl<<"Connected to CAEN Digitizer Model "<<BoardInfo.ModelName<<", recognized as board "<<b<<endl;
        logfile<<"ROC FPGA Release is "<<BoardInfo.ROC_FirmwareRel<<"; \tAMC FPGA Release is "<<BoardInfo.AMC_FirmwareRel<<endl;	
        //    ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);            /* Get Board Info */
        ret = CAEN_DGTZ_Reset(handle);                          /* Reset Digitizer */
        ret = CAEN_DGTZ_SetRecordLength(handle,NSAMPLES);       /* Set the lenght of each waveform (in samples) */
        ret = CAEN_DGTZ_SetChannelEnableMask(handle,0xFF);      /* Enable channel 0-7 */
        //ret = CAEN_DGTZ_SetInterruptConfig(handle,CAEN_DGTZ_DISABLE,1,1,1,CAEN_DGTZ_IRQ_MODE_ROAK);
        ret = CAEN_DGTZ_SetPostTriggerSize(handle, postTriggerRatio);   /* Post trigger size in percentage */

        for( c=0; c<8; c++ )  {
            ret = CAEN_DGTZ_SetChannelDCOffset(handle, c, DcOffset[c]);              /* Set channel DC offsect */
            ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, c, Threshold[c]);     /* Set selfTrigger threshold */
            ret = CAEN_DGTZ_SetTriggerPolarity(handle, c, CAEN_DGTZ_TriggerOnFallingEdge);  /* Set falling edge trigger for channel 0 */
        }

        // >>>> zaq chy
        if(TriggerMode==2){//外部触发
            ret = CAEN_DGTZ_SetTriggerPolarity(handle, TriggerCh, CAEN_DGTZ_TriggerOnRisingEdge);  /* Set falling edge trigger for channel 0 */
        }
        // <<<<<<

        ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_EXTOUT_ONLY, 0xFF);  /* Set trigger on channel 0-7 to be EXTOUT_ONLY */

        //cout<<hex<<data<<dec<<endl;
        if( TriggerMode == 1 || TriggerMode==2)  {
            unsigned int data;
            ret = CAEN_DGTZ_ReadRegister(handle, ADDR_GLOBAL_TRG_MASK, &data);
            // >>>>>>>>>>> zaq,chy
            data = data  | (0x0900000+(1<<TriggerCh));   /* Majority>3, Coincidence window A, channel 0-4 */
            ret = CAEN_DGTZ_WriteRegister(handle, ADDR_GLOBAL_TRG_MASK, data);  //  Majority trigger 4 on channel 0-4
            cout<<hex<<data<<dec<<endl;
        } 

            //if( TriggerMode == 1 )  {
            //ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0xFF);
            //ret = CAEN_DGTZ_WriteRegister(handle, ADDR_GLOBAL_TRG_MASK, 0xC0000040);  // up
            //ret = CAEN_DGTZ_WriteRegister(handle, ADDR_GLOBAL_TRG_MASK, 0xC0000080);  // down
            //}

            /*
            uint32_t nc = 0;
            for(nc=0; nc<8; nc++)
            {
                CAEN_DGTZ_TriggerMode_t mode;
                CAEN_DGTZ_GetChannelSelfTrigger(handle, nc, &mode);
                printf("Get self trigger nc%d: %d\n", nc, mode);
            }
            */

        ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle,1);                             /* Set the max number of events to transfer in a sigle readout */
        // >>>>>>>>>>>>>>> zaq chy
        ret = CAEN_DGTZ_SetAcquisitionMode(handle,CAEN_DGTZ_SW_CONTROLLED);   /* Set the acquisition mode */
        //ret = CAEN_DGTZ_SetAcquisitionMode(handle,CAEN_DGTZ_FIRST_TRG_CONTROLLED);   /* Set the acquisition mode */

        if(ret != CAEN_DGTZ_Success) {
            cout<<"Errors during Digitizer Configuration."<<endl;
            logfile<<"Errors during Digitizer Configuration."<<endl;
            goto QuitProgram;
        }

        /* Malloc Readout Buffer.
        NOTE1: The mallocs must be done AFTER digitizer's configuration!
        NOTE2: In this example we use the same buffer, for every board. We
        use the first board to allocate the buffer, so if the configuration
        is different for different boards (or you use different board models), may be
        that the size to allocate must be different for each one. */
        ret = CAEN_DGTZ_MallocReadoutBuffer(handle,&buffer,&size);
        ret = CAEN_DGTZ_AllocateEvent(handle, (void**)&Event16);

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
    
        ret = CAEN_DGTZ_ClearData(handle);

        ret = CAEN_DGTZ_SWStartAcquisition(handle);

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

        uint32_t FADCdataReady;
        int Trigger;
        uint32_t nEvts;
        uint32_t Counter;
        long long TimeTag;
        long long PrevTimeTag;
        long long TimeTagSec, TimeTagNanoSec;
        uint32_t NRoll;

        nEvts=0;
        Counter=0;
        NRoll=0;
        PrevTimeTag = 0;
    
        unsigned int offset, thres;
        for(int chl=0; chl<8; chl++) {
            CAEN_DGTZ_GetChannelDCOffset( handle, chl, &offset );
            CAEN_DGTZ_GetChannelTriggerThreshold( handle, chl, &thres );
        //cout<<offset<<" "<<thres<<"  ";
        }
      //cout<<endl;


        while(1) {
            if( TriggerMode == 0 ) {
                usleep(1e4);
                ret = CAEN_DGTZ_SendSWtrigger(handle);
                }

            /* All boards must have triggers at the same time, otherwise wait. */
            Trigger=1;
            // Check vme status register to get the data ready bit
            ret = CAEN_DGTZ_ReadRegister(handle, 0xEF04, &FADCdataReady); //vme status
            FADCdataReady = FADCdataReady & 0x1;
            //  cout<<"Data Ready : "<<FADCdataReady<<endl;
            Trigger = Trigger & FADCdataReady;
            long long dtWaveform;

            //    cout<<FADCdataReady<<endl;
            /* Decode data */
            if( Trigger ) {
                NTriggers++;
                TriggerNo++;
                Readout.ChannelId.clear();
                Readout.Waveform.clear();

                ret = CAEN_DGTZ_ReadData(handle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,buffer,&bsize);  /* Read the buffer from the digitizer */

                /* decoding */
                ret = CAEN_DGTZ_GetNumEvents(handle, buffer, bsize, &nEvts);
                ret = CAEN_DGTZ_GetEventInfo(handle,buffer,bsize,0,&eventInfo,&evtptr);
                ret = CAEN_DGTZ_DecodeEvent(handle,evtptr,(void**)&Event16);

                /* Keep some record */	 
                TotalSize += bsize/1.0e6; // convert to MB 

                Counter = eventInfo.EventCounter;
                TimeTag = (long long)(eventInfo.TriggerTimeTag & 0x7FFFFFFF) * 8;
                //cout<<"Board "<<b<<" TimeTag[b]-TimeTag[0]: "<<TimeTag[b]-TimeTag[0]<<endl;

                /* The following calculation require event rate in 1/s level */
                //cout<<"TimeTag::"<<TimeTag[b]<<"  Previous TimeTag::"<<PrevTimeTag[b]<<endl;
                if(TimeTag-PrevTimeTag<0)  {
                    NRoll++;
                }
                PrevTimeTag = TimeTag;
                
                TimeTag += (eventInfo.Pattern >> 16) & 0x7;
                TimeTagSec = TimeTag/(long long)1e9;
                TimeTagNanoSec = TimeTag%(long long)1e9;
                
                tadd = (long long)NRoll*(8*(((long long)1<<31)-1));  /* in nanosecond */
                //cout<<"NRoll::"<<NRoll<<"  tadd::"<<tadd/1e9<<endl;

                /* Fill tree contents */
                Ns = Event16->ChSize[0];

                /* Add in each channel and sample */
                dtWaveform = (tadd-tadd) + (TimeTagNanoSec - TimeTagNanoSec)
                    +(TimeTagSec-TimeTagSec)*(long long)1e9 - TrigTDelay;
				//if(TriggerNo==1 || TriggerNo%500==0) 
				//cout<<b<<":\t"<<dtWaveform[b]+TrigTDelay[b]<<endl;
            

                for(i=0; i<SampleCh.size(); i++)  {
                    OverThreshold = 0;
                    Readout.ChannelId.push_back(SampleCh[i]);
                    //cout<<"Ns["<<b<<"]: "<<Ns[b]<<endl;
                    /*
                    if( b==0 && i==0 )  {
                    for(int kk=0; kk<10; kk++)  {
                    cout<< Event16[b]->DataChannel[i][kk] <<" ";
                    }
                    cout<<endl;
                    }
                    */

                    for(j=0; j<Ns; j++)  {

                        int idx = j;
                        Readout.Waveform.push_back( Event16->DataChannel[SampleCh[i]][idx] );

                        /* Do a sample-by-sample comparison with threshold */
                        if( OverThreshold == 0 )  {
                            if( Event16->DataChannel[SampleCh[i]][idx] <= Threshold[SampleCh[i]] )  {
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
            
            TotalWriteSize += ((Readout.Waveform.size()+Readout.ChannelId.size())/1e6*4); /* in MB */

            /* Fill some per-trigger information */
            //cout<<setiosflags(ios::fixed);
            //cout<<"T0+tadd[0]+TimeTag[0] "<<T0<<" "<<tadd[0]<<" "<<TimeTag[0]<<endl;
            long long tempSec = T0Sec+TimeTagSec;
            long long tempNanoSec = T0NanoSec+tadd+TimeTagNanoSec;
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
            cout<<"\nBoard "<<b<<": Retrieved "<<Counter+1<<" Events\n"<<endl;
            logfile<<"\nBoard "<<b<<": Retrieved "<<Counter+1<<" Events\n"<<endl;
        goto QuitProgram;

        /********************************************/
        /*          Quit program routine            */
        /********************************************/
    QuitProgram:
        // Free the buffers and close the digitizers
            //ret = CAEN_DGTZ_ClearData(handle);
            ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);
            ret = CAEN_DGTZ_SWStopAcquisition(handle);
            ret = CAEN_DGTZ_CloseDigitizer(handle);   

        if(ReadoutTree)  {
            ReadoutTree->Write();
            // Pay attention to the last line. It is necessary to file splitting.
            // http://root.cern.ch/root/htmldoc/TTree.html#TTree:ChangeFile
            OutFile = ReadoutTree->GetCurrentFile(); //to get the pointer to the current file
            OutFile->Close();
        }
    }