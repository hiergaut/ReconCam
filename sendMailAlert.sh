#! /bin/bash -e

# echo -e 'Subject: test
# Testing ssmtp' | sudo sendmail -v gauthierbouyjou2@gmail.com

motionDir="motion/$1"
echo "ReconCam alert: motionDir='$motionDir'"

mpack -s "ReconCam alert: $1" $motionDir/trace.jpg gauthierbouyjou@aol.com
# mpack -s "ReconCam alert: $1" $motionDir/video.webm gauthierbouyjou@aol.com

# echo $1
