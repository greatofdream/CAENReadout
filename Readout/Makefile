.PHONY:begin end
RunNo:=$(shell cat config/RunNo.txt)
DN:
	rm config/end
	../build/Readout -i config/RunNo.txt -o data -t darknoise -s config/setting.json -c config/DNconfig.json
QE:
	rm config/end
	../build/Readout -i config/RunNo.txt -o data -t extrigger -s config/setting.json -c config/QEconfig.json
DCR:
	rm config/end
	../build/Readout -i config/RunNo.txt -o data -t extrigger -s config/setting.json -c config/DCRconfig.json
AP:
	rm config/end
	../build/Readout -i config/RunNo.txt -o data -t extrigger -s config/setting.json -c config/APconfig.json

begin: runinfo/$(RunNo).json
	rm config/end
	../build/Readout -i config/RunNo.txt -o data -t extrigger -s config/setting.json -c runinfo/$(RunNo).json
end:
	echo end>config/end

copy:
	cat config/RunNo.txt| xargs ./scpJp.sh

runinfo/%.json: runinfo/%.csv
	python3 CSVData.py --run $* --origincsv runinfo/$*.csv --runcsv runinfo/RUNINFO.csv --pmtcsv runinfo/PMTINFO.csv --testcsv runinfo/TESTINFO.csv -o $@
flow: runinfo/$(RunNo).json
	python3 flow.py -i $^
.DELETE_ON_ERROR: