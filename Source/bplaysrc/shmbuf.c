/*
**
** bplay/shmbuf.c (C) David Monro 1996
**
** Copyright under the GPL - see the file COPYING in this directory
**
** Adapted by J.A. Bezemer for use with GramoFile - July, August 1998
**
** Patch to compile with egcs from Daniel Kobras, applied by J.A. Bezemer
** - October, 1998
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include "../errorwindow.h"
#include "../clrscr.h"
#include "../playwav.h"
#include "../secshms.h"
#include "../reclp_main.h"
#include "../boxes.h"
#include "../buttons.h"
#include <string.h>
#ifndef OLD_CURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif

#ifndef SEMMSL
#ifdef __FreeBSD__
/*
 * you may want to adjust this to whats configured into your kernel,
 * 30 is just the current default. (see <sys/sem.h>)  -nox
 */
#define SEMMSL  30
#else
#define SEMMSL  32
#endif
#endif

#ifdef _SEM_SEMUN_UNDEFINED
union semun				/* This has gone out of standard  */
{					/* libc headers as of glibc2.1,   */
	int val;			/* we need to define it ourselves.*/
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};
#endif

extern void finish_curses(int sig);

/* The size of the big array */
/* (currently 256K - nearly 1.5 sec at max rate) */
#define BIGBUFFSIZE 0x040000

/* Types */
typedef struct blockinf_t
{
    int count;	/* How many bytes in this buffer */
    int last;	/* Should we terminate after this buffer? */
    int setit;	/* Should we re-set the audio parameters to be the ones here? */
    int speed;
    int bits;
    int stereo;
} blockinf_t;

/* Statics - mostly shared memory etc */
static int shmid, shmid2, *disksemid, *sndsemid;
static char *bigbuff;
static char **buffarr;
static int numbuffs, numsemblks;
static blockinf_t *buffinf;

/* prototypes */
void cleanupsems(void);
static void sighandler(int i);

/* Extern globals */
extern int abuf_size;
extern int audio;
extern char *progname;


/* extern prototypes */
extern void ErrDie(char *err);
extern void snd_parm(int speed, int bits, int stereo);
extern void sync_audio(void);

void init_shm(void)
{
    int i;

	/* Create, attach and mark for death the big buffer */
    shmid = shmget(IPC_PRIVATE, BIGBUFFSIZE,
	IPC_EXCL | IPC_CREAT | 0600);
    if (shmid == -1)
	ErrDie("shmget");
    bigbuff = shmat(shmid, IPC_RMID, SHM_RND);
    if (bigbuff == (char*)-1)
    {
	perror("shmat");
	if(shmctl(shmid, IPC_RMID, NULL))
		perror("shmctl");
	exit(-1);
    }
    if(shmctl(shmid, IPC_RMID, NULL))
	ErrDie("shmctl");

    /* Create an array of pointers. Point them at equally spaced
    ** chunks in the main buffer, to give lots of smaller buffers
    */
    numbuffs = BIGBUFFSIZE/abuf_size;
    buffarr = (char**)malloc(numbuffs*sizeof(char*));
    for (i=0; i<numbuffs; i++)
	buffarr[i] = bigbuff + i * abuf_size;

    /* Create a small amount of shared memory to hold the info
    ** for each buffer.
    */
    shmid2 = shmget(IPC_PRIVATE, numbuffs*sizeof(blockinf_t),
	IPC_EXCL | IPC_CREAT | 0600);
    if (shmid2 == -1)
	ErrDie("shmget");
    buffinf = (blockinf_t*)shmat(shmid2, IPC_RMID, SHM_RND);
    if (buffinf == (blockinf_t*)((char*)-1))
    {
	perror("shmat");
	if(shmctl(shmid2, IPC_RMID, NULL))
		perror("shmctl");
	exit(-1);
    }
    if(shmctl(shmid2, IPC_RMID, NULL))
	ErrDie("shmctl");

#if USEBUFFLOCK
	/* Ok, go root to lock the buffers down */
    if(setreuid(geteuid(), getuid()) == -1)
    {
#ifndef LP2CD
	fprintf(stderr, "%s: setreuid: %s: continuing anyway\n",
	    progname, strerror(errno));
	fprintf(stderr, "real uid = %d, effective uid = %d\n",
	    getuid(), geteuid());
#endif
    }

    if(shmctl(shmid, SHM_LOCK, NULL) || shmctl(shmid2, SHM_LOCK, NULL))
#ifndef LP2CD
	fprintf(stderr,
	    "%s: shmctl: %s: continuing with unlocked buffers\n",
		progname, strerror(errno))
#endif
		;

    if(setreuid(geteuid(), getuid()) == -1)
    {
#ifndef LP2CD
	fprintf(stderr, "%s: setreuid: %s: continuing anyway\n",
	    progname, strerror(errno));
	fprintf(stderr, "real uid = %d, effective uid = %d\n",
	    getuid(), geteuid());
#endif
    }

#endif
    /* Set up the appropriate number of semaphore blocks */
    numsemblks = numbuffs/SEMMSL;
    if((numsemblks * SEMMSL) < numbuffs)
	numsemblks++;
    /* Malloc arrays of semaphore ids (ints) for the semaphores */
    if ((disksemid = (int*)malloc(sizeof(int)*numsemblks)) == NULL)
	ErrDie("malloc");
    if ((sndsemid = (int*)malloc(sizeof(int)*numsemblks)) == NULL)
	ErrDie("malloc");
    /* Create the semaphores */
    for (i=0;i<numsemblks;i++)
    {
	if ((disksemid[i] = semget(IPC_PRIVATE, SEMMSL,
	    IPC_EXCL | IPC_CREAT | 0600)) == -1)
	    ErrDie("semget");
	if ((sndsemid[i] = semget(IPC_PRIVATE, SEMMSL,
	    IPC_EXCL | IPC_CREAT | 0600)) == -1)
	    ErrDie("semget");
    }
    /* Catch some signals, so we clean up semaphores */
    signal(SIGINT, sighandler);
}


