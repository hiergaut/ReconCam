#! /bin/bash


while true; do

	for dir in $(find motion -mindepth 4 -type d); do
		if [ ! -e $dir/detection.mp4 ]; then
			if [ -e $dir/record.mp4 ]; then
				echo "##########################################################"
				./a.out -stream="$dir/record.mp4" --script=./sendMailAlert.sh
			fi
			touch $dir/detection.mp4
		fi
	done

	sleep 10
	echo -n "#"
done
