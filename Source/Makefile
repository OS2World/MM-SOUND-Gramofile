PROG = gramofile.exe

SRCS = boxes.c buttons.c checkfile.c dirfilemenu.c errorwindow.c 	\
        gramofile.c mainmenu.c reclp_filenm.c reclp_main.c scrollmenu.c	\
        stringinput.c textwindow.c yesnowindow.c clrscr.c helpline.c	\
        signpr_main.c signpr_infilenm.c signpr_outfilenm.c		\
        signpr_general.c signpr_median.c signpr_filtmenu.c signpr_wav.c	\
	secshms.c playwav.c signpr_cmf.c signpr_mean.c signpr_doubmed.c	\
	splashscr.c tracksplit.c tracksplit_filenm.c			\
        tracksplit_parammenu.c signpr_cmf2.c signpr_cmf3.c signpr_rms.c signpr_copy.c	\
        signpr_exper.c endian.c signpr_mono.c signpr_l1fit.c sun_scandir.c

OBJS = $(SRCS:.c=.o)
SHELL = /bin/sh

CC = gcc
LDFLAGS = -s -Zmt -Zcrtdll -Zbin-files

########## CHOOSE YOUR ARCHITECTURE:    (NOTE: also see bplaysrc/Makefile!)

# For Linux (and maybe others), use these:
CFLAGS = -Wall -O2 -DTURBO_MEDIAN -DTURBO_BUFFER -DOLD_CURSES
#CFLAGS = -Wall -O2 -DTURBO_MEDIAN -DTURBO_BUFFER
DEPS = $(OBJS) makebplay
LIBS = -lncurses -lrfftw -lfftw -lm
COPY_A = -a

# For FreeBSD (and maybe others), use these:
#CFLAGS = -Wall -O2 -DTURBO_MEDIAN -DTURBO_BUFFER
#DEPS = $(OBJS) makebplay
#LIBS = -lncurses  -lrfftw -lfftw -lm
#COPY_A = -p

# For IRIX (and maybe others), use these:
#CFLAGS = -Wall -O2 -DTURBO_MEDIAN -DTURBO_BUFFER -DSWAP_ENDIAN -DOLD_CURSES
#DEPS = $(OBJS)
#LIBS = -lcurses  -lrfftw -lfftw -lm
#COPY_A = -a

##########


$(PROG): $(DEPS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(PROG) $(LIBS)
	@echo ''
	@echo ''
	@echo ''
	@echo "  If you're one of those that didn't read the README, please do so now."
	@echo ''

makebplay: yesnowindow.o boxes.o buttons.o textwindow.o errorwindow.o \
           clrscr.o secshms.o
#	$(MAKE) -C bplaysrc
#	-rm bplay_gramo brec_gramo
#	cp $(COPY_A) bplaysrc/bplay ./bplay_gramo
#	ln -s bplay_gramo brec_gramo

.PHONY: clean
clean:
	$(MAKE) -C bplaysrc clean
	-rm -f gramofile bplay_gramo brec_gramo *.o *.d *~

.PHONY: indent
indent:
	indent *.c *.h

#%.d: %.c   - according to 'info make', doesn't work
#	$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $< \
#		      | sed '\''s/\($*\)\.o[ :]*/\1 $@/g'\'' > $@'
#
# 'some.o: some.c other.h'   ==> 'some some.dsome.c other.h'

%.d: %.c
	$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $< \
		      | sed '\''s/\($*\)\.o/& $@/g'\'' > $@'
#
# 'some.o: some.c other.h'   ==> 'some.o some.d: some.c other.h'  => OK

include $(SRCS:.c=.d)

