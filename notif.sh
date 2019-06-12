#! /bin/bash -e


if [ -z $1 ]; then
    echo "empty parameter"
    exit 1
fi

mkdir -p alert
touch alert/$1
sleep 18
rm alert/$1

find alert/ -empty -exec rm alert.jpg \;

