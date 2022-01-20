#! /bin/bash -e

# echo -e 'Subject: test

motionDir="motion/$1"
echo "ReconCam alert: motionDir='$motionDir'"

if [ -e "sendMailEnable.txt" ]; then

	mails=$(cat mails.txt | tr '\n' ' ')

	if [ -z "$mails" ]; then
		echo "no mail found"
		exit 1
	fi

	mpack -s "ReconCam alert: $1" $motionDir/trace.jpg $mails
else
	echo "sendMail is disable"

fi
