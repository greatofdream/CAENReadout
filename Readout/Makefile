.PHONY:begin end
begin:
	rm config/end
	../build/Readout -i config/runno.txt -o data -t extrigger -s config/setting.json -c config/config.json
end:
	echo end>config/end
