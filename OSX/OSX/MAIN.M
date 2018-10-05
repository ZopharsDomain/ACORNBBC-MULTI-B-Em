#include "b-em.h"
#import <Cocoa/Cocoa.h>

int main(int argc, char *argv[])
{
	allegro_init();
	[NSBundle loadNibNamed: @"MainMenu" owner: NSApp];
	bem_main(argc, argv);
	return 0;
}
END_OF_MAIN()
