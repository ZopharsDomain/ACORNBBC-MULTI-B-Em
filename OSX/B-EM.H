#include <allegro.h>

/*6502*/
void reset6502();
void dumpregs();
void exec6502(int lines, int cpl);
extern int interrupt;
extern int nmi,oldnmi,nmilock;
extern unsigned char a,x,y,s;
extern unsigned short pc;
struct __ptype
{
        int c,z,i,d,v,n;
};
extern struct __ptype p;
extern unsigned char *ram;
extern unsigned char os[16384];
extern int output,timetolive;

/*VIAs*/
typedef struct VIA
{
        unsigned char ora,orb,ira,irb;
        unsigned char ddra,ddrb;
        unsigned long t1l,t2l;
        int t1c,t2c;
        unsigned char acr,pcr,ifr,ier;
        int t1hit,t2hit;
        unsigned char porta,portb;
} VIA;
extern VIA sysvia,uservia;

void resetsysvia();
unsigned char readsysvia(unsigned short addr);
//void writesysvia(unsigned short addr, unsigned char val,);
void updatesystimers();

/*Video*/
extern unsigned short vidbank;

/*Disc*/
#define SIDES 2
#define TRACKS 80
#define SECTORS 16
#define SECTORSIZE 256

extern unsigned char discs[2][SIDES][SECTORS*TRACKS][SECTORSIZE];
extern int ddensity,inreadop;
extern int idmarks;
extern int sectorsleft;
extern int cursec[2];
extern int curtrack[2],inttrack[2];
extern int fdiin[2];
extern int discaltered[2];
extern int sides[2];
extern int adfs[2];
extern int motorofff;
extern SAMPLE *seeksmp;
extern SAMPLE *seek2smp;
extern SAMPLE *seek3smp;
extern SAMPLE *stepsmp;
extern SAMPLE *motorsmp;
extern SAMPLE *motoroffsmp;
extern SAMPLE *motoronsmp;
extern int curside,curdisc;
extern int discint;
extern int sectorpos;
extern int readflash;
extern int driveled;

/*Config*/
extern int ddnoise;
extern int fasttape;
extern int hires;
extern int soundfilter;
extern char uefname[260];
extern int uefena;
extern int soundon;
extern int curwave;
extern int blurred,mono;
extern int ddnoise;
extern int model,tube;
extern char discname[2][260];
extern int quit;
extern int autoboot,firstautoboot;
extern int fasttape;

/*Sound*/
extern AUDIOSTREAM *as;
extern int logging;

/*Video*/
extern BITMAP *buffer;
extern unsigned char crtc[32];
extern int interlaceline;
extern unsigned short vidmask;

/*ADC*/
extern int adcconvert;

/*Tube*/
extern int tubeirq;
extern int tubecycs;

/*Tape*/
extern int tapelcount,tapellatch;
extern int chunklen;
extern int blocks,motor;
extern int chunkid,chunklen,intone;

/* various functions */
extern void vblankint();
extern unsigned char readcrtc(unsigned short addr);
extern void trapos();
extern unsigned char readadc(unsigned short addr);
extern unsigned char read1770(unsigned short addr);
extern void updateusertimers();
extern unsigned char readuservia(unsigned short addr);
extern unsigned char readtubehost(unsigned short addr);
extern void writecrtc(unsigned short addr, unsigned char val);
extern void writeadc(unsigned short addr, unsigned char val);
extern void write1770(unsigned short addr, unsigned char val);
extern void writeula(unsigned short addr, unsigned char val);
extern void writesysvia(unsigned short addr, unsigned char val);
extern void writeuservia(unsigned short addr, unsigned char val);
extern void writetubehost(unsigned short addr, unsigned char val);
extern void poll1770();
extern void poll8271();
extern void pollbyte();
extern void pollacia();
extern void polladc();
extern void polltape();
extern void logvols(int line);
extern void drawline(int line6502);
extern void exectube(int tubelinecycs);
void write1770fdcdata(unsigned char v);
void end1770readid();
void idcrcerror();
void sectorerror();
void datacrcerror();
void syscb1();
unsigned char readtube(unsigned long addr, char *p);
void writetube(unsigned long addr, unsigned char val, void *p);
void sectorerror1770();
void datacrcerror1770();
void idcrcerror1770();
void load_config();
void initmem();
void loadroms();
void resetuservia();
void resetcrtc();
void resetacia();
void initserial();
void initsnd();
void initvideo();
void reset1770();
void reset8271(int reload);
void resetarm();
void resettube();
void loaddiscsamps();
void openuef(char *fn);
void logsound();
void checkkeys();
void updatebuffer(signed short *buffer, int len);
void loadcmos();
void initadc();
void loadarmrom();
void reset1770s();
void reset8271s();
void remaketablesa();
void remaketables();
void entergui();
void save_config();
void savecmos();
void checkdiscchanged(int disc);
inline void releasekey(int row, int col);
void dcdlow();
void receive(unsigned char val);
void dcd();
void savestate(char *fn);

int bem_main(int argc, char *argv[]);
#ifdef __APPLE__
/* from MenuModel.m */
void process_gui();
void setup_gui();
void shutdown_gui();
#endif
