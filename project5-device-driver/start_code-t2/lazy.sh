sudo apt-get install bridge-utils
sudo apt-get install uml-utilities
sudo brctl addbr br0
sudo tunctl -t tap0 -u djf
sudo brctl addif br0 tap0 
sudo ifconfig tap0 up
