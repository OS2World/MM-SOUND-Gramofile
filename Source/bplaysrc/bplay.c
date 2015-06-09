/*
** brec/bplay.c (C) David Monro 1996
**
** Copyright under the GPL - see the file COPYING in this directory
**
** Adapted by J.A. Bezemer for use with GramoFile - July 1998
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/time.h>
#include <sys/resource.h>

#ifndef __FreeBSD__
#include <sys/soundcard.h>
#else
#include <machine/soundcard.h>
#endif

#include "fmtheaders.h"

#include "../yesnowindow.h"
void init_curses(void);
void finish_curses(int sig);
#include "../errorwindow.h"
#include "../clrscr.h"
#include "../reclp_main.h"
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif

/* Types and constants */
typedef enum sndf_t {F_UNKNOWN, F_WAV, F_VOC, F_RAW} sndf_t;

#define MSPEED	1
#define MBITS	2
#define MSTEREO	4

/* Globals */
char *progname;
int forked;

/* Prototypes */

#ifdef linux
/* This is in libc, but not in the header files. -- but it IS in
   Red Hat 5.0.... Libc6?
   well i'd guess its not in the headers because its nonstandard, i.e.
   (probably) only exists on linux...  -nox */
#ifndef REDHAT50
extern char *basename(char *name);
#endif
#endif

void Usage(void);
void ErrDie(char *err);
void Die(char *err);

void getbcount(int speed, int bits, int stereo, long *bcount,
	long timelim, long samplim, long timejmp, long sampjmp, long *bjump);
void playraw(int thefd, char hd_buf[20], int speed, int bits, int stereo);
void playwav(int thefd, char hd_buf[20], int mods, int speed, int bits,
  int stereo, long timelim, long samplim, long timejmp, long sampjmp );
/* was: void playwav(int thefd, char hd_buf[20], int mods, int speed, int bits, int stereo); */

void playvoc(int thefd, char hd_buf[20]);

/* extern globals */
extern int audio, abuf_size, fmt_mask;

/* extern prototypes */
extern void init_sound(int recorder);
extern void snd_parm(int speed, int bits, int stereo);
extern void init_shm(void);
extern void shmrec(int outfd, long bcount, int terminate);
extern void diskread(int outfd, long bcount, long skipped, char hd_buf[20], 
    int terminate, int speed, int bits, int stereo);
extern volatile void audiowrite(void);
extern void initsems(int disks, int snds);
extern void cleanupsems(void);


