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

//#define CU_IGNORE_TOUCHPAD

struct MouseInterface; // this interface will be used
		       // when multimouse support is came
		       // out. (multicursor)

struct ControlUnit {
	int fdKeyboard, fdTouchscreen, fdTouchpad, fdMouse;

	uint8_t specialKeys;
	void* data;
	void (*SuperSpace)(void* data);
	void (*TouchscreenPositionX)(unsigned int value, void* data);
	void (*TouchscreenPositionY)(unsigned int value, void* data);
	void (*TouchscreenClick)(void* data, unsigned short code, 
			unsigned int value);
	void (*TouchpadPositionX)(unsigned int value, void* data);
	void (*TouchpadPositionY)(unsigned int value, void* data);

	void (*MouseMoveX)(int value, void* data);
	void (*MouseMoveY)(int value, void* data);
	void (*MouseKey)(void* data, unsigned short code, unsigned int value);
};

int
cuOpenNonBlock(char* filename)
{
	if (filename == NULL) return -1;
	int result = open(filename, O_RDONLY);
	if (result <= 0) return result;
	int flags = fcntl(result,F_GETFL,0);
	fcntl(result,F_SETFL,flags|O_NONBLOCK);
	return result;
}

struct ControlUnit
cuCreateControlUnit(char* keyboardpath,char* touchscreenpath,
		char* touchpadpath, char* mousepath)
{
	struct ControlUnit cu;
	cu.fdKeyboard = cuOpenNonBlock(keyboardpath);
	cu.fdTouchscreen = cuOpenNonBlock(touchscreenpath);
	cu.fdTouchpad = cuOpenNonBlock(touchpadpath);
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
	unsigned int size = 0;
	
	size = read(fd, &ev, sizeof(struct input_event));
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

int
cuTouchpadEventread(int fd, struct ControlUnit* cu)
{
	struct input_event ev;
	errno = 0;
	unsigned int size = 0;

	size = read(fd, &ev, sizeof(struct input_event));
	if(errno == EAGAIN) {
		return 0;
	} else if(size < sizeof(struct input_event)) {
		perror("\nerror touchpad event reading");
		return -1;
	}
	if(ev.type == EV_ABS) {
		if(ev.code == ABS_MT_POSITION_X)
			cu->TouchpadPositionX(ev.value, cu->data);
		else if(ev.code == ABS_MT_POSITION_Y)
			cu->TouchpadPositionY(ev.value, cu->data);
	}
}

int
cuMouseEventread(int fd, struct ControlUnit* cu)
{
	struct input_event ev;
	errno = 0;
	unsigned int size = 0;

	size = read(fd, &ev, sizeof(struct input_event));
	if(errno == EAGAIN) {
		return 0;
	} else if(size < sizeof(struct input_event)) {
		perror("\nerror mouse event reading");
		return -1;
	}
	if(ev.type == EV_REL) {
		if(ev.code == REL_X)
			cu->MouseMoveX(ev.value, cu->data);
		else if(ev.code == REL_Y)
			cu->MouseMoveY(ev.value, cu->data);
	} else if(ev.type == EV_KEY) {
		cu->MouseKey(cu->data, ev.code, ev.value);
	}
}

int 
cuEventRead(struct ControlUnit* cu)
{
	int ret = 0;
keyboardeventreadloopdo:
	//cuKeyboardEventread(cu->fdKeyboard,cu);
	if (cu->fdKeyboard);
	else if( (ret = cuKeyboardEventread(cu->fdKeyboard,cu)) == 1 ) goto keyboardeventreadloopdo;
	else if(ret == -1) return -1;

touchscreeneventreadloopdo:
	//cuTouchscreenEventread(cu->fdMouse,cu);
	if(cu->fdTouchscreen == -1);
	else if( (ret = cuTouchscreenEventread(cu->fdTouchscreen,cu)) == 1 ) goto touchscreeneventreadloopdo;
	else if(ret == -1) return -1;

mouseeventreadloopdo:
	if(cu->fdMouse == -1);
	else if( (ret == cuMouseEventread(cu->fdMouse,cu)) == 1 ) goto mouseeventreadloopdo;
	else if(ret == -1) return -1;

#ifndef CU_IGNORE_TOUCHPAD
touchpadeventreadloopdo:
	if (cu->fdTouchpad == -1);
	else if( (ret = cuTouchpadEventread(cu->fdTouchpad,cu)) == 1 ) goto touchpadeventreadloopdo;
	else if(ret == -1) return -1;
#endif

	return 0;
}
