#ssh -C root@192.168.7.2 "pkill -f ./Test"
scp -Cr src odroid@192.168.1.232:/home/odroid/run
ssh -Cf odroid@192.168.1.232 "cd /home/odroid/run && rm Test && make && ./Test &"
