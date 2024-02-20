
	
	
	





// This experimental function, gets last "NULL" in the window
// list and then sets it to a new window. ram and pointer work
// is great.
void
surfaceAddWindow(struct Surface* surface,
			uint32_t sx, uint32_t sy, 
	     		uint32_t ex, uint32_t ey)
{
	struct Window** lastnull;
	for(lastnull = &surface->wins ; 
			*lastnull!=NULL ; 
			lastnull=&(*lastnull)->next);
					/// OHOHOHHHHMMMMMMM I loved that.
	*lastnull = createWindow(sx,sy,ex,ey);
}
