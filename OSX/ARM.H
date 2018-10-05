/*ARM*/
extern unsigned long *usrregs[16],userregs[16],superregs[16],fiqregs[16],irqregs[16];
extern unsigned long armregs[16];
extern int armirq,armfiq;
#define PC ((armregs[15])&0x3FFFFFC)

void resetarm();
void execarm(int cycles);
void dumparmregs();
extern int databort;
