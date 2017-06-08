#!/bin/bash

scp -r src gen Makefile odroid@192.168.1.232:/home/odroid/run
ssh -Cf odroid@192.168.1.232 "cd /home/odroid/run && make clean && make client"
#ssh -Cf odroid@192.168.1.232 "cd /home/odroid/run && sudo ./" $1
make clean && make server
