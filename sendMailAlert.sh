#! /bin/bash -e

# echo -e 'Subject: test

motionDir="motion/$1"
echo "ReconCam alert: motionDir='$motionDir'"

mails=$(cat mail.txt)

if [ -z "$mails" ]; then
	echo "no mail found"
	exit 1
fi

mpack -s "ReconCam alert: $1" $motionDir/trace.jpg $mails

# echo $1
