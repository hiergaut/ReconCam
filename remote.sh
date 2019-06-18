#! /bin/bash -e

if [ $# -ne 1 ]; then                                                                                                  
        echo "usage : $0 event";                                                                                       
        exit 1                                                                                                         
fi                                                                                                                     

rep="/var/www/html/ReconCam"
scp -P 57954 alert.jpg pi@192.168.1.10:$rep
ssh pi@192.168.1.10 -p 57954 $rep/script.sh $1
