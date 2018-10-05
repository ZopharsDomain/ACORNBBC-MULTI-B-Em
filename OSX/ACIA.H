extern unsigned char aciacr; /*Control register*/
extern unsigned char aciasr; /*Status register*/
extern unsigned char aciadr; /*Data register*/
extern int aciadrf;          /*Data register full?*/

unsigned char readacia(unsigned short addr);
void writeacia(unsigned short addr, unsigned char val);
void writetoacia(unsigned char val);
