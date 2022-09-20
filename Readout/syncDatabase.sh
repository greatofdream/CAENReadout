No=$1
echo $No
user=greatofdream
host=bat.g.airelinux.org
remotedir=$(python3 -c "import config;print(config.remoteDir)")
echo "start transfer meta data to" $remotedir
rsync -P runinfo/$No.csv $user@$host:$remotedir/pmtTest
rsync -P runinfo/RUNINFO.csv $user@$host:$remotedir/pmtTest
rsync -P runinfo/PMTINFO.csv $user@$host:$remotedir/pmtTest
rsync -P runinfo/TESTINFO.csv $user@$host:$remotedir/pmtTest
