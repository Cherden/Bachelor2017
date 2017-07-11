setup odroid with ubuntu mate:

#set static ip
#TODO replace address with input
sudo mv /etc/network/interfaces /etc/network/interfaces.bak
sudo echo "auto eth0\niface eth0 inet static\n\taddress 192.168.1.233/24\n\tbroadcast 192.168.1.255\n\tgateway 192.168.1.1" >> /etc/network/interfaces
sudo service network-manager restart

#disbale gui
sudo systemctl set-default multi-user.target --force
sudo systemctl disable lightdm.service --force
sudo systemctl disable graphical.target --force
sudo systemctl disable plymouth.service --force

#update linux dist
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
sudo apt autoremove

#install neceserray packages
sudo apt-get install git-core cmake freeglut3-dev pkg-config build-essential libxmu-dev libxi-dev libusb-1.0-0-dev autoconf automake libtool curl make g++ unzip

#reboot and connect with your PC
sudo reboot

#copy ssh key
#TODO if ssh-key does not exist, create
ssh-copy-id odroid@192.168.1.233

#copy repos
scp -r libfreenect/ protobuf/ odroid@192.168.1.233

#install libfreenect
cd libfreenect
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig

#install protobuf
cd ~/protobuf
./autogen.sh
./configure
cd src
make -j4
sudo make install
sudo ldconfig



#restart
sudo reboot
