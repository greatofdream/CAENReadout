.PHONY:begin end
RunNo:=$(shell cat config/RunNo.txt)
PASSWORD:=$(shell cat password.txt)
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
HVon: runinfo/$(RunNo).csv runinfo/$(RunNo).json
	python3 HVcontrol.py --origincsv $< --pmtcsv runinfo/PMTINFO.csv --on
HVoff:
	python3 HVcontrol.py --origincsv runinfo/$(shell echo $(RunNo)-1|bc).csv --pmtcsv runinfo/PMTINFO.csv

Laseron:
	echo "$(PASSWORD)"|sudo -S python3 LaserControl.py --on
Laseroff:
	echo "$(PASSWORD)"|sudo -S python3 LaserControl.py

flow: runinfo/$(RunNo).json runinfo/$(shell echo $(RunNo)+1|bc).json
	make HVon
	python3 flow.py -i $<
	make Laseron
	python3 flow.py -i $(word 2,$^) --cool 10
	make Laseroff
	make HVoff

.DELETE_ON_ERROR: