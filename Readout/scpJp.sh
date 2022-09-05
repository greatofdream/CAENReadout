No=$(($1-1))
echo $No
user=greatofdream
host=bat.g.airelinux.org
remotedir=/mnt/neutrino
ssh $user@$host "mkdir -p $remotedir/pmtTest/$No ;"
src="data/Jinping_1ton_*_202*_00000$No*.root"
rsync -avHP ${src} $user@$host:$remotedir/pmtTest/$No/
./syncDatabase.sh $No
