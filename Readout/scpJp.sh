No=$(($1-1))
echo $No
ssh jinping@JinpingNu "mkdir -p /srv/JinpingData/Jinping_1ton_Data/pmtTest/$No ;"
src="data/Jinping_1ton_*_202*_00000$No*.root"
scp ${src} jinping@JinpingNu:/srv/JinpingData/Jinping_1ton_Data/pmtTest/$No/