/* Does an up on the appropriate semaphore */
void up(int *semblk, int xsemnum)
{
    struct sembuf sbuf;

    sbuf.sem_num = xsemnum%SEMMSL;
    sbuf.sem_op = 1;
    sbuf.sem_flg = 0;

    if (semop(semblk[xsemnum/SEMMSL], &sbuf, 1) == -1)
	perror("semop");
}

/* Does a down on the appropriate semaphore */
void down(int *semblk, int xsemnum)
{
    struct sembuf sbuf;

    sbuf.sem_num = xsemnum%SEMMSL;
    sbuf.sem_op = -1;
    sbuf.sem_flg = 0;

    if (semop(semblk[xsemnum/SEMMSL], &sbuf, 1) == -1)
	perror("semop");
}

/* The recording function */
void shmrec(int outfd, long totalcount, int terminate)
{
    pid_t pid;
    int i;
    button_t ok_button;
    char timestring[100];

    nodelay(stdscr, TRUE);

    sync();

    pid = fork();
    if (pid == 0)
    {
	int cbuff = 0;
	long totalrd = 0; /* we need it here too... */

#ifdef VUMETER
	signed short *ssptr;
	signed short leftvalue, rightvalue, maxleft, maxright;
	long samplesabove50pct = 0;
	long samplesabove90pct = 0;
	long samplesabove99pct = 0;
	long samplestooloud = 0;
#endif

	/* Uncatch the signals */
	signal(SIGINT, SIG_DFL);

	/* Child process writes the disk */
	while(1)
	{
	    long count, numwr, trgt;
	    char *tmpptr;

	    /* Grab the buffer. Blocks till it is OK to do so. */
	    down(disksemid, cbuff);
	    /* Spit it out */
	    tmpptr = buffarr[cbuff];
#ifdef VUMETER
	    ssptr = (signed short *) tmpptr;
#endif
	    numwr = 0;
	    trgt = buffinf[cbuff].count;
	    while ( (numwr < trgt) &&
	      ((count = write(outfd, tmpptr, trgt - numwr)) > 0) )
	    {
		numwr += count;
		tmpptr += count;
                totalrd += count;
	    }

#ifdef VUMETER
	    maxleft = 0;
	    maxright = 0;
	    trgt = buffinf[cbuff].count;

	    for (numwr = 0; numwr < trgt; numwr += 4 )
	    {
		leftvalue=abs(*ssptr);
		if (leftvalue > maxleft) maxleft = leftvalue;
		ssptr ++;

		rightvalue=abs(*ssptr);
		if (rightvalue > maxright) maxright = rightvalue;
		ssptr ++;

		if (leftvalue > 16383 || rightvalue > 16383) 
		  samplesabove50pct ++;
		if (leftvalue > 29490 || rightvalue > 29490) 
		  samplesabove90pct ++;
		if (leftvalue > 32439 || rightvalue > 32439) 
		  samplesabove99pct ++;
		if (leftvalue > 32764 || rightvalue > 32764) 
		  samplestooloud ++;
	    }
	    move(ERROR_WINDOW_Y + 2,ERROR_WINDOW_X +1);
	    addstr("L: =");
	    leftvalue = maxleft / (32768/(ERROR_WINDOW_W-6));
	    for (numwr = 0; numwr < leftvalue; numwr ++)
		addch(numwr >= ERROR_WINDOW_W-8 ? '#' : '=');
	    for (; numwr < ERROR_WINDOW_W-6; numwr ++)
		addch(' ');

	    move(ERROR_WINDOW_Y + 3,ERROR_WINDOW_X +1);
	    addstr("R: =");
	    rightvalue = maxright / (32768/(ERROR_WINDOW_W-6));
	    for (numwr = 0; numwr < rightvalue; numwr ++)
		addch(numwr >= ERROR_WINDOW_W-8 ? '#' : '=');
	    for (; numwr < ERROR_WINDOW_W-6; numwr ++)
		addch(' ');

	    move(0,79);
	    refresh();
#endif

	    /* Mark the buffer as clean */
	    up(sndsemid, cbuff);

	    /* If the block was marked as the last one, stop */
	    if (buffinf[cbuff].last)
		break;
	    /* Advance the pointer */
	    cbuff++;
	    cbuff%=numbuffs;
	}
	/* Tidy up and exit, we are being waited for */
	close(outfd);

#ifdef VUMETER
	/* Display some informative data. This is really weird: we
           display it here (in the child), then exit() and wait for a key
           in the _parent_ process. But it's the only simple way to get it
           working */
	clearscreen(RECLP_HEADERTEXT);                          

	printw("\n\n");
	printw("      Recording information:\n\n\n");
	fsec2hmsf ( (double) totalrd / (4 * 44100) , timestring);
	printw("    Recorded time    : %s\n", timestring);
	printw("    Recorded samples : %11ld\n", totalrd / 4);
	printw("    Recorded bytes   : %11ld  (excl. header)\n", totalrd);
	printw("\n");
	printw("    Samples above 50%% of max. volume  : %9ld  (%5.1f%%)\n",
          samplesabove50pct, samplesabove50pct * 100. / (totalrd/4));
	printw("    Samples above 90%% of max. volume  : %9ld  (%5.1f%%)\n",
          samplesabove90pct, samplesabove90pct * 100. / (totalrd/4));
	printw("    Samples above 99%% of max. volume  : %9ld  (%5.1f%%)\n",
          samplesabove99pct, samplesabove99pct * 100. / (totalrd/4));
	printw("    Really too loud (clipped) samples : %9ld  (%5.1f%%)\n",
          samplestooloud, samplestooloud * 100. / (totalrd/4));

#if 0
	/* The computation of the avg volume is not simple. One approach
           is totalvolume+=abs(sampleleft)+abs(sampleright) for each
           sample, but if totalvolume gets too big, nothing is added any
           more (lack of precision). If anyone has a better (working)
           idea, please tell me! */
	printw("\n");
	printw("    Average volume : %7.1f  (%5.1f%% of max.)\n",
          totalvolume / (totalrd/2),
          (totalvolume / (totalrd/2) * 100) / 32768);
					/* (totalrd/2)=((totalrd/4)*2) */
#endif /* 0 */

	ok_button.text = " OK ";
	ok_button.y = 20;
	ok_button.x = 71;
	ok_button.selected = TRUE;

        button_display (&ok_button);
        mybox (ok_button.y - 1, ok_button.x - 1,
               3, strlen (ok_button.text) + 2);
	move (0, 79);
	refresh();
#endif

	exit(0);
    }
    else
    {
	/* Parent reads audio */
	int cbuff = 0;
	long totalrd = 0;

	int stoprecording=0;
	int in_ch;

	while (totalrd < totalcount && !stoprecording)
	{
	    long trgt, count, numrd;
	    char *tmpptr;
	    trgt = totalcount - totalrd;
	    if (trgt > abuf_size)
		trgt = abuf_size;
	    /* Get the buffer. Blocks until OK to do so */
	    down(sndsemid, cbuff);
	    /* Read a block of data */
	    numrd = 0;
	    tmpptr = buffarr[cbuff];
	    while( (numrd < trgt) &&
		((count = read(audio, tmpptr, trgt - numrd)) > 0) )
	    {
		numrd += count;
		tmpptr += count;
	    }
	    /* Update the count for this block */
	    buffinf[cbuff].count = numrd;
	    /* Mark the buffer dirty */
	    up(disksemid, cbuff);
	    /* Update the amount done */
	    totalrd += numrd;
	    /* Tell the reader to stop if needed */

	    in_ch=getch();
#ifdef DEBUG
printw(" %d",cbuff);
#endif
	    if (in_ch==KEY_ENTER || in_ch==13 || in_ch==27)
		stoprecording=1;
	    if ( ((totalrd >= totalcount) && terminate) || stoprecording)
		buffinf[cbuff].last = 1;
	    /* Update the counter */
	    cbuff++;
	    cbuff%=numbuffs;
	}
	/* Tidy up and wait for the child */
	close(audio);

	/* XXX fix the occasional deadlock in the following wait()  -nox */
	for (i = 0; i < numbuffs; i++)
	    up(disksemid, i);

	wait(NULL);
	
	/* Free all the semaphores */
	cleanupsems();

#ifdef VUMETER
	nodelay(stdscr, TRUE);

	/* child has displayed informative data */
	do
	  i = getch ();
	while (i != 13 && i != KEY_ENTER && i != 27);
#endif
    }
}

