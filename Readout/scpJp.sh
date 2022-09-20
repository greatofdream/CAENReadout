No=$(($1-1))
echo $No
user=greatofdream
host=bat.g.airelinux.org
remotedir=$(python3 -c "import config;print(config.remoteDir)")
echo "start transfer sample data to" $remotedir
ssh $user@$host "mkdir -p $remotedir/pmtTest/$No ;"
src="data/Jinping_1ton_*_202*_00000$No*.root"
rsync -avHP ${src} $user@$host:$remotedir/pmtTest/$No/
./syncDatabase.sh $No
