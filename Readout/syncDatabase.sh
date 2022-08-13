No=$1
echo $No
user=greatofdream
host=bat.g.airelinux.org
remotedir=/mnt/neutrino
rsync runinfo/$No.csv $user@$host:$remotedir/pmtTest/$No/
rsync runinfo/RUNINFO.csv $user@$host:$remotedir/pmtTest
rsync runinfo/PMTINFO.csv $user@$host:$remotedir/pmtTest
rsync runinfo/TESTINFO.csv $user@$host:$remotedir/pmtTest