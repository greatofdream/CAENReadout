#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include "main.h"
#include "readout.hxx"
#include "config.hxx"
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <vector>
#include <iostream>
using namespace std;
namespace po = boost::program_options;
MyMainFrame::MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h):TGMainFrame(p,w,h) {


   // Create canvas widget
   fEcanvas = new TRootEmbeddedCanvas("Ecanvas",this,200,200);
   this->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX |
                   kLHintsExpandY, 10,10,10,1));
   // Create a horizontal frame widget with buttons
   TGHorizontalFrame *hframe = new TGHorizontalFrame(this,200,40);
   TGTextButton *draw = new TGTextButton(hframe,"&Draw");
   draw->Connect("Clicked()","MyMainFrame",this,"DoDraw()");
   hframe->AddFrame(draw, new TGLayoutHints(kLHintsCenterX,
                                            5,5,3,4));
   TGTextButton *exit = new TGTextButton(hframe,"&Exit",
                                "gApplication->Terminate(0)");
   hframe->AddFrame(exit, new TGLayoutHints(kLHintsCenterX,
                                            5,5,3,4));
   this->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,
                                             2,2,2,2));

   // Set a name to the main frame
   this->SetWindowName("Simple Example");

   // Map all subwindows of main frame
   this->MapSubwindows();

   // Initialize the layout algorithm
   this->Resize(this->GetDefaultSize());

   // Map main frame
   this->MapWindow();
}
void MyMainFrame::SampleOne(){
    waveinfo = readout->SampelOne();
}
void MyMainFrame::DoDraw() {
   sampleOne();
   // Draws function graphics in randomly chosen interval
   TF1 *f1 = new TF1("f1","sin(x)/x",0,gRandom->Rndm()*10);
   f1->SetLineWidth(3);
   f1->Draw();
   TCanvas *fCanvas = fEcanvas->GetCanvas();
   fCanvas->cd();
   fCanvas->Update();
}
MyMainFrame::~MyMainFrame() {
   // Clean up used widgets: frames, buttons, layout hints
   this->Cleanup();
   delete this;
}
MyMainFrame* example() {
   // Popup the GUI...
   return new MyMainFrame(gClient->GetRoot(),200,200);
}

int main(int argc, char **argv) {
    // Parsing Arguments
    po::options_description program("readout");
    program.add_options()(
        "help,h", "Help")(
        "input,i", po::value<string>(), "run number file")(
        "output,o", po::value<string>(), "Output H5 directory")(
        "type,t", po::value<string>()->default_value("extrigger"), "Sample mode, valid option: trigger, darknoise, extrigger")(
        "setting,s", po::value<string>(), "Setting file for device")(
        "config,c", po::value<string>(), "Config file of selected board and channel");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, program), vm);
    po::notify(vm);
    if (vm.count("help"))
    {
        cout << program << endl;
        return 1;
    }
    auto inputfilename = vm["input"].as<string>();
    auto outputfilename = vm["output"].as<string>();
    auto type = vm["type"].as<string>();
    auto settingfilename = vm["setting"].as<string>();
    auto configfilename = vm["config"].as<string>();
    Config deviceinfo = Config(settingfilename);
    deviceinfo.setBoard(configfilename);

    MyMainFrame* app = example();
    app->readout = new ReadoutData(deviceinfo.boardinfo[0].vmebaseaddress, deviceinfo.boardinfo[0].BoardId, deviceinfo.samplen, deviceinfo.postTriggerRatio);
    readout->readRunNo(inputfilename);
    if(type=="trigger"){
        readout->setTriggerMode(1, deviceinfo.boardinfo[0].triggerch);
    }else if(type=="pedestal"){
        readout->setTriggerMode(0);
    }else if(type=="extrigger"){
        readout->setTriggerMode(2, deviceinfo.boardinfo[0].triggerch);
    }
    readout->setSampleCh(deviceinfo.boardinfo[0].samplech);
    readout->setPedestal();
    readout->setDevice();
    TApplication theApp("App",&argc,argv);
    
    theApp.Run();
    return 0;
}