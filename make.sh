scp -Cr src root@192.168.7.2:/root/
ssh -C root@192.168.7.2 "cd /root/src && make && ./Test"
