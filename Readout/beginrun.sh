cd ~/xtwork/mcpPMT/Readout
rm config/end
../build/Readout -i config/RunNo.txt -o data -t extrigger -s config/setting.json -c config/config.json
