No=$(($1-1))
echo $No
user=greatofdream
host=bat.g.airelinux.org
remotedir=/mnt/neutrino
ssh $user@$host "mkdir -p $remotedir/pmtTest/$No ;"
src="data/Jinping_1ton_*_202*_00000$No*.root"
rsync -a ${src} $user@$host:$remotedir/pmtTest/$No/
rsync runinfo/$No.csv $user@$host:$remotedir/pmtTest/$No/
rsync runinfo/RUNINFO.csv $user@$host:$remotedir/pmtTest
rsync runinfo/PMTINFO.csv $user@$host:$remotedir/pmtTest