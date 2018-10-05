/*B-em 1.1 by Tom Walker*/
/*Main loop*/

#include <allegro.h>
#include "b-em.h"

int quit=0;

static int scupdate=0;
static void update50()
{
        scupdate=1;
}
END_OF_FUNCTION(update50);

int autoboot,wah;
void parsecommandline(char *s)
{
        char *t;
        int c;
        char fn[256],s2[256];
        int found;
        for (t=strtok(s," ");t;t=strtok(0," "))
        {
                if (!stricmp(t,"-disc"))
                {
                        found=0;
                        t=strtok(0," ");
//                        printf("%i\n",t);
                        if (!t) return;
                        if (t[0]=='\"') /*Quoted filename*/
                        {
                                strcpy(fn,t+1);
                                while (1)
                                {
//                                        printf("filename : %s\n",fn);
                                        for (c=0;c<strlen(fn);c++)
                                        {
                                                if (fn[c]=='\"')
                                                {
                                                        found=1;
                                                        break;
                                                }
                                        }
                                        if (found) break;
                                        t=strtok(0," ");
                                        if (!t) break;
                                        strcat(fn," ");
                                        strcat(fn,t);
                                }
                                if (!found) return;
                                fn[c]=0;
                                strcpy(discname[0],fn);
                        }
                        else
                        {
                                strcpy(discname[0],t);
                        }
/*                        t=strtok(0," ");
                        if (t)
                        {
                                if (t[0]=='"')
                                strcpy(discname[0],t);
                        }
                        else
                           return;*/
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
        }
}

int main(int argc, char *argv[])
{
        unsigned short *p;
        char s[256];
        int c;
        int resetting=0;
        printf("B-em v1.1\n");
        allegro_init();
//        atexit(dumpram2);
        load_config();
        if (argc>1)
        {
                s[0]=0;
                for (c=1;c<argc;c++)
                {
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
        install_int_ex(update50,MSEC_TO_TIMER(20));
        while (!quit)
        {
                exec6502(312,128);
                if (logging) logsound();
//                drawscr();
                checkkeys();
                poll_joystick();
                if (!motor || !fasttape)
                {
//                        printf("Waiting on sound\n");
                        if (soundon)
                        {
                                p=0;
//                                printf("Entering sound wait\n");
                                while (!p)
                                {
                                        p=(unsigned short *)get_audio_stream_buffer(as);
//                                        printf("Polling\n");
                                }
//                                printf("Finished waiting\n");
                                updatebuffer(p,624);
                                free_audio_stream_buffer(as);
                        }
                        else
                        {
                                while (!scupdate)
                                {
                                        yield_timeslice();
                                        p++;
                                }
                                scupdate=0;
                        }
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
                if (key[KEY_F11])
                {
                        while (key[KEY_F11]) yield_timeslice();
                        entergui();
                }
                if (mouse_b&2)
                {
                        while (mouse_b&2) yield_timeslice();
                        entergui();
                }
                if (wah && key[KEY_ESC]) quit=1;
                if (autoboot)
                {
                        autoboot--;
                        if (!autoboot)
                           releasekey(0,0);
                }
        }
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