void main(int argc, char *argv[])
{
	
	int recorder = 0;		/* 1 if recording, 0 else */
	int thefd;			/* The file descriptor */
	int speed, bits, stereo;	/* Audio parameters */
	long timelim;			/* Recording time in secs */
	long samplim;			/* Recording time in samples */
	long timejmp;			/* Skip time in secs */
	long sampjmp;			/* Skip time in samples */
	long bcount;			/* Number of bytes to record */
	long bjump;			/* Number of bytes to skip */
	int themask;			/* Permission mask for file */
	sndf_t filetype;		/* The file type */
	int mods;			/* So user can override */
	int optc;			/* For getopt */

	init_curses();

#ifdef linux
	progname = basename(argv[0]);	/* For errors */
#else
	progname = strrchr(argv[0], '/');  /* Replacement for e.g. FreeBSD */
	if (!progname || !*++progname)
	    progname = argv[0];
#endif

	/* Ok, find out if we record or play */
	if (strcmp(progname, "brec_gramo") == 0)
	    recorder = 1;
	else
	{
	    if (!(strcmp(progname, "bplay_gramo") == 0))
		Die("must be called as bplay_gramo or brec_gramo");
	    else
		recorder = 0;
	}
	/* Default values */
	speed = 8000; bits = 8; stereo = 0;
	timelim = 0; samplim = 0; bcount = 0;
	timejmp = 0; sampjmp = 0; bjump = 0;
	filetype = F_UNKNOWN;
	mods = 0;
	/* Parse the options */
	while ((optc = getopt(argc, argv, "Ss:b:t:T:j:J:rvw")) != -1)
	{
		switch(optc)
		{
		case 's':
			speed = atoi(optarg);
			if (speed < 300)
				speed *= 1000;
			mods |= MSPEED;
			break;
		case 'b':
			bits = atoi(optarg);
			if ((bits != 8) && (bits != 16))
				Usage();
			mods |= MBITS;
			break;
		case  'S':
			stereo = 1;
			mods |= MSTEREO;
			break;
		case 't':
			timelim = atol(optarg);
			break;
		case 'T':
			samplim = atol(optarg);
			break;
		case 'j':
			timejmp = atol(optarg);
			break;
		case 'J':
			sampjmp = atol(optarg);
			break;
		case 'r':
			filetype = F_RAW;
			break;
		case 'v':
			filetype = F_VOC;
			break;
		case 'w':
			filetype = F_WAV;
			break;
		default:
			Usage();
		}
	}

#if 1
	/* This program is either set-uid or run by root (I hope...) */
	if (setpriority(PRIO_PROCESS, 0, -20) == -1)
#ifndef LP2CD
		fprintf(stderr, "%s: setpriority: %s: continuing anyway\n",
			progname, strerror(errno))
#endif
		;
	
#endif /* 1 */
	/* Drop out of suid mode before we open any files */
	if(setreuid(geteuid(), getuid()) == -1)
	{
#ifndef LP2CD
		fprintf(stderr, "%s: setreuid: %s: continuing anyway\n",
			progname, strerror(errno));
		fprintf(stderr, "real uid = %d, effective uid = %d\n",
			getuid(), geteuid());
#endif
	}

	/* Calculate the time limit in samples if need be */
	if (optind > argc - 1)
	{
	    if(recorder)	/* No file, so stdin or stdout */
		thefd = 1;
	    else
		thefd = 0;
	}
	else
	{
	    /* Open a file */
	    if(recorder)
	    {
		/* Ok, get the mask for the opening the file */
		themask = umask(0);
		umask(themask);
		if ((thefd = open(argv[optind], O_CREAT | O_TRUNC | O_WRONLY,
		    (~themask) & 0666)) == -1)
		    ErrDie(argv[optind]);
	    }
	    else
		if ((thefd = open(argv[optind], O_RDONLY)) == -1)
		    ErrDie(argv[optind]);
	}

	/* Open and set up the audio device */
	init_sound(recorder);

	/* Check if the card is capable of the requested operation */
	/*
	** Can't check for stereo yet, just number of bits. Also,
	** can't check for 12 bit operations yet.
	*/

#if 0
	if ((bits == 8) & !(fmt_mask & AFMT_U8)) Die("Format not supported by audio device");
	if ((bits == 16) & !(fmt_mask & AFMT_S16_BE)) Die("Format not supported by audio device");
#endif

	/* Set up the shared buffers and semaphore blocks */
	init_shm();

	/* Call the appropriate routine */
	if (recorder)
	{
		getbcount(speed, bits, stereo, &bcount,
			 timelim, samplim, timejmp, sampjmp, &bjump);
#ifdef DEBUG
		fprintf(stderr, "bcount: %ld\n", bcount);
#endif


/* By CS: */
			clearscreen(RECLP_HEADERTEXT);
			if (!yesno_window(
"Press Enter to start recording.",
				     " Start ", " Cancel ", 1))
			{
			    close(thefd);
			    close(audio);
			    finish_curses(0);
			}

			header(RECLP_HEADERTEXT);
			error_window_display(
"Press Enter to stop recording.", " Stop ");

/* End CS */


		if (filetype == F_UNKNOWN)
			filetype = F_RAW;	/* Change to change default */
		switch(filetype)
		{
		case F_WAV:
			/* Spit out header here... */
#ifndef LP2CD
			fprintf(stderr, "Writing MS WAV sound file");
#endif
			{
				wavhead header;

				char *riff = "RIFF";
				char *wave = "WAVE";
				char *fmt = "fmt ";
				char *data = "data";

				memcpy(&(header.main_chunk), riff, 4);
				header.length = sizeof(wavhead) - 8 + bcount;
				memcpy(&(header.chunk_type), wave, 4);

				memcpy(&(header.sub_chunk), fmt, 4);
				header.sc_len = 16;
				header.format = 1;
				header.modus = stereo + 1;
				header.sample_fq = speed;
				header.byte_p_sec = ((bits > 8)? 2:1)*(stereo+1)*speed;
/* Correction by J.A. Bezemer: */
				header.byte_p_spl = ((bits > 8)? 2:1)*(stereo+1);
			/* was: header.byte_p_spl = (bits > 8)? 2:1; */

				header.bit_p_spl = bits;

				memcpy(&(header.data_chunk), data, 4);
				header.data_length = bcount;
				write(thefd, &header, sizeof(header));
			}
		case F_RAW:
			if (filetype == F_RAW)
#ifndef LP2CD
				fprintf(stderr, "Writing raw sound file")
#endif
				;
#ifndef LP2CD
			fprintf(stderr, ", %dHz, %dbit, %s\n", speed, bits, (stereo)? "stereo":"");
#endif
			snd_parm(speed, bits, stereo);
			initsems(0, 1);

			shmrec(thefd, bcount, 1);
			break;
		case F_VOC:
			/* Spit out header here... */
			fprintf(stderr, "Writing CL VOC sound file");
			fprintf(stderr, ", %dHz, %dbit, %s\n", speed, bits, (stereo)? "stereo":"");
			{
				vochead header;
				blockTC ablk;
				blockT9 bblk;
				int i;
				char fill = 0;

				for (i=0;i<20;i++)
					header.Magic[i] = VOC_MAGIC[i];
				header.BlockOffset = 0x1a;
				header.Version = 0x0114;
				header.IDCode = 0x111F;
				write(thefd, &header, sizeof(vochead));

				snd_parm(speed, bits, stereo);
				initsems(0, 1);

				i = (bcount >= 0xFFFFF2)? 0xFFFFF2 + 12 : bcount;
				ablk.BlockID = 9;
				ablk.BlockLen[0] = (i + 12) & 0xFF;
				ablk.BlockLen[1] = ((i + 12) >> 8) & 0xFF;
				ablk.BlockLen[2] = ((i + 12) >> 16) & 0xFF;
				bblk.SamplesPerSec = speed;
				bblk.BitsPerSample = bits;
				bblk.Channels = stereo + 1;
				bblk.Format = (bits == 8)? 0 : 4;
				write(thefd, &ablk, sizeof(ablk));
				write(thefd, &bblk, sizeof(bblk));
				shmrec(thefd, i, 1);
				write(thefd, &fill, 1);
			}
			break;
		default:
			Die("internal error - fell out of switch");
		}
	}
	else
	{
		int count;
		pid_t pid;
		char hd_buf[20];	/* Holds first 20 bytes */

		count = read(thefd, hd_buf, 20);
		if (count < 0) ErrDie("read");
		if (count < 20) Die("input file less than 20 bytes long.");

		initsems(1, 0);

		pid = fork();
		if(!pid)
			audiowrite();	/* Doesn't return */
		forked = 1;

	    if (filetype == F_RAW)
			playraw(thefd, hd_buf, speed, bits, stereo);
	    else
	    {
		/* Pick the write output routine */
		if(strstr(hd_buf, VOC_MAGIC) != NULL)
			playvoc(thefd, hd_buf);
		else if(strstr(hd_buf, "RIFF") != NULL)
			playwav(thefd, hd_buf, mods, speed, bits, stereo,
				timelim, samplim, timejmp, sampjmp);
		else /* Assume raw data */
			playraw(thefd, hd_buf, speed, bits, stereo);
	    }
		wait(NULL);
		cleanupsems();
	}

	finish_curses(0);
}