void diskread(int infd, long totalplay, long skipped, char hd_buf[20], 
    int terminate, int speed, int bits, int stereo)
{

    int count, i, limited = 0;
    char *tmppt;
    long numread, totalread = 0;
    int first = 1;

    static int triggered = 0;	/* Have we let the writer go? */
    static int cbuff = 0;	/* Which buffer */

char tempstring[50];
int in_ch;

    if (totalplay) limited = 1;
    if (totalplay == -1)
    {
	totalplay = 0;
	limited = 1;
    }

    clearscreen(PLAYWAV_HEADERTEXT);
    error_window_display("Playing...", " Stop ");
    nodelay(stdscr, TRUE);
    refresh();

    while (1)
    {
	int trgt;

	/* Wait for a clean buffer */
	down(disksemid, cbuff);
	/* Read from the input */
	numread = 0;
	trgt = abuf_size;
	if (limited && (totalread + trgt > totalplay))
	    trgt = totalplay - totalread;
	tmppt = buffarr[cbuff];
	if(first && trgt)
	{
	    buffinf[cbuff].setit = 1;
	    buffinf[cbuff].speed = speed;
	    buffinf[cbuff].bits = bits;
	    buffinf[cbuff].stereo = stereo;
	    if(hd_buf)
	    {
		memcpy(tmppt, hd_buf, 20);
		tmppt += 20; numread = 20;
	    }
	    first = 0; 
	}
	while ( (numread < trgt) &&
	    ((count = read(infd, tmppt, trgt - numread)) != 0) )
	{
	    tmppt += count; numread += count;
	}
#ifdef DEBUG
	fprintf(stderr, "in:%d, %d\n", cbuff, numread);
#endif
	/* Update the count for this block */
	buffinf[cbuff].count = numread;
	totalread += numread;

	in_ch = getch();
	/* Was it our last block? */
	if (numread < abuf_size )
	{
	    break;
	}
	if ( in_ch == 27 || in_ch == KEY_ENTER || in_ch == 13 )
	{
	    mvprintw(ERROR_WINDOW_Y+2, ERROR_WINDOW_X+1,
		"Time:                 ");
	    move(0,79);
	    refresh();
	    break;
	}

	if(triggered)
	{
	    up(sndsemid, cbuff);

	    fsec2hmsf( (skipped + totalread - BIGBUFFSIZE - 65536.) /
		(speed * (bits/8) * (stereo+1)), tempstring);

	    mvprintw(ERROR_WINDOW_Y+2, ERROR_WINDOW_X+1,
		"Time: %s", tempstring);
	    move(0,79);
	    refresh();

/* fprintf(stderr,"\nbyte(1): %ld", skipped + totalread - BIGBUFFSIZE - 65536);
*/	}
	else
	    if(cbuff == numbuffs-1)
	    {
#ifdef DEBUG
fprintf(stderr, "Triggering (in loop)\n");
#endif
		for(i = 0; i < numbuffs; i++)
		    up(sndsemid,i);
		    triggered = 1;
	    }
	/* Update counter */
	cbuff++;
	cbuff %= numbuffs;
    }
    /* Finish off this set of buffers */
    if(terminate)
    {
	buffinf[cbuff].last = 1;
	if(!triggered)
#ifdef DEBUG
fprintf(stderr, "Triggering (after loop, partial)\n");
#endif
	    /* If it wasn't triggered, we haven't filled past cbuff */
	    for(i = 0; i < cbuff; i++)
		up(sndsemid, i);
	up(sndsemid, cbuff);
    }
    else if((!triggered) && (cbuff == numbuffs-1))
    {
#ifdef DEBUG
fprintf(stderr, "Triggering (after loop, full)\n");
#endif
	for(i = 0; i < numbuffs; i++)
	    up(sndsemid,i);
	    triggered = 1;
    }
    else if(triggered)
	up(sndsemid,cbuff);
    cbuff++;
    cbuff %= numbuffs;
}

