#!/bin/bash

make clean
scp -r src gen Makefile odroid@192.168.1.234:/home/odroid/run
ssh -Cf odroid@192.168.1.234 "cd /home/odroid/run && make clean && make client"
#ssh -Cf odroid@192.168.1.232 "cd /home/odroid/run && make clean && make client"
#scp -r src gen Makefile odroid@192.168.1.233:/home/odroid/run
#ssh -Cf odroid@192.168.1.233 "cd /home/odroid/run && make clean && make client"
#ssh -Cf odroid@192.168.1.232 "cd /home/odroid/run && sudo ./" $1
make server