void Usage(void)
{
int i;
	printw(
		"Usage: %s [-S] [-s Hz] [-b 8|16] [-t secs] [-r|-v|-w] [filename]\n",
		progname);
	printw("\nPress any key...");
	refresh();
	i = getch();

	finish_curses(-1);
	exit(1);
}

void ErrDie(char * err)
{
char string[500];
	sprintf(string, "%s: %s", err, strerror(errno));
	error_window(string);
	finish_curses(-1);
	exit(-1);
}

void Die(char * err)
{
char string[500];
	sprintf(string, "%s", err);
	error_window(string);
	finish_curses(-1);
	exit(-1);
}

void getbcount(int speed, int bits, int stereo, long *bcount,
	long timelim, long samplim, long timejmp, long sampjmp, long *bjump)
{
        if(timelim)
        {
/* fprintf(stderr,"tl=%ld, spd=%d, bits=%d\n",timelim, speed, bits); */
                *bcount = speed*timelim*((bits+7)/8L);
/* fprintf(stderr,"bc=%ld\n",bcount); */
                if (stereo) *bcount <<= 1;
/* fprintf(stderr,"bc2=%ld\n",bcount); */
        }
        if(samplim)
        {
                *bcount = samplim*((bits+7)/8L);
                if (stereo) *bcount <<= 1;
        }
        if(timejmp)
        {
                *bjump = speed*timejmp*((bits+7)/8L);
                if (stereo) *bjump <<= 1;
        }
        if(sampjmp)
        {
                *bjump = sampjmp*((bits+7)/8L);
                if (stereo) *bjump <<= 1;
        }
}

