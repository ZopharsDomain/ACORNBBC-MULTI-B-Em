#import "MenuModel.h"

/* mutex stuff */
#include <pthread.h>
#include "b-em.h"
pthread_mutex_t mutex;

#define AdvancePtr(v) 	v = (v+1)&31
#define preferences [NSUserDefaults standardUserDefaults]
MenuModel *MyMenu;

int InstructionWritePtr, InstructionReadPtr;
struct GUIEvent
{
	enum ET
	{
		ET_LOADDISC1, ET_LOADDISC2, ET_LOADTAPE, ET_LOADSTATE, ET_SAVESTATE,

		ET_SETFASTTAPE, ET_SETSOUNDENABLE, ET_SETLOWPASSFILTER, ET_SETHIGHPASSFILTER,
		ET_SETDISCNOISES, ET_SETBLURFILTER, ET_SETMONOCHROME, ET_SETAUTOBOOT,

		ET_SELECTMODEL, ET_SELECTOUTPUT, ET_SELECTWAVEFORM,

		ET_BREAK, ET_QUIT
	} Type;
	union
	{
		char *Filename;
		BOOL NewState;
		int Index;
	} Data;
} EventQueue[32];

/* functions for telling B-Em to do stuff, mostly derived from gui.c */
void loaddisc(char *tempname, int drive)
{
	int c;

	checkdiscchanged(drive);
	strcpy(discname[drive],tempname);
	c = strlen(discname[drive]);
	while(c)
	{
		if(discname[drive][c]=='.')
		{
			c++;
			break;
		}
		c--;
	}
	if ((discname[drive][c]=='d'||discname[drive][c]=='D')&&(c!=strlen(discname[drive])))
		load8271dsd(discname[drive],drive);
	else if ((discname[drive][c]=='a'||discname[drive][c]=='A')&&(c!=strlen(discname[drive])))
		load1770adfs(discname[drive],drive);
	else if ((discname[drive][c]=='f'||discname[drive][c]=='F')&&(c!=strlen(discname[drive])))
		load8271fdi(discname[drive],drive);
	else if (c!=strlen(discname[drive]))
		load8271ssd(discname[drive],drive);
}

/* external callable */
void process_gui()
{
	pthread_mutex_lock(&mutex);
	while(InstructionReadPtr != InstructionWritePtr)
	{
		switch(EventQueue[InstructionReadPtr].Type)
		{
			case ET_LOADDISC2:
			case ET_LOADDISC1:
				loaddisc(EventQueue[InstructionReadPtr].Data.Filename, (EventQueue[InstructionReadPtr].Type == ET_LOADDISC2) ? 1 : 0);
				free(EventQueue[InstructionReadPtr].Data.Filename);
			break;
			case ET_LOADTAPE:
                strcpy(uefname,EventQueue[InstructionReadPtr].Data.Filename);
                openuef(uefname);
				free(EventQueue[InstructionReadPtr].Data.Filename);
			break;
			case ET_LOADSTATE:
                loadstate(EventQueue[InstructionReadPtr].Data.Filename);
				free(EventQueue[InstructionReadPtr].Data.Filename);
			break;
			case ET_SAVESTATE:
                savestate(EventQueue[InstructionReadPtr].Data.Filename);
				free(EventQueue[InstructionReadPtr].Data.Filename);
			break;
			case ET_SETBLURFILTER:		blurred = EventQueue[InstructionReadPtr].Data.NewState ? 1 : 0;							break;
			case ET_SETDISCNOISES:		ddnoise = EventQueue[InstructionReadPtr].Data.NewState ? 1 : 0;							break;
			case ET_SETFASTTAPE:		fasttape = EventQueue[InstructionReadPtr].Data.NewState ? 1 : 0;						break;
			case ET_SETHIGHPASSFILTER:	soundfilter = (soundfilter&1) | EventQueue[InstructionReadPtr].Data.NewState ? 2 : 0;	break;
			case ET_SETLOWPASSFILTER:	soundfilter = (soundfilter&2) | EventQueue[InstructionReadPtr].Data.NewState ? 1 : 0;	break;
			case ET_SETMONOCHROME:		mono = EventQueue[InstructionReadPtr].Data.NewState ? 1 : 0; updatepalette();			break;
			case ET_SETSOUNDENABLE:		soundon = EventQueue[InstructionReadPtr].Data.NewState ? 1 : 0;							break;
			case ET_SETAUTOBOOT:		firstautoboot = EventQueue[InstructionReadPtr].Data.NewState ? 50 : 0;					break;
			case ET_SELECTMODEL:
				if(model != EventQueue[InstructionReadPtr].Data.Index)
				{
					model = EventQueue[InstructionReadPtr].Data.Index;
					if(model == 9) model = 7;
					tube = (EventQueue[InstructionReadPtr].Data.Index == 9) ? 1 : 0;

					model ? remaketables() : remaketablesa();
					loadroms();
					reset6502();
					reset8271(0);
					reset1770();
					resetsysvia();
					resetuservia();
					memset(ram,0, (model >= 4) ? 65536 : 32768);
					if(tube) resetarm();
				}
			break;

			case ET_SELECTOUTPUT:
				switch(EventQueue[InstructionReadPtr].Data.Index)
				{
					case 0: hires = 0; break;
					case 1: hires = 3; break;
					case 2: hires = 1; break;
					case 3: hires = 2; break;
				}
				updategfxmode();
				[MyMenu showPreferences:nil];
				enable_hardware_cursor();
				select_mouse_cursor(MOUSE_CURSOR_ARROW);
				show_mouse(screen);
			break;

			case ET_SELECTWAVEFORM:
				curwave = EventQueue[InstructionReadPtr].Data.Index;
			break;

			case ET_BREAK:
				resetarm();
				resetuservia();
				reset1770s();
				reset8271s();
				if (model<1) remaketablesa();
				else         remaketables();
				reset6502();
			break;

			case ET_QUIT:
				quit = 1;
			break;
		}
		AdvancePtr(InstructionReadPtr);
	}
	pthread_mutex_unlock(&mutex);
}

