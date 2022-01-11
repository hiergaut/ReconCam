#! /bin/bash

for dir in $(find motion -atime 2 -mindepth 4 -type d | sort); do
	rm -vf $dir
done

while true; do

	for dir in $(find motion -mindepth 4 -type d); do
		if [ ! -e $dir/detection.mp4 ]; then
			if [ -e $dir/record.mp4 ]; then
				echo "##########################################################"
				./a.out -stream="$dir/record.mp4"
			fi
			touch $dir/detection.mp4
		fi
	done

	sleep 10
	echo -n "#"
done