void playraw(int thefd, char hd_buf[20], int speed, int bits, int stereo)
{
    fprintf(stderr, "Playing raw data : %d bit, Speed %d %s ...\n",
	bits, speed, (stereo)? "Stereo" : "Mono");
    diskread(thefd, 0,0, hd_buf, 1, speed, bits, stereo);
}

void playwav(int thefd, char hd_buf[20], int mods, int speed, int bits,
  int stereo, long timelim, long samplim, long timejmp, long sampjmp )
{
    wavhead wavhd;
    int count;
long bcount = 0, bjump = 0;

    memcpy((void*)&wavhd, (void*)hd_buf, 20);
    count = read(thefd, ((char*)&wavhd)+20, sizeof(wavhd) - 20);
    if(wavhd.format != 1) Die("Input is not a PCM WAV file");
#ifndef LP2CD
    if (! (mods&MSPEED))
/* This should _not_ be overridden by the commandline options, for those
   are always 44100 Hz 16 bit Stereo, and we want _all_ wavs to
   play correctly. */
#endif
	speed = wavhd.sample_fq;
#ifndef LP2CD
    if (! (mods&MBITS))
#endif
	bits = wavhd.bit_p_spl;
#ifndef LP2CD
    if (! (mods&MSTEREO))
#endif
	stereo = wavhd.modus - 1;
#ifndef LP2CD
    fprintf(stderr, "Playing WAVE : %d bit, Speed %d %s ...\n",
	bits, speed, (stereo)? "Stereo" : "Mono");
#endif
/* By CS: */
    getbcount(speed, bits, stereo, &bcount, timelim, samplim,
		timejmp, sampjmp, &bjump);
    if (bjump) lseek(thefd, bjump, SEEK_CUR);

    diskread(thefd, bcount, bjump, NULL, 1, speed, bits, stereo);
}

