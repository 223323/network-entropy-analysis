#!/bin/bash

# dirs
archives_dir=.
ns3_dir=../../ns3/ns-allinone-3.26/ns-3.26

# cmdline vars
entropy_cmd=
merge_cmd=
postfix=
ns3_app=
ns3_app_cmd=
ns3_pcap_name=
pcap=
start_attack=0.01
attack_times="$(realpath attack-times.csv)"

endtime=100
subintervals=10

calling_cmd="$@"

# get merge_cmd and entropy_cmd params
while [ $# -gt 0 ];
do
	if [ "$1" == '--' ]; then
		shift
		break
	fi
	
	if [ $1 == '-m' ] || [ $1 == '--merge-cmd' ]; then
		merge_cmd="$merge_cmd $2"
		shift
	elif [ $1 == '-a' ] || [ $1 == '--app-cmd' ] || [ $1 == '--cmd' ]; then
		entropy_cmd="$entropy_cmd $2"
		shift
	elif [ $1 == '-p' ]; then
		postfix=$2
		shift
	elif [ $1 == '-h' ] || [ $1 == '--help' ]; then
		echo 'for NS3:'
		echo '	./process.sh --ns3-app synflood --ns3-pcap-name pcap-1-0.pcap'
		echo 'standalone:'
		echo '	./process.sh --pcap <pcap_filepath> [--end-time 60] [--attack-times=attack-times.csv] [-m merge_cmd] [-a entropy_cmd] [-p postfix]'
		exit
	elif [ $1 == '--tsalis' ]; then
		postfix="${postfix}-tsalis"
		entropy_cmd="$entropy_cmd --tsalis"
	elif [ $1 == '--renyi' ]; then
		postfix="${postfix}-renyi"
		entropy_cmd="$entropy_cmd --renyi"
	elif [ $1 == '--byte-entropy' ]; then
		postfix="${postfix}-byte-entropy"
		entropy_cmd="$entropy_cmd --byte-entropy"
	elif [ $1 == '--no-verbose' ] || [ $1 == '-s' ] || [ $1 == '-q' ]; then
		# silent/quiet
		entropy_cmd="$entropy_cmd --no-verbose"
	elif [ $1 == '--ns3-app' ]; then
		ns3_app=$2
		shift
	elif [ $1 == '--ns3-app-cmd' ]; then
		ns3_app_cmd=$2
		shift
	elif [ $1 == '--ns3-pcap-name' ] || [ $1 == '--ns3-pcap' ]; then
		ns3_pcap_name=$2
		shift
	elif [ $1 == '-p' ] || [ $1 == '--pcap' ]; then
		pcap=$(realpath $2)
		shift
	elif [ $1 == '--start-attack' ]; then
		start_attack=$2
		shift
	elif [ $1 == '--time-scale' ]; then
		entropy_cmd="$entropy_cmd --time-scale $2"
		shift
	elif [ $1 == '--subintervals' ]; then
		subintervals=$2
		entropy_cmd="$entropy_cmd --subintervals $subintervals"
		shift
	elif [ $1 == '--end-time' ] || [ $1 == '--endtime' ] || [ $1 == '--max-time' ]; then
		endtime=$2
		shift
	elif [ $1 == '--plot-end-time' ]; then
		pltendtime=$2
		shift
	elif [ $1 == '--attack-times' ]; then
		attack_times="$(realpath "$2")"
		shift
	fi
	shift
done



echo 'merge_cmd = ' $merge_cmd
echo 'entropy cmd = ' $entropy_cmd
echo 'ns3_app = ' $ns3_app
echo 'ns3_app_cmd = ' $ns3_app_cmd

# cd to project base dir
# cd ../

# process ns3 app
if [ ! -z $ns3_app ]; then
	echo 'launching ns3'
	ns3_run() {
		app_name=$1
		work_dir=work
		mkdir -p $work_dir
		export ns3_cwd_dir=$work_dir/$app_name
		rm -rf $ns3_cwd_dir/*
		mkdir -p $ns3_cwd_dir
		./waf --cwd $ns3_cwd_dir --run $app_name --command-template="%s $cmd"
	}

	pushd $ns3_dir
	ns3_run $ns3_app $ns3_app_cmd
	pcap=$(realpath $ns3_cwd_dir/$ns3_pcap_name)
	if [ -z $pcap ]; then
		echo 'pcap not found at ' $pcap
		exit
	fi
	popd
fi



# backup prev output if exists
if [ -e output ]
then
	bkp_to=output-$RANDOM.zip
	bkp_path=$archives_dir/backups
	while [ -e $bkp_path/$bkp_to ];
	do	
		bkp_to=output-$RANDOM.zip
	done
	zip -r $bkp_path/$bkp_to.zip output
	rm -rf output
fi

# compile
make all

if [ $? != 0 ]; then
	echo 'compiling failed'
	exit
fi


pltendtime=${pltendtime:-$endtime}

# run entropy analysis
echo '------------------'
echo 'running entropy analysis'
echo ./entropy $pcap $entropy_cmd
./entropy "$pcap" $entropy_cmd --end-time $endtime

if [ $? != 0 ]; then
	echo
	echo 'entropy processing failed'
	rm -rf output
	exit
fi

# run merge if given params
if [ ${#merge_cmd} -gt 0 ]; then
	echo 'running merge'
	./merge $merge_cmd
	if [ $? != 0 ]; then
		echo 'merge failed'
	fi
fi

# generate diagrams
cd output

echo '------------------'
echo 'plotting diagrams'
octave-cli --path "../scripts" --eval "plot_all($start_attack, $pltendtime, $subintervals, \"$attack_times\")" # 2> /dev/null
echo '------------------'
echo "$0 $calling_cmd" > cmd.txt


# back to project base dir
cd ..

# outputs folder
outputs_folder=$archives_dir/outputs
mkdir -p $outputs_folder
date=$(date "+%d.%m.%Y_%H:%M:%S")
output_name=$(basename "$pcap")"$postfix"-"$date"

# archive as zip
zip_dir=$archives_dir/archives
mkdir -p $zip_dir
archive=$zip_dir/$output_name.zip
echo 'creating archive ' $archive
zip -r $archive output > /dev/null

# move to outputs folder
mv output $outputs_folder/$output_name

echo output saved to $outputs_folder/$output_name

