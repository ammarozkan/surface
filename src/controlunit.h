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

struct ControlUserFiles {
	int fdKeyboard, fdTouchscreen, fdTouchpad, fdMouse;
	uint8_t specialKeys;
};
struct ControlUnit {
	uint32_t usercount;
	struct ControlUserFiles* users;

	void* data;
	void (*SuperSpace)(uint32_t userid, void* data);
	void (*TouchscreenPositionX)(uint32_t userid, unsigned int value, void* data);
	void (*TouchscreenPositionY)(uint32_t userid, unsigned int value, void* data);
	void (*TouchscreenClick)(uint32_t userid, void* data, unsigned short code, 
			unsigned int value);
	void (*TouchpadPositionX)(uint32_t userid, unsigned int value, void* data);
	void (*TouchpadPositionY)(uint32_t userid, unsigned int value, void* data);

	void (*MouseMoveX)(uint32_t userid, int value, void* data);
	void (*MouseMoveY)(uint32_t userid, int value, void* data);
	void (*MouseKey)(uint32_t userid, void* data, unsigned short code, unsigned int value);
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

struct ControlUserFiles
getControlUser(char* keyboardpath, char* touchscreenpath,
		char* touchpadpath, char* mousepath)
{
	struct ControlUserFiles cuf;
	cuf.fdKeyboard; cuf.fdTouchscreen; cuf.fdTouchpad; cuf.fdMouse;
	cuf.fdKeyboard = cuOpenNonBlock(keyboardpath);
	cuf.fdTouchscreen = cuOpenNonBlock(touchscreenpath);
	cuf.fdTouchpad = cuOpenNonBlock(touchpadpath);
	cuf.fdMouse = cuOpenNonBlock(mousepath);
	return cuf;
}

struct ControlUnit
cuCreateControlUnit(char* keyboardpath,char* touchscreenpath,
		char* touchpadpath, char* mousepath)
{
	struct ControlUnit cu;
	cu.users = malloc(sizeof(struct ControlUserFiles));
	cu.usercount = 1;
	cu.users[0] = getControlUser(keyboardpath, touchscreenpath, touchpadpath, mousepath);
	return cu;
}

void
handleSpecialKey(uint32_t userid, uint8_t cu_key, unsigned int value,
		struct ControlUnit* cu)
{
	if(value == 1) 
		cu->users[userid].specialKeys = cu->users[userid].specialKeys | cu_key;
	else if(value == 0) 
		cu->users[userid].specialKeys = cu->users[userid].specialKeys & ~cu_key;
}

void
handleKey(uint32_t userid, unsigned short code, unsigned int value,
		struct ControlUnit* cu)
{
	if(cu->users[userid].specialKeys & CU_SUPERKEY && value == 1)
	{
		switch(code)
		{
		case KEY_SPACE:
			cu->SuperSpace(userid, cu->data);
			break;
		}
	}
}

int
cuKeyboardEventread(int userid, struct ControlUnit* cu)
{
	struct input_event ev;
	unsigned int size;
	errno = 0;
	size = read(cu->users[userid].fdKeyboard, &ev, sizeof(struct input_event));
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
			handleSpecialKey(userid, CU_SUPERKEY, ev.value, cu);
			break;
		default: 
			handleKey(userid, ev.code, ev.value, cu);
			break;
		}
	}
	return 1;
}

int cuTouchscreenEventread(int userid, struct ControlUnit* cu)
{
	struct input_event ev;
	errno = 0;
	unsigned int size = 0;
	
	size = read(cu->users[userid].fdTouchscreen, &ev, sizeof(struct input_event));
	if(errno == EAGAIN) {
		return 0;
	} else if(size < sizeof(struct input_event)) {
		perror("\nerror touchpad event reading");
		return -1;
	}
	if(ev.type == EV_ABS) {
		if(ev.code == ABS_X) 
			cu->TouchscreenPositionX(userid, ev.value, cu->data);
		else if(ev.code == ABS_Y) 
			cu->TouchscreenPositionY(userid, ev.value, cu->data);
	} else if(ev.type == EV_KEY) {
		cu->TouchscreenClick(userid, cu->data,ev.code,ev.value);
	}
}

int
cuTouchpadEventread(int userid, struct ControlUnit* cu)
{
	struct input_event ev;
	errno = 0;
	unsigned int size = 0;
	size = read(cu->users[userid].fdTouchpad, &ev, sizeof(struct input_event));
	if(errno == EAGAIN) {
		return 0;
	} else if(size < sizeof(struct input_event)) {
		perror("\nerror touchpad event reading");
		return -1;
	}
	if(ev.type == EV_ABS) {
		if(ev.code == ABS_MT_POSITION_X)
			cu->TouchpadPositionX(userid, ev.value, cu->data);
		else if(ev.code == ABS_MT_POSITION_Y)
			cu->TouchpadPositionY(userid, ev.value, cu->data);
	}
}

int
cuMouseEventread(int userid, struct ControlUnit* cu)
{
	struct input_event ev;
	errno = 0;
	unsigned int size = 0;

	size = read(cu->users[userid].fdMouse, &ev, sizeof(struct input_event));
	if(errno == EAGAIN) {
		return 0;
	} else if(size < sizeof(struct input_event)) {
		perror("\nerror mouse event reading");
		return -1;
	}
	if(ev.type == EV_REL) {
		if(ev.code == REL_X)
			cu->MouseMoveX(userid, ev.value, cu->data);
		else if(ev.code == REL_Y)
			cu->MouseMoveY(userid, ev.value, cu->data);
	} else if(ev.type == EV_KEY) {
		cu->MouseKey(userid, cu->data, ev.code, ev.value);
	}
	return 0;
}

int
cuUserRead(struct ControlUnit* cu, int userid)
{
	int ret = 0;
	struct ControlUserFiles cuf = cu->users[userid];


keyboardeventreadloopdo:
	if(cuf.fdKeyboard == -1);
	else if( (ret = cuKeyboardEventread(userid, cu)) == 1) goto keyboardeventreadloopdo;
	else if(ret == -1) return -1;

touchscreeneventreadloopdo:
	if(cuf.fdTouchscreen == -1);
	else if( (ret = cuTouchscreenEventread(userid,cu)) == 1 ) goto touchscreeneventreadloopdo;
	else if(ret == -1) return -1;

mouseeventreadloopdo:
	if(cuf.fdMouse == -1);
	else if( (ret = cuMouseEventread(userid,cu)) == 1 ) goto mouseeventreadloopdo;
	else if(ret == -1) return -1;

#ifndef CU_IGNORE_TOUCHPAD
touchpadeventreadloopdo:
	if (cuf.fdTouchpad == -1);
	else if( (ret = cuTouchpadEventread(userid,cu)) == 1 ) goto touchpadeventreadloopdo;
	else if(ret == -1) return -1;
#endif

	return 0;
}

int 
cuEventRead(struct ControlUnit* cu)
{
	for (unsigned int i = 0 ; i < cu->usercount ; i += 1) {
		if (cuUserRead(cu, i) == -1) return -1;
	}
	return 0;
}
