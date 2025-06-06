# brnana
C(  o  .  o  ) ╯   brnana - the minimal bridge with extra potassium!🍌 

A lightweight Linux kernel module that implements a custom Ethernet bridge with basic port enslaving and forwarding support.

## Build
```
$ make
```
## Load the Module
```
$ sudo insmod brnana.ko num_bridge=1
```
Check if it is loaded:
```
$ lsmod | grep "brnana"
```
Check the Interface
```
$ ip addr
3: brnana0: <BROADCAST,MULTICAST,UP,LOWER_UP> ...
    link/ether 00:00:00:00:00:00 brd ff:ff:ff:ff:ff:ff
```

## Testing with Dummy Interfaces
```sh
# Load the dummy network driver (if not already loaded)
sudo modprobe dummy

# Create a dummy network interface named dummy0
sudo ip link add dummy0 type dummy

# Bring the dummy interface up
sudo ip link set dummy0 up

# Enslave dummy0 to the brnana bridge (e.g., brnana0)
sudo ip link set dummy0 master brnana0

# Detach dummy0 from the brnana bridge
sudo ip link set dummy0 nomaster

# Delete the dummy interface
sudo ip link delete dummy0
```
## Sample Output
```
$ ip addr
3: brnana0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UNKNOWN group default qlen 1000
    link/ether 00:00:00:00:00:00 brd ff:ff:ff:ff:ff:ff
4: dummy0: <BROADCAST,NOARP,UP,LOWER_UP> mtu 1500 qdisc noqueue master brnana0 state UNKNOWN group default qlen 1000
    link/ether 02:36:dd:b5:2d:e9 brd ff:ff:ff:ff:ff:ff
    inet6 fe80::36:ddff:feb5:2de9/64 scope link 
       valid_lft forever preferred_lft forever
```

```
$ sudo dmesg
[  107.502301] C( o . o ) ╯ brnana: 1 bridge loaded
[  107.502345] C( o . o ) ╯ brnana: bridge 0 init
[  107.502724] C( o . o ) ╯ brnana: New BR 0
[  107.534732] C( o . o ) ╯ brnana: bridge 0 open
[  150.731318] C( o . o ) ╯ brnana: enslaved dummy0 to brnana0
[  280.721538] C( o . o ) ╯ brnana: removing port dummy0 from brnana0
[  327.221292] C( o . o ) ╯ brnana: 1 bridge unloaded
[  327.221752] C( o . o ) ╯ brnana: bridge 0 stop
[  327.222751] C( o . o ) ╯ brnana: bridge 0 uninit
```
## Unload the Module
```
$ sudo rmmod brnana.ko
```