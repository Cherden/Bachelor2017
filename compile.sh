#!/bin/bash

copy_files () {
	for ip in $*
	do
		echo "Copying files to 192.168.1.$ip ..."
		scp -r src gen Makefile odroid@192.168.1.$ip:/home/odroid/run >> /dev/null 2>&1
	done
}

compile () {
	for ip in $*
	do
		echo "Compiling files for 192.168.1.$ip ..."
		ssh -Cf odroid@192.168.1.$ip "cd /home/odroid/run && make clean && make client" >> /dev/null 2>&1
	done
}

while getopts a:n: opt
do
	case $opt in
		a)
			echo "Copying and compiling for all."
			copy_files 232 233 234
			compile 232 233 234
			break
		;;
		n)
			echo "Copying and compiling one."
			copy_files $OPTARG
			# compile $OPTARG
			break
		;;
	esac
done

echo "Make server ..."
make clean >> /dev/null 2>&1
# make server >> /dev/null 2>&1

echo "Done."
