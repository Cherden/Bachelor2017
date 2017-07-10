setup odroid with ubuntu mate:

#set static ip
#TODO replace address with input
sudo mv /etc/network/interfaces /etc/network/interfaces.bak
sudo echo "auto eth0\niface eth0 inet static\n\taddress 192.168.1.233/24\n\tbroadcast 192.168.1.255\n\tgateway 192.168.1.1" >> /etc/network/interfaces
sudo service network-manager restart

#copy ssh key
#TODO if ssh-key does not exist, create
ssh-copy-id odroid@192.168.1.233


#route internet
#TODO

#update linux dist
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
sudo apt autoremove

#disbale gui
sudo systemctl set-default multi-user.target --force
sudo systemctl disable lightdm.service --force
sudo systemctl disable graphical.target --force
sudo systemctl disable plymouth.service --force

#TODO install libfreenect, protobuf

#protobuf
./autogen.sh
./configure
make -j4
sudo make install
sudo ldconfig

#libfreenect


#restart
sudo reboot
