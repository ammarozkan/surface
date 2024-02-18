// Targets:
//
// The button that named "Super" or "Command" or "Windows"
// will be used as root button. When user touches it, 
// nothing can understand what user is doing while 
// touching it. With so, user can do whetever user wanted
// in the desktop. Proccesses shouldn't slow down user
// if this button is pressing. Press on this button,
// will prevent sending any other action to proccesses.
// 
// For example, in a case that a fullscreen app frozen,
// user should be able to Super + S and Super + MouseClick
// to panic button and see whats happening.
//
// Super + F -> fullscreen in a virtual desktop
// Super + W -> fullscreen
// Super + A -> put it left
// Super + D -> put it right
// Super + S -> break putting if putted to somewhere.
// Super + Q -> I_QUIT
// Super + Space -> Terminal
// Super + E -> put it left up?
// Super + C -> put it left down?
// Super + Z -> put it right down?
//

// 0b0000 -> 0b1111
//

#include <linux/input.h>
#include <fcntl.h> // ioctl, fcntl
#include <errno.h>


#define CU_FULLON 0b11111111
#define CU_SUPERKEY 0b10000000
#define CU_SUPERKEY_REVERSE 0b01111111

#define KEY_SUPER 125


struct ControlUnit {
	int fdKeyboard, fdMouse;

	uint8_t specialKeys;
	void* data;
	void (*SuperSpace)(void* data);
	void (*SuperK)(void* data);
	void (*TouchscreenPositionX)(unsigned int value, void* data);
	void (*TouchscreenPositionY)(unsigned int value, void* data);
	void (*TouchscreenClick)(void* data, unsigned short code, 
			unsigned int value);
};

int
cuOpenNonBlock(char* filename)
{
	int result = open(filename, O_RDONLY);
	if (result <= 0) return result;
	int flags = fcntl(result,F_GETFL,0);
	fcntl(result,F_SETFL,flags|O_NONBLOCK);
	return result;
}

struct ControlUnit
cuCreateControlUnit(char* keyboardpath,char* mousepath)
{
	struct ControlUnit cu;
	cu.fdKeyboard = cuOpenNonBlock(keyboardpath);
	cu.fdMouse = cuOpenNonBlock(mousepath);
	return cu;
}

void
handleSpecialKey(uint8_t cu_key, unsigned int value,
		struct ControlUnit* cu)
{
	if(value == 1) 
		cu->specialKeys = cu->specialKeys | cu_key;
	else if(value == 0) 
		cu->specialKeys = cu->specialKeys & ~cu_key;
}

void
handleKey(unsigned short code, unsigned int value,
		struct ControlUnit* cu)
{
	if(cu->specialKeys & CU_SUPERKEY && value == 1)
	{
		switch(code)
		{
		case KEY_SPACE:
			cu->SuperSpace(cu->data);
			break;
		case KEY_K:
			cu->SuperK(cu->data);
			break;
		}
	}
}

int
cuKeyboardEventread(int fd,struct ControlUnit* cu)
{
	struct input_event ev;
	unsigned int size;
	errno = 0;
	size = read(fd, &ev, sizeof(struct input_event));
	if (errno == EAGAIN) {
		return 0;
	} else if (size < sizeof(struct input_event)) {
		perror("\nerror event reading");
		return -1;
	}
	if(ev.type == EV_KEY)
	{
		switch(ev.code) {
		case KEY_SUPER:
			handleSpecialKey(CU_SUPERKEY, ev.value, cu);
			break;
		default: 
			handleKey(ev.code, ev.value, cu);
			break;
		}
	}
	return 1;
}

int cuTouchscreenEventread(int fd, struct ControlUnit* cu)
{
	struct input_event ev;
	errno = 0;
	unsigned int size = read(fd, &ev, sizeof(struct input_event));
	if(errno == EAGAIN) {
		return 0;
	} else if(size < sizeof(struct input_event)) {
		perror("\nerror touchpad event reading");
		return -1;
	}
	if(ev.type == EV_ABS) {
		if(ev.code == ABS_X) 
			cu->TouchscreenPositionX(ev.value, cu->data);
		else if(ev.code == ABS_Y) 
			cu->TouchscreenPositionY(ev.value, cu->data);
	} else if(ev.type == EV_KEY) {
		cu->TouchscreenClick(cu->data,ev.code,ev.value);
	}
}

int cuEventRead(struct ControlUnit* cu)
{
	int ret = 0;
keyboardeventreadloopdo:
	//cuKeyboardEventread(cu->fdKeyboard,cu);
	if( (ret = cuKeyboardEventread(cu->fdKeyboard,cu)) == 1 ) goto keyboardeventreadloopdo;
	else if(ret == -1) return -1;

touchpadeventreadloopdo:
	//cuTouchscreenEventread(cu->fdMouse,cu);
	if( (ret = cuTouchscreenEventread(cu->fdMouse,cu)) == 1 ) goto touchpadeventreadloopdo;
	else if(ret == -1) return -1;

	return 0;
}
