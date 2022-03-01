#! /bin/bash


while true; do

	for dir in $(find motion -mindepth 4 -maxdepth 4 -type d -atime -1 | sort -r); do
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
