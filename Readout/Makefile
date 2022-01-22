.PHONY:begin end
DN:
	rm config/end
	../build/Readout -i config/RunNo.txt -o data -t extrigger -s config/setting.json -c config/DNconfig.json

begin:
	rm config/end
	../build/Readout -i config/RunNo.txt -o data -t extrigger -s config/setting.json -c config/config.json
end:
	echo end>config/end

copy:
	cat config/RunNo.txt| xargs ./scpJp.sh
