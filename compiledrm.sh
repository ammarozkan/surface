rm /surfacedesktop/regulardesktop-0
rm drme.out
gcc src/drme.c -I /usr/include/libdrm -I /usr/local/include/freetype2 -o drme.out -lm -ldrm -lgbm -lEGL -lGLESv2 -lfreetype
./drme.out --setgpu /dev/dri/card1 --setkeyboard /dev/input/event1 --settouchscreen /dev/input/event2
