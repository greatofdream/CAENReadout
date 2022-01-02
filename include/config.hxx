#ifndef _CONFIG_H
#define _CONFIG_H
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <iostream>
using namespace std;
class Board{
public:
    int BoardId;
    int triggerch;
    vector<int> samplech;
    unsigned int vmebaseaddress;
};
class Config{
public:
    // 整个VME机箱信息
    int N;
    vector<unsigned int> VMEBaseAddress;
    //选中的板子的信息
    int32_t samplen;
    int8_t postTriggerRatio;
    boost::property_tree::ptree pt;
    boost::property_tree::ptree board;
    vector<Board> boardinfo;
    Config(string filename){
        boost::property_tree::read_json(filename, pt);
        N = pt.get_child("N").get_value<int>();
        auto address = pt.get_child("address");
        for(auto it2=address.begin(); it2!=address.end(); it2++){
            VMEBaseAddress.push_back(stoi(it2->second.get_value<string>(),0,16));
        }
    }
    void setBoard(string filename){
        boost::property_tree::read_json(filename, board);
        samplen = board.get_child("sampleN").get_value<int32_t>();
        postTriggerRatio = board.get_child("postTriggerRatio").get_value<int8_t>();
        Board binfo;
        binfo.BoardId = board.get_child("BoardId").get_value<int>();
        binfo.triggerch = board.get_child("triggerch").get_value<int>();
        auto samplech = board.get_child("samplech");
        for(auto it2=samplech.begin(); it2!=samplech.end(); it2++){
            binfo.samplech.push_back(it2->second.get_value<int>());
        }
        binfo.vmebaseaddress = getVMEAddress(binfo.BoardId);
        boardinfo.push_back(binfo);

        cout<<"sampleN:"<<samplen<<endl;
        cout<<"boardid:"<<binfo.vmebaseaddress<<endl;
        // auto it = board.get_child("info");
        // for(auto it2=it.begin(); it2!=it.end();it2++){
        //     Board binfo;
        //     binfo.BoardId = it2->second.get_child("BoardId").get_value<int>();
        //     binfo.triggerch = it2->second.get_child("triggerch").get_value<int>();
        //     auto samplech = it2->second.get_child("samplech");
        //     for(auto it3=samplech.begin(); it3!=samplech.end(); it3++){
        //         binfo.samplech.push_back(it3->second.get_value<int>());
        //     }
        //     boardinfo.push_back(binfo);
        // }
    }
    int getBoardN(){
        return boardinfo.size();
    }
    int getVMEAddress(int id){
        if(id<0||id>N)
            return VMEBaseAddress[0];
        else
            return VMEBaseAddress[id];
    }
    void print(boost::property_tree::ptree const& pt)
    {
        using boost::property_tree::ptree;
        ptree::const_iterator end = pt.end();
        for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
            std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
            print(it->second);
        }
    }
};
#endif
