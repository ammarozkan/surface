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



int
keyboard_eventread(int fd)
{
	struct input_event ev;
	unsigned int size;

	size = read(fd, &ev, sizeof(struct input_event));
	if (size < sizeof(struct input_event)) {
		perror("\nerror event reading");
		return -1;	
	}


}