void playvoc(int thefd, char hd_buf[20])
{
    int count;
    int speed=0, bits=0, stereo=0;
    int inloop=0, loop_times;
    long bytecount, loop_pos=0;
    vochead vochd;
    blockTC ccblock;
    int lastblocktype = -1;
    int quit = 0;

    fprintf(stderr, "Playing Creative Labs Voice file ...\n");
    memcpy((void*)&vochd, (void*)hd_buf, 20);
    count = read(thefd, ((char*)&vochd)+20, sizeof(vochd) - 20);
    fprintf(stderr, "Format version %d.%d\n", vochd.Version>>8,
	vochd.Version&0xFF);
    if (vochd.IDCode != (~vochd.Version+0x1234))
	fprintf(stderr, "Odd - version mismatch - %d != %d\n",
	    vochd.IDCode, ~vochd.Version+0x1234);
    if(sizeof(vochd) < vochd.BlockOffset)
    {
	int off = vochd.BlockOffset - sizeof(vochd);
	char *junk;
	junk = (char*) malloc(off);    
	read(thefd, junk, off);
    }
    while(!quit)
    {
    if ((read(thefd, (char*)&ccblock, sizeof(ccblock))) == -1)
    {
#ifdef DEBUG
	fprintf(stderr, "Terminating\n");
#endif
	diskread(thefd, -1,0, NULL, 1, speed, bits, stereo);
	quit = 1;
	continue;
    }
#ifdef DEBUG
    fprintf(stderr, "Block of type %d found\n", ccblock.BlockID);
#endif
    switch(ccblock.BlockID)
    {
    case 1:
	{
	blockT1 tblock;
	read(thefd, (char*)&tblock, sizeof(tblock));
	if(tblock.PackMethod != 0) Die("Non PCM VOC block");
	if (lastblocktype != 8)
	{
	    speed = 256000000/(65536 - (tblock.TimeConstant << 8));
	    bits = 8;
	    stereo = 0;
	}
	bytecount = DATALEN(ccblock) -2;
	diskread(thefd, bytecount,0, NULL, 0, speed, bits, stereo);
	lastblocktype = 1;
	}
	break;
    case 8:
	{
	blockT8 tblock;
	read(thefd, (char*)&tblock, sizeof(tblock));
	if(tblock.PackMethod != 0) Die("Non PCM VOC block");
	speed = 256000000/(65536 - tblock.TimeConstant);
	bits = 8;
	stereo = tblock.VoiceMode;
	if (stereo) speed >>=1;
	lastblocktype = 8;
	}
	break;
    case 9:
	{
	blockT9 tblock;
	read(thefd, (char*)&tblock, sizeof(tblock));
	if(tblock.Format != 0 && tblock.Format != 4)
	    Die("Non PCM VOC block");
	speed = tblock.SamplesPerSec;
	bits = tblock.BitsPerSample;
	stereo = tblock.Channels - 1;
	bytecount = DATALEN(ccblock) - 12;
	diskread(thefd, bytecount,0, NULL, 0, speed, bits, stereo);
	lastblocktype = 9;
	}
	break;
    case 0:
#ifdef DEBUG
	fprintf(stderr, "Terminating\n");
#endif
	diskread(thefd, -1,0, NULL, 1, speed, bits, stereo);
	quit = 1;
	break;
    case 6:
	inloop = 1;
	read(thefd, (char*)&loop_times, 2);
	loop_times++;
#ifdef DEBUG
fprintf(stderr, "Beginning loop %d\n", loop_times);
#endif
	loop_pos = lseek(thefd, 0, SEEK_CUR);
	if(loop_pos == -1)
	{
	    fprintf(stderr, "Input not seekable - loop will only be played once\n");
	    loop_times = 1;
	}
	lastblocktype = ccblock.BlockID;
	break;
    case 7:
	if(!inloop)
	{
	    fprintf(stderr, "Loop end with no loop start - ignoring\n");
	    break;
	}
	if(loop_times != 0xFFFF) --loop_times;
	if(loop_times)
	{
#ifdef DEBUG
	    fprintf(stderr, "Looping...\n");
#endif
	    lseek(thefd, loop_pos, SEEK_SET);
	}
	else
	    inloop = 0;
	lastblocktype = ccblock.BlockID;
	break;
    default:
	{
	int rd = 0, trgt = BUFSIZ;
	char junkbuf[BUFSIZ];
    
	fprintf(stderr, "Ignored\n");
	bytecount = DATALEN(ccblock);
	while(rd < bytecount)
	{
	    if (rd + trgt > bytecount)
		trgt = bytecount - rd;
	    count = read(thefd, junkbuf, trgt);
	    if (count < 0) ErrDie("read");
	    if (count == 0) Die("premature eof in input");
	    rd += count;
	}
	lastblocktype = ccblock.BlockID;
	}
	break;
    }
    }
}

/* ----- BELOW: ADDED BY CS ------------------------------------------------ */


void init_curses(void)
{
	initscr();
	keypad(stdscr, TRUE);
	nonl();
	cbreak();
	noecho();

	refresh(); /* Xterms erase everything after the first refresh,
		      so refresh one time before anything is added. */

	return;
}

void finish_curses(int sig)
{
	endwin();
	exit(0);
}

