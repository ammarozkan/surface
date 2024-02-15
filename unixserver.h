// UNIX file system server implementation for connection to other programs.
//  
//
//
//
// They need to send information about what they want to be in the screen, window etc.
//
// Maybe a authentication work is needed in procces id wise. Like, is this a program
// that executed "from" here?
//
// Struct definitions is needed for writing.



// Examples (may not be exist in the final application)
struct ProgramFirstRequest {
	uint32_t windowsize_x, windowsize_y;
};

struct ProgramRequestBase {
	uint32_t requestType;
};
