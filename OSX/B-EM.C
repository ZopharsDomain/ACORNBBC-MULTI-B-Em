/*B-em 1.0 by Tom Walker*/
/*Main loop*/

#include <allegro.h>
#include "b-em.h"
#include <string.h>

int quit=0;

static int scupdate=0;
static void update50()
{
        scupdate++;
}
END_OF_FUNCTION(update50);

char snapname[260] = "";
extern int autoboot;
int wah;
int loadcmdlinefile(char *t)
{
	/* determine filename */
	char namestring[260];
	int bonus = 0;
	if (t[0]=='\"') /*Quoted filename*/
	{
		int c = 1;
		while(1)
		{
			if(t[c] == '\"') break;
			namestring[c-1] = t[c];
			c++;
		}
		namestring[c-1] = '\0';
		bonus = 2;
	}
	else
	{
		int c = 0;
		while(1)
		{
			if(t[c] == ' ') break;
			namestring[c] = t[c];
			c++;
		}
		namestring[c] = '\0';
	}

	/* decide whether this is a uef */
	char *e = namestring + strlen(namestring);
	while(*e != '.' && e > namestring) e--;
	if(*e != '.') return strlen(namestring);

	if(tolower(e[1]) == 'u')
	{
		strcpy(uefname, namestring);
	}
	else
		if(tolower(e[2]) == 'n')
		{
			strcpy(snapname, namestring);
		}
		else
		{
			strcpy(discname[0], namestring);
			autoboot=firstautoboot;
		}

	return strlen(namestring)+bonus;
}

void parsecommandline(char *s)
{
	char *pt, *t;
	char tb[260];
	int found;
	
	pt = s;
	while(strlen(pt) > 1)
	{
		if(pt[0] == '-')
		{
			/* copy and start strtok'ing */
			strcpy(tb, pt);
			t = strtok(tb, " ");
			if(!stricmp(t,"-disc") || !stricmp(t,"-tape") || !stricmp(t,"-snap"))
			{
				found=0;
				t=strtok(0," ");
				if (!t) return;
				loadcmdlinefile(t);
			}
			if (!stricmp(t,"-autoboot"))
				autoboot=50;
			if (!stricmp(t,"-esc"))
				wah=1;
			if (!stricmp(t,"-model"))
			{
				t=strtok(0," ");
				if (t)
				{
					if (!stricmp(t,"a"))     model=0;
					if (!stricmp(t,"b"))     model=1;
					if (!stricmp(t,"bsw"))   model=2;
					if (!stricmp(t,"ntscb")) model=3;
					if (!stricmp(t,"b+"))    model=4;
					if (!stricmp(t,"b+96"))  model=5;
					if (!stricmp(t,"b+128")) model=6;
					if (!stricmp(t,"m128"))  model=7;
				}
				else
					return;
			}
			pt += t - tb + 1;
		}
		else
		{
			pt += loadcmdlinefile(pt) + 1;
		}
	}
}

void quitfunc()
{
	quit = TRUE;
}
END_OF_FUNCTION(quitfunc);

#define ISLEN	8192