void setup_gui()
{
	[MyMenu setupFromGlobals];
	InstructionReadPtr = InstructionWritePtr = 0;
	pthread_mutex_init(&mutex, NULL);
}

void shutdown_gui()
{
	pthread_mutex_destroy(&mutex);
}

@implementation MenuModel

- (void)awakeFromNib
{
	MyMenu = self;
}

- (void)setupFromGlobals
{
	/* setup tick boxes */
	[blurFilterButton setState:(blurred ? NSOnState : NSOffState)];
	[discNoisesButton setState:(ddnoise ? NSOnState : NSOffState)];
	[fastTapeButton setState:(fasttape ? NSOnState : NSOffState)];
	[highPassFilterButton setState:(soundfilter&1 ? NSOnState : NSOffState)];
	[lowPassFilterButton setState:(soundfilter&2 ? NSOnState : NSOffState)];
	[monochromeButton setState:(mono ? NSOnState : NSOffState)];
	[soundEnableButton setState:(soundon ? NSOnState : NSOffState)];
	[autobootButton setState:(firstautoboot ? NSOnState : NSOffState)];

	if(soundon)
	{
		[highPassFilterButton setEnabled:YES];
		[lowPassFilterButton setEnabled:YES];
	}
	else
	{
		[highPassFilterButton setEnabled:NO];
		[lowPassFilterButton setEnabled:NO];
	}

	/* set model & output type */
	int index;
	switch(hires)
	{
		case 0: index = 0; break;
		case 1: index = 2; break;
		case 2: index = 3; break;
		case 3: index = 1; break;
	}
	[outputList selectCellAtRow:index column:0];
	[modelList selectCellAtRow:model column:0];

	/* set sound waveform */
	[waveformPopUpButton selectItemAtIndex:curwave];
}

- (void)dealloc
{
	[super dealloc];
}

#define	DoFileOpen(ev, pth, typ) \
    NSString *path = nil;\
    NSOpenPanel *openPanel = [ NSOpenPanel openPanel ];\
    NSString * dirPath = [ preferences objectForKey:@ pth];\
    if ( [ openPanel runModalForDirectory:dirPath file:@"SavedGame" types: typ ] ) \
	{\
        path = [ [ openPanel filenames ] objectAtIndex:0 ];\
		[preferences setObject:[openPanel directory] forKey:@ pth ];\
    }\
	else\
		return;\
	[preferences synchronize ]; \
\
	pthread_mutex_lock(&mutex);\
	EventQueue[InstructionWritePtr].Type = ev;\
	EventQueue[InstructionWritePtr].Data.Filename = strdup( [path cString] );\
	AdvancePtr(InstructionWritePtr);\
	pthread_mutex_unlock(&mutex);

- (IBAction)loaddisc1:(id)sender
{
	NSArray *discTypes = [NSArray arrayWithObjects: @"ssd", @"dsd", @"img", @"adf", @"adl", @"fdi", nil];
	DoFileOpen(ET_LOADDISC1, "LastOpenDiscPath", discTypes);
}

- (IBAction)loaddisc2:(id)sender
{
	NSArray *discTypes = [NSArray arrayWithObjects: @"ssd", @"dsd", @"img", @"adf", @"adl", @"fdi", nil];
	DoFileOpen(ET_LOADDISC2, "LastOpenDiscPath", discTypes);
}

- (IBAction)loadstate:(id)sender
{
	NSArray *stateTypes = [NSArray arrayWithObjects: @"snp", nil];
	DoFileOpen(ET_LOADSTATE, "LastOpenStatePath", stateTypes);
}

- (IBAction)loadtape:(id)sender
{
	NSArray *tapeTypes = [NSArray arrayWithObjects: @"uef", nil];
	DoFileOpen(ET_LOADTAPE, "LastOpenTapePath", tapeTypes);
}

