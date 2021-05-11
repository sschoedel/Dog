### Dog

Code and CAD models for a small, cheap, 3D printed quadruped robot.

All Arduino based code runs on a Teensy 4.0 connected to a larger cpu. This can be a Raspberry Pi. In my case I am using a VOXL board from ModalAI.

![alt text](https://github.com/sschoedel/Dog/blob/Images/Three_Point_Blue.JPG)




## Compiling VOXL code:

1. Enter VOXL docker
'''
sudo docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
sudo docker run --net=host --privileged=true -v $(pwd)/:/opt/data/workspace/ -it voxl-compile-vins-fusion
'''

2. Compile with gcc
'''
gcc main.c -o dog.c
'''
