#include "config.hxx"
#include "readout.hxx"
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <vector>
#include <iostream>
using namespace std;
namespace po = boost::program_options;
int main(int argc, char** argv)
{
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
    ReadoutData readout = ReadoutData(deviceinfo.boardinfo[0].vmebaseaddress, deviceinfo.boardinfo[0].BoardId, deviceinfo.samplen, deviceinfo.postTriggerRatio);
    if(type=="trigger"){
        readout.setTriggerMode(1);
    }else if(type=="darknoise"){
        readout.setTriggerMode(0);
    }else if(type=="extrigger"){
        readout.setTriggerMode(2);
    }
    readout.setPedestal();
    // readout.sampleData();
    return 0;
}
