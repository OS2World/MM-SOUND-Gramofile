/* Signal Processing - Wave File Functions

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_wav.h"
#include "fmtheaders.h"
#include "errorwindow.h"
#include "signpr_general.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "endian.h"

/* ----- SOURCE & READING -------------------------------------------------- */

FILE *sourcefile;

int
openwavsource (char *filename)
/* returns 0: failure (error_window has been displayed), 1: success.
   More or less adapted from bplay.c, with stdio-patch by Axel Kohlmeyer
 */
{
  int count;
  char hd_buf[20];
  wavhead wavhd;

  if ((sourcefile = fopen (filename, "rb")) == NULL)
    {
      error_window ("The source file could not be opened.");
      return 0;
    }

  count = fread (hd_buf, 1, 20, sourcefile);
  if (count < 20)
    {
      fclose (sourcefile);
      error_window ("The source file could not be read, or is too short.");
      return 0;
    }

  if (strstr (hd_buf, "RIFF") == NULL)
    {
      fclose (sourcefile);
      error_window ("The source file is not a .wav file, and can not be \
processed.");
      return 0;
    }

  memcpy ((void *) &wavhd, (void *) hd_buf, 20);
  count = fread (((char *) &wavhd) + 20, 1, sizeof (wavhd) - 20, sourcefile);

  if (count < sizeof (wavhd) - 20)
    {
      fclose (sourcefile);
      error_window ("The source file is too short. Probably it is not \
a .wav sound file.");
      return 0;
    }

#ifdef SWAP_ENDIAN
  wavhd.format = SwapTwoBytes (wavhd.format);
  wavhd.sample_fq = SwapFourBytes (wavhd.sample_fq);
  wavhd.bit_p_spl = SwapTwoBytes (wavhd.bit_p_spl);
  wavhd.modus = SwapTwoBytes (wavhd.modus);
#endif

  if (wavhd.format != 1)
    {
      fclose (sourcefile);
      error_window ("The source file is a .wav file with unknown format, \
and can not be processed.");
      return 0;
    }

  if (wavhd.sample_fq != 44100)
    {
      fclose (sourcefile);
      error_window ("The source file is not sampled at 44100 Hz, and can \
not be processed.");
      return 0;
    }

  if (wavhd.bit_p_spl != 16)
    {
      fclose (sourcefile);
      error_window ("The source file does not have 16 bit samples, and \
can not be processed.");
      return 0;
    }

  if (wavhd.modus != 2)
    {
      fclose (sourcefile);
      error_window ("The source file is not stereo, and can not be \
processed.");
      return 0;
    }

  /* Well, everything seems to be OK */
  return 1;
}

void
closewavsource ()
{
  fclose (sourcefile);
}

int
seeksamplesource (long samplenumber)
/* returns 0: failure, 1: success */
{
  struct stat buf;

  if (fstat (fileno (sourcefile), &buf))
    return 0;

  if (sizeof (wavhead) + 2 * 2 * samplenumber >= buf.st_size)
    return 0;

  if (fseek (sourcefile, sizeof (wavhead) + 2 * 2 * samplenumber, SEEK_SET) > -1)
    return 1;
  else
    return 0;
}

sample_t
readsamplesource ()
{
  sample_t sample;

  if (fread (&sample, 1, 4, sourcefile) != 4)
    {
      /* reading after end of file - this just happens when using
         pre-read buffers! */
      sample.left = 0;
      sample.right = 0;
    }

#ifdef SWAP_ENDIAN
  sample = SwapSample (sample);
#endif

  return sample;
}


/* ----- DESTINATION & WRITING --------------------------------------------- */

FILE *destfile;
int destfileispipe;		/* remember open() - - -> close() */

int
openwavdest (char *filename, long bcount)
/* returns 0: failure (error_window has NOT been displayed), 1: success.
   bcount must be 4 * number_of_samples !
   Adapted from bplay.c
 */
{
  wavhead header;
  char *riff = "RIFF";
  char *wave = "WAVE";
  char *fmt = "fmt ";
  char *data = "data";

  if (*filename == '|')
    {
      destfileispipe = 1;	/* remember for closing */
      if ((destfile = popen (filename + 1, "w")) == NULL)
	return 0;
    }
  else
    {
      destfileispipe = 0;
      if ((destfile = fopen (filename, "wb")) == NULL)
	return 0;
    }


  memcpy (&(header.main_chunk), riff, 4);
  header.length = sizeof (wavhead) - 8 + bcount;
  memcpy (&(header.chunk_type), wave, 4);

  memcpy (&(header.sub_chunk), fmt, 4);
  header.sc_len = 16;
  header.format = 1;
  header.modus = /*stereo */ 1 + 1;
  header.sample_fq = /*speed */ 44100;
  header.byte_p_sec = /*((bits > 8)? 2:1)*(stereo+1)*speed */ 2 * 2 * 44100;
  header.byte_p_spl = /*(bits > 8)? 2:1 */ 4;	/* stereo & 16 bit..? */
  header.bit_p_spl = /*bits */ 16;	/* stereo doesn't count here? */

  memcpy (&(header.data_chunk), data, 4);
  header.data_length = bcount;

#ifdef SWAP_ENDIAN
  header.length = SwapFourBytes (header.length);
  header.sc_len = SwapFourBytes (header.sc_len);
  header.format = SwapTwoBytes (header.format);
  header.modus = SwapTwoBytes (header.modus);
  header.sample_fq = SwapFourBytes (header.sample_fq);
  header.byte_p_sec = SwapFourBytes (header.byte_p_sec);
  header.byte_p_spl = SwapTwoBytes (header.byte_p_spl);
  header.bit_p_spl = SwapTwoBytes (header.bit_p_spl);
  header.data_length = SwapFourBytes (header.data_length);
#endif

  fwrite (&header, sizeof (header), 1, destfile);

  return 1;
}

void
closewavdest ()
{
  if (destfileispipe)
    pclose (destfile);
  else
    fclose (destfile);
}

void
writesampledest (sample_t sample)
{

#ifdef SWAP_ENDIAN
  sample = SwapSample (sample);
#endif

  fwrite (&sample, 4, 1, destfile);
}
