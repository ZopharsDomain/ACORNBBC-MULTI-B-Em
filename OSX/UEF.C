/*B-em 1.0 by Tom Walker*/
/*UEF handling (including HQ-UEF support)*/

#include <allegro.h>
#include <zlib.h>
#include <stdio.h>
#include "b-em.h"

int startchunk;
int blocks=0;
int tapelcount,tapellatch,pps;
int intone=0;
gzFile *uef;

int inchunk=0,chunkid=0,chunklen=0;
int chunkpos=0,chunkdatabits=8;
float chunkf;

#define gzread2(uef,targ) targ = gzgetc(uef); targ |= gzgetc(uef) << 8;
#define gzread4(uef,targ) targ = gzgetc(uef); targ |= gzgetc(uef) << 8; targ |= gzgetc(uef) << 16; targ |= gzgetc(uef) << 24;

void openuef(char *fn)
{
      int c;
      if (uef)
         gzclose(uef);
      uef=gzopen(fn,"rb");
      if (!uef) return;
      for (c=0;c<12;c++)
          gzgetc(uef);
      inchunk=chunklen=chunkid=chunkpos=0;
      tapellatch=(1000000/(1200/10))/64;
      tapelcount=0;
      pps=120;
}

void rewindit()
{
        int c;
        gzseek(uef,0,SEEK_SET);
        for (c=0;c<12;c++)
            gzgetc(uef);
        inchunk=chunklen=chunkid=0;
}

int ueffileopen()
{
        if (!uef)
           return 0;
        return 1;
}

void polltape()
{
        int c;
        unsigned long templ;
        float *tempf;
        unsigned char temp;
        if (!uef)
           return;
        if (!inchunk)
        {
                startchunk=1;
//                printf("%i ",gztell(uef));
				gzread2(uef, chunkid);
				gzread4(uef, chunklen);
                if (gzeof(uef))
                {
                        gzseek(uef,12,SEEK_SET);
					gzread2(uef, chunkid);
					gzread4(uef, chunklen);
                }
                inchunk=1;
                chunkpos=0;
//                printf("Chunk ID %04X len %i\n",chunkid,chunklen);
        }
//        else
//           printf("Chunk %04X\n",chunkid);
        switch (chunkid)
        {
                case 0x000: /*Origin*/
                for (c=0;c<chunklen;c++)
                    gzgetc(uef);
                inchunk=0;
                return;

                case 0x005: /*Target platform*/
                for (c=0;c<chunklen;c++)
                    gzgetc(uef);
                inchunk=0;
                return;

                case 0x100: /*Raw data*/
                if (startchunk)
                {
                        dcdlow();
                        startchunk=0;
                }
                chunklen--;
                if (!chunklen)
                {
                        inchunk=0;
                        blocks++;
                }
                receive(gzgetc(uef));
                return;

                case 0x104: /*Defined data*/
                if (!chunkpos)
                {
                        chunkdatabits=gzgetc(uef);
                        gzgetc(uef);
                        gzgetc(uef);
                        chunklen-=3;
                        chunkpos=1;
                }
                else
                {
                        chunklen--;
                        if (chunklen<=0)
                           inchunk=0;
                        temp=gzgetc(uef);
//                        printf("%i : %i %02X\n",gztell(uef),chunklen,temp);
                        if (chunkdatabits==7) receive(temp&0x7F);
                        else                  receive(temp);
                }
                return;

                case 0x110: /*High tone*/
                if (!intone)
                {
                        dcd();
                        intone=gzgetc(uef);
                        intone|=(gzgetc(uef)<<8);
                        intone>>=2;
                        if (!intone) intone=1;
//                        printf("intone %i\n",intone);
                }
                else
                {
                        intone--;
                        if (intone==0)
                        {
                                inchunk=0;
                        }
                }
/*                if (!intone)
                {
                        templ=gzgetc(uef); templ|=(gzgetc(uef)<<8);
//                        printf("High tone %04X\n",templ);
                        if (templ>20)
                        {
                                dcd();
                                intone=6;
                        }
                        else
                           inchunk=0;
                }
                else
                {
                        intone--;
                        if (intone==0)
                        {
                                inchunk=0;
                        }
                }*/
                return;

                case 0x111: /*High tone with dummy byte*/
                if (!intone)
                {
                        dcd();
                        intone=3;
                }
                else
                {
                        if (intone==4)
                           dcd();
                        intone--;
                        if (intone==0 && inchunk==2)
                        {
                                inchunk=0;
                                gzgetc(uef); gzgetc(uef);
                                gzgetc(uef); gzgetc(uef);
                        }
                        else if (!intone)
                        {
                                inchunk=2;
                                intone=4;
                                receive(0xAA);
                        }
                }
                return;

                case 0x112: /*Gap*/
                if (!intone)
                {
//                        dcd();
                        intone=gzgetc(uef);
                        intone|=(gzgetc(uef)<<8);
                        intone>>=2;
//                        printf("gap intone %i\n",intone);
                        if (!intone) intone=1;
                }
                else
                {
                        intone--;
                        if (intone==0)
                        {
                                inchunk=0;
                        }
                }
                return;
/*                if (!intone)
                {
                        intone=3;
                }
                else
                {
                        intone--;
                        if (intone==0)
                        {
                                inchunk=0;
                                gzgetc(uef); gzgetc(uef);
                        }
                }*/
                return;

                case 0x113: /*Float baud rate*/
                templ=gzgetc(uef);
                templ|=(gzgetc(uef)<<8);
                templ|=(gzgetc(uef)<<16);
                templ|=(gzgetc(uef)<<24);
                tempf=(float *)&templ;
                tapellatch=(1000000/((*tempf)/10))/64;
                pps=(*tempf)/10;
                inchunk=0;
                return;

                case 0x116: /*Float gap*/
                if (!chunkpos)
                {
                        templ=gzgetc(uef);
                        templ|=(gzgetc(uef)<<8);
                        templ|=(gzgetc(uef)<<16);
                        templ|=(gzgetc(uef)<<24);
                        tempf=(float *)&templ;
                        chunkf=*tempf;
//                        printf("Gap %f\n",chunkf);
                        chunkpos=1;
                }
                else
                {
//                        printf("Gap now %f\n",chunkf);
                        chunkf-=((float)1/(float)pps);
                        if (chunkf<=0) inchunk=0;
                }
                return;

                case 0x114: /*Security waves*/
                case 0x115: /*Polarity change*/
//                default:
                for (c=0;c<chunklen;c++)
                    gzgetc(uef);
                inchunk=0;
                return;

//116 : float gap
//113 : float baud rate

        }
        allegro_exit();
        printf("Bad chunk ID %04X length %i\n",chunkid,chunklen);
        exit(-1);
}

void closeuef()
{
        if (uef)
           gzclose(uef);
}