int bem_main(int argc, char *argv[])
{
		short *p;
        char s[256];
        int c;
        int resetting=0;
		short ISBuffer[ISLEN];
		int ISBWritePtr = 0, ISBReadPtr = 0, EmptyBuf;
		memset(ISBuffer, 0, ISLEN << 1);

        printf("B-em v1.0\n");
        load_config();
        if (argc>1)
        {
				s[0]=0;
				for (c=1;c<argc;c++)
				{
						/* check if we have an argument with spaces that isn't encased in quotes */
						if(strcspn(argv[c], " ") != strlen(argv[c]))
						{
							strcat(s,"\"");
							strcat(s,argv[c]);
							strcat(s,"\"");
						}
						else
							strcat(s,argv[c]);
						strcat(s," ");
				}
				parsecommandline(s);
		}
        loadcmos();
        install_keyboard();
        key_led_flag=0;
        install_timer();
        install_mouse();
        initmem();
        loadroms();
        reset6502();
        resetsysvia();
        resetuservia();
        resetcrtc();
        resetacia();
        initserial();
        initsnd();
        initvideo();
        initadc();
        reset8271(1);
        reset1770();
        loadarmrom();
        resetarm();
        resettube();
//        tubeinit6502();
//        tubeinitz80();
        loaddiscsamps();
        openuef(uefname);
        if (!uefena && model<3) trapos();
//        printf("Inited\n");
        install_int_ex(update50,MSEC_TO_TIMER(5));
		
		LOCK_FUNCTION(quitfunc);
		LOCK_VARIABLE(quit);
		set_close_button_callback(quitfunc);
		
#ifdef __APPLE__
		setup_gui();
		enable_hardware_cursor();
		select_mouse_cursor(MOUSE_CURSOR_ARROW);
		show_mouse(screen);
#endif

		if(snapname[0])
		{
			loadstate(snapname);
			snapname[0] = 0;
		}
        while (!quit)
        {
                exec6502(312,128);
                if (logging) logsound();
//                drawscr();
                checkkeys();
                poll_joystick();
				if (!motor || !fasttape)
				{
                        if (soundon)
                        {
							if(ISBWritePtr < ISLEN-624)
								updatebuffer(&ISBuffer[ISBWritePtr],624);
							else
							{
								short TSBuffer[624];
								updatebuffer(TSBuffer,624);
								memcpy(&ISBuffer[ISBWritePtr], TSBuffer, (ISLEN-ISBWritePtr) << 1);
								memcpy(ISBuffer, &TSBuffer[ISLEN-ISBWritePtr], (624-ISLEN+ISBWritePtr) << 1);
							}
							ISBWritePtr = (ISBWritePtr+624)&(ISLEN-1);

							EmptyBuf = ISBWritePtr - ISBReadPtr;
							if(EmptyBuf < 0) EmptyBuf += ISLEN;
							if((EmptyBuf > 1024+624) && (p=(short *)get_audio_stream_buffer(as)))
							{
								while(EmptyBuf >= 2672) /* 2048+624 */
								{
									ISBReadPtr = (ISBReadPtr+1024)&(ISLEN-1);
									EmptyBuf -= 1024;
								}

								memcpy(p, &ISBuffer[ISBReadPtr], 2048);
								ISBReadPtr = (ISBReadPtr+1024)&(ISLEN-1);
								free_audio_stream_buffer(as);
							}
						}
						if(scupdate < 4)
						{
							rest((4-scupdate)*5);
							scupdate -= 4;
						}
						else
							scupdate=0;
				}
                if (resetting && !key[KEY_F12]) resetting=0;
                if (key[KEY_F12] && !resetting)
                {
                        if (key[KEY_LCONTROL] || key[KEY_RCONTROL])
                        {
                                resetsysvia();
                                memset(ram,0,65536);
                        }
                        resetarm();
                        resetuservia();
//                        resetacia();
//                        initserial();
                        reset1770s();
                        reset8271s();
                        if (model<1) remaketablesa();
                        else         remaketables();
                        reset6502();
//                        printf("Reset\n");
                        resetting=1;
//                        while (key[KEY_F12])
//                        {
//                                sleep(1);
//                        }
                }
				
#ifndef __APPLE__
                if (key[KEY_F11]|key[KEY_BACKQUOTE])
                {
                        while (key[KEY_F11]|key[KEY_BACKQUOTE]) yield_timeslice();
                        entergui();
                }
                if (mouse_b&2)
                {
                        while (mouse_b&2) yield_timeslice();
                        entergui();
                }
                if (wah && key[KEY_ESC]) quit=1;
#else
				process_gui();
#endif
                if (autoboot)
                {
                        autoboot--;
                        if (!autoboot)
                           releasekey(0,0);
                }
        }

#ifdef __APPLE__
		shutdown_gui();
#endif
//        output=1;
//        exec6502(200,128);
//        dumpram();
        save_config();
        savecmos();
//        dumparmregs();
//        savebuffers();
//        dumpregs();
        checkdiscchanged(0);
        checkdiscchanged(1);
/*        dumptuberegs();
        dumptube();
        dumpram2();
        dumpuservia();*/
//        dumparmregs();
//        dumpsysvia();
//        printf("%i\n",vidbank);
//        dumpcrtc();
        return 0;
}

#ifndef __APPLE__
/* on OS X, a different main is used that first instantiates the OS X GUI */
int main(int argc, char *argv[])
{
	allegro_init();
	b-em_main(argc, argv);
}
END_OF_MAIN()
#endif
