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

execute () {
	for ip in $*
	do
		echo "Starting 192.168.1.$ip ..."
		ssh -Cf odroid@192.168.1.$ip "cd /home/odroid/run && sudo ./client" >> /dev/null 2>&1
	done
}

server () {
	echo "Compile server ..."
	make clean >> /dev/null 2>&1
	make server >> /dev/null 2>&1
}

usage () {
	echo "Usage of compile.sh"
	echo "\t-a\t| --all\t\t\tCompile all nodes and server."
	echo "\t-c [n]\t| --compile\t\tCompile the nodes given after this parameter."
	echo "\t-e [n] \t| --execute\t\tExecute the nodes given after this parameter."
}

IP_LIST="232 233 234"
SLEEP_TIME=23

if [ $# -gt 0 ]; then
	while [ "$1" != "" ]
	do
	    case $1 in
	        -a | --all )
						copy_files $IP_LIST
						compile $IP_LIST
						server
						sleep $SLEEP_TIME
						# execute $IP_LIST
						break
          ;;

					-c | --compile )
						while [ "$2" != "" ] && ! [[ "$2" =~ '^[0-9]+$' ]]
						do
							copy_files $2
							compile $2
							shift
						done
						sleep $SLEEP_TIME
						break
					;;

					-e | --execute )
						if [ "$2" == "all" ];then
							execute $IP_LIST
							break
						fi

						while [ "$2" != "" ] && ! [[ "$2" = '^[0-9]+$' ]]
						do
							execute $2
							shift
						done
						break
					;;

	        -h | --help )
						usage
            exit
          ;;

	        * )
						usage
            exit 1
	    esac
	    shift
	done
else
	usage
fi
