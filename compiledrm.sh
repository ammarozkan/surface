rm drme.out
gcc drme.c -I /usr/include/libdrm -o drme.out -ldrm
./drme.out --setgpu /dev/dri/card1