- (IBAction)savestate:(id)sender
{
    NSString *path = nil;
    NSSavePanel *savePanel = [NSSavePanel savePanel];
	[savePanel setRequiredFileType:@"snp"];
    NSString * dirPath = [preferences objectForKey:@"LastOpenStatePath"];
    if ( [savePanel runModalForDirectory:dirPath file:@"SavedState"] )
	{
        path = [savePanel filename];
		[preferences setObject:[savePanel directory] forKey:@"LastOpenStatePath"];
    }
	else
		return;

	[preferences synchronize ];

	pthread_mutex_lock(&mutex);
	EventQueue[InstructionWritePtr].Type = ET_SAVESTATE;
	EventQueue[InstructionWritePtr].Data.Filename = strdup( [path cString] );
	AdvancePtr(InstructionWritePtr);
	pthread_mutex_unlock(&mutex);
}

#define GetButtonState(ev, button)\
	pthread_mutex_lock(&mutex);\
	EventQueue[InstructionWritePtr].Type = ev;\
	EventQueue[InstructionWritePtr].Data.NewState = ([button state] == NSOnState);\
	AdvancePtr(InstructionWritePtr);\
	pthread_mutex_unlock(&mutex);

- (IBAction)setBlurFilter:(id)sender
{
	GetButtonState(ET_SETBLURFILTER, blurFilterButton);
}

- (IBAction)setDiscNoises:(id)sender
{
	GetButtonState(ET_SETDISCNOISES, discNoisesButton);
}

- (IBAction)setFastTape:(id)sender
{
	GetButtonState(ET_SETFASTTAPE, fastTapeButton);
}

- (IBAction)setHighPassFilter:(id)sender
{
	GetButtonState(ET_SETHIGHPASSFILTER, highPassFilterButton);
}

- (IBAction)setLowPassFilter:(id)sender
{
	GetButtonState(ET_SETLOWPASSFILTER, lowPassFilterButton);
}

- (IBAction)setSoundEnable:(id)sender
{
	GetButtonState(ET_SETSOUNDENABLE, soundEnableButton);
	if([soundEnableButton state] == NSOnState)
	{
		[highPassFilterButton setEnabled:YES];
		[lowPassFilterButton setEnabled:YES];
	}
	else
	{
		[highPassFilterButton setEnabled:NO];
		[lowPassFilterButton setEnabled:NO];
	}
}

- (IBAction)setMonochrome:(id)sender
{
	GetButtonState(ET_SETMONOCHROME, monochromeButton);
}

- (IBAction)setAutoboot:(id)sender
{
	GetButtonState(ET_SETAUTOBOOT, autobootButton);
}

#define GetRadioState(ev, list)\
	pthread_mutex_lock(&mutex);\
	EventQueue[InstructionWritePtr].Type = ev;\
	EventQueue[InstructionWritePtr].Data.Index = [list selectedRow];\
	AdvancePtr(InstructionWritePtr);\
	pthread_mutex_unlock(&mutex);

- (IBAction)setModel:(id)sender
{
	GetRadioState(ET_SELECTMODEL, modelList);
}

- (IBAction)setOutput:(id)sender
{
	GetRadioState(ET_SELECTOUTPUT, outputList);
}

- (IBAction)setWaveform:(id)sender
{
	pthread_mutex_lock(&mutex);
	EventQueue[InstructionWritePtr].Type = ET_SELECTWAVEFORM;
	EventQueue[InstructionWritePtr].Data.Index = [waveformPopUpButton indexOfSelectedItem];
	AdvancePtr(InstructionWritePtr);
	pthread_mutex_unlock(&mutex);
}

- (IBAction)showPreferences:(id)sender
{
	[preferencesWindow makeKeyAndOrderFront:self];
}

- (IBAction)resetMachine:(id)sender
{
	pthread_mutex_lock(&mutex);
	EventQueue[InstructionWritePtr].Type = ET_BREAK;
	AdvancePtr(InstructionWritePtr);
	pthread_mutex_unlock(&mutex);
}

- (IBAction)terminate:(id)sender
{
	pthread_mutex_lock(&mutex);
	EventQueue[InstructionWritePtr].Type = ET_QUIT;
	AdvancePtr(InstructionWritePtr);
	pthread_mutex_unlock(&mutex);
}

// Internal function used to launch a file from the app bundle's resources folder. By Ewan Roberts!
-(void) openResourceFile:( NSString *) fileName
{
	NSString *myBundle = [[NSBundle mainBundle] bundlePath];
	NSString *fullpath = [NSString stringWithFormat:@"%@/Contents/Resources/%@",myBundle, fileName];
	[[NSWorkspace sharedWorkspace] openFile:fullpath ];
}

- (IBAction)showHelp:(id)sender
{
	[self openResourceFile:@"readme.txt"];
}

- (IBAction)showHistory:(id)sender
{
	[self openResourceFile:@"history.txt"];
}

@end
