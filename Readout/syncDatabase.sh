No=$1
echo $No
user=greatofdream
host=bat.g.airelinux.org
remotedir=/mnt/neutrino
rsync -P runinfo/$No.csv $user@$host:$remotedir/pmtTest
rsync -P runinfo/RUNINFO.csv $user@$host:$remotedir/pmtTest
rsync -P runinfo/PMTINFO.csv $user@$host:$remotedir/pmtTest
rsync -P runinfo/TESTINFO.csv $user@$host:$remotedir/pmtTest