rm /surfacedesktop/regulardesktop-0
rm drme.out
gcc src/drme.c -I /usr/include/libdrm -o drme.out -ldrm -lgbm -lEGL -lGLESv2
./drme.out --setgpu /dev/dri/card1 --setkeyboard /dev/input/event1 --settouchscreen /dev/input/event2
