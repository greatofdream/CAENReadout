/*
  Output Tree structure
  Extracted from JPReadout.hh
  
  Zhe Wang, 2016, 10
*/

#ifndef _READOUT_TYPE_H_
#define _READOUT_TYPE_H_

struct Readout_t {
  Int_t RunNo;
  Int_t TriggerNo;
  Int_t TriggerType;
  Int_t TriggerTag;
  Int_t DetectorID;
  Int_t Sec;
  Int_t NanoSec;
  std::vector<UInt_t> ChannelId;
  std::vector<UInt_t> Waveform;
};

#endif /* _READOUT_TYPE_H_ */
