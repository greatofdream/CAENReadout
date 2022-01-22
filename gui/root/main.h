#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include "../../include/readout.hxx"
#include "../../include/ReadoutType.h"
class MyMainFrame: public TGMainFrame {
private:
   TRootEmbeddedCanvas *fEcanvas;
   Readout_t* waveinfo;
public:
    ReadoutData* readout;
    MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h);
    virtual ~MyMainFrame();
    void SampleOne();
    void DoDraw();
    ClassDef(MyMainFrame,0)
};