volatile void audiowrite(void)
{
    int cbuff = 0, count, numwr, trgt;
    char *tmpptr;

    /* Uncatch the signals, so we don't clean up twice */
    signal(SIGINT, SIG_DFL);

    /* Child process writes the audio */
    while(1)
    {
	/* Wait for dirty buffer */
	down(sndsemid, cbuff);
	/* Spit to the audio device */
	if(buffinf[cbuff].setit)
	{
	    snd_parm(buffinf[cbuff].speed, buffinf[cbuff].bits,
		buffinf[cbuff].stereo);
	    buffinf[cbuff].setit = 0;
	}
	trgt = buffinf[cbuff].count;
	numwr = 0;
	tmpptr = buffarr[cbuff];
	while ( (numwr < trgt) &&
	    ((count = write(audio, tmpptr, trgt - numwr)) > 0) )
	{
	    if (count == -1)
		ErrDie("write");
	    numwr += count;
	    tmpptr += count;
	}
#ifdef DEBUG
fprintf(stderr, "out:%d, %d\n", cbuff, numwr);
#endif
	/* Was it the last buffer? */
	if (buffinf[cbuff].last)
	{
	    up(disksemid, cbuff);	/* Not really needed */
	    break;
	}
	/* Mark as clean */
	up(disksemid, cbuff);
	/* Update counter */
	cbuff++;
	cbuff %= numbuffs;
    }
    /* Tidy up and be reaped */
    sync_audio();
    close(audio);
    exit(0);
}

void initsems(int disks, int snds)
{
    int i,j;

    for (i=0;i<numsemblks;i++)
	for (j=0; j<SEMMSL;j++)
	{
	    if(semctl(disksemid[i], j, SETVAL, (union semun) disks) == -1)
		ErrDie("semctl");
	    if(semctl(sndsemid[i], j, SETVAL, (union semun) snds) == -1)
		ErrDie("semctl");
	}
}
	
void cleanupsems(void)
{
    int i;

    for (i = 0; i < numsemblks; i++)
    {
	semctl(disksemid[i], 0, IPC_RMID, (union semun) 0);
	semctl(sndsemid[i], 0, IPC_RMID, (union semun) 0);
    }
}

static void sighandler(int i)
{
#ifndef LP2CD
    fprintf(stderr, "signal %d received, cleaning up.\n", i);
#endif
    cleanupsems();
    finish_curses(1);
    exit(1);
}
