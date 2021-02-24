Small dog cad and code



Compiling VOXL code:

1. Enter VOXL docker
sudo docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
sudo docker run --net=host --privileged=true -v $(pwd)/:/opt/data/workspace/ -it voxl-compile-vins-fusion

2. Compile with gcc
gcc main.c -o dog.c
