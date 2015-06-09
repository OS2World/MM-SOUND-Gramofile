# Modified for use with GramoFile, June 1998, J.A. Bezemer

PROG = bplay
OBJS = bplay.o sndfunc.o shmbuf.o ../yesnowindow.o ../boxes.o \
        ../buttons.o ../textwindow.o ../errorwindow.o \
        ../clrscr.o ../secshms.o
#SRCS = bplay.c sndfunc.o shmbuf.c

CC = gcc
LDFLAGS = 

########## CHOOSE YOUR ARCHITECTURE:    (NOTE: also see ../Makefile!)

# For Linux (and maybe others), use these:
CFLAGS = -Wall -O2 -DUSEBUFFLOCK -DREDHAT50 -DLP2CD -DVUMETER # -DDEBUG
LIBS = -lncurses

# For FreeBSD (and maybe others), use these:
#CFLAGS = -Wall -O2 -DLP2CD -DVUMETER # -DDEBUG
#LIBS = -lncurses

# -DOLD_CURSES can be used here, too.

##########

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(PROG) $(LIBS)
	ln -sf bplay brec

clean:
	rm -f bplay brec *.o
