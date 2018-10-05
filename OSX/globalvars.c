/*
 *  globalvars.c
 *  B-Em
 *
 *  Created by Thomas Harte on 13/01/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "b-em.h"

/*6502*/
int interrupt;
int oldnmi,nmilock;
unsigned char a,x,y,s;
unsigned short pc;
struct __ptype p;
int timetolive;

/*VIAs*/
VIA sysvia,uservia;

/*Video*/
unsigned short vidbank;

/*Disc*/
int ddensity, disctime;
int adfs[2];
int motorofff;
SAMPLE *seeksmp;
SAMPLE *seek2smp;
SAMPLE *seek3smp;
SAMPLE *stepsmp;
SAMPLE *motorsmp;
SAMPLE *motoroffsmp;
SAMPLE *motoronsmp;
int curside;
int discint;
int sectorpos;

/*Config*/
int fasttape;
int hires;
int blurred,mono;
int model,tube;
int autoboot, firstautoboot;
int fasttape;

/*Sound*/
AUDIOSTREAM *as;

/*Video*/
BITMAP *buffer;
unsigned short vidmask;

/*ADC*/
int adcconvert;

/*Tape*/
int motor;

/* ACIA */
unsigned char aciacr, aciadr;