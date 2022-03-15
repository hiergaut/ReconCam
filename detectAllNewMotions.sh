#! /bin/bash


while true; do
	isProcess=false

	for dir in $(find motion -mindepth 4 -maxdepth 4 -type d -atime -1 | sort -r); do
		if [ ! -e $dir/detection.mp4 ]; then
			if [ -e $dir/record.mp4 ]; then
				echo "##########################################################"
				./a.out -stream="$dir/record.mp4" --script=./sendMailAlert.sh
			fi
			touch $dir/detection.mp4
			isProcess=true
			break
		fi
	done

	if ! $isProcess; then
		sleep 10
		echo -n "#"
	fi
done
