#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

char usage[] =
  "Usage:\n";
char err_argf[] =
  "f has to be summin\n";

void error(int line, char * file);

void *progress(void *ptr);

long inputLowerSample;

int main(int argc, char *argv[])
{
  double fiddleFactor = 1, factor;
  int nom, denom;
  FILE *inputFile, *outputFile;

  if(argc == 1 ||
    (argc == 2 && (
      !strncmp(argv[1], "-h", 3) ||
      !strncmp(argv[1], "--help", 3) )))
  {
    printf("%s", usage);
    exit(0);
  }

  int optCount = 0;
  char o;
  int infoMode = 0;
  char input[256];
  while((o = getopt(argc, argv, "f:i:")) != -1)
  {
    if(optarg == NULL) exit(1);
    switch(o)
    {
      case 'f':
	if((sscanf(optarg, "%lf", &fiddleFactor) != 1) ||
	  (fiddleFactor <= 0))
	{
	  printf("%s", err_argf);
	  exit(1);
	}
        break;
      case 'i':
	strncpy(input, optarg, 256);
	infoMode = 1;
	break;
      default:
        abort();
    }
    optCount++;
  }
  if(optCount > 1)
  {
    printf("%s", usage);
    exit(0);
  }

  if(!infoMode) strncpy(input, argv[optind + 1], 256);

  if((inputFile = fopen(input, "r")) == NULL)
    error(__LINE__, __FILE__);

  int ifd = fileno(inputFile);
  int intDump;
  char strDump[5];
  
  if(read(ifd, strDump, 4) != 4)
    error(__LINE__, __FILE__);
  strDump[4] = 0;
  if(strcmp(strDump, "RIFF"))
  {
    printf("Can't find RIFF string.\n", strDump);
    exit(1);
  }
  
  if(read(ifd, &intDump, 4) != 4)
    error(__LINE__, __FILE__);
  printf("RIFF size: %u\n", intDump);
  
  if(lseek(ifd, 0xc, SEEK_SET) == -1)
    error(__LINE__, __FILE__);
  if(read(ifd, strDump, 4) != 4)
    error(__LINE__, __FILE__);
  strDump[4] = 0;
  if(strcmp(strDump, "fmt "))
  {
    printf("Can't find fmt string.\n", strDump);
    exit(1);
  }
  
  short shortDump, bps;
  if(lseek(ifd, 0x16, SEEK_SET) == -1)
    error(__LINE__, __FILE__);
  if(read(ifd, &shortDump, 2) != 2)
    error(__LINE__, __FILE__);
  if(shortDump == 2)
    printf("channels: 2\n");
  else
  {
    printf("Only stereo channels are supported.\n", strDump);
    exit(1);
  }
  
  if(lseek(ifd, 0x22, SEEK_SET) == -1)
    error(__LINE__, __FILE__);
  if(read(ifd, &bps, 2) != 2)
    error(__LINE__, __FILE__);
  if((bps == 16) || (bps == 32))
    printf("bps: %i\n", bps);
  else
  {
    printf("Only 16 and 32 bps are supported.\n", strDump);
    exit(1);
  }

  if(lseek(ifd, 0x28, SEEK_SET) == -1)
    error(__LINE__, __FILE__);
  if(read(ifd, &intDump, 4) != 4)
    error(__LINE__, __FILE__);
  printf("DATA size: %u\n", intDump);
  int inputSamples = intDump / 4;
  if(bps == 32) inputSamples /= 2;
  
  if(infoMode) exit(0);

  lseek(ifd, 0, SEEK_SET);
  
  if(argc - optind != 3)
  {
    printf("%s", usage);
    exit(1);
  }

  if((sscanf(argv[optind], "%i/%i", &nom, &denom) != 2) ||
    (nom <= 0) || (denom <= 0))
  {
    printf("%s", usage);
    exit(1);
  }

  factor = (double)nom / (double)denom * fiddleFactor;
  
  if((outputFile = fopen(argv[optind + 2], "w")) == NULL)
    error(__LINE__, __FILE__);
    
  if(setvbuf(inputFile, NULL, _IOFBF, 4096) ||
    setvbuf(outputFile, NULL, _IOFBF, 4096))
    error(__LINE__, __FILE__);

  if(bps == 16)
  {
    #define sample short
    #include "macro.c"
  }
  else
  {
    #undef sample
    #define sample int
    #include "macro.c"
  }
}

void error(int line, char * file)
{
  printf("[%s:%i] Last set error code is %i: %s\n"
    "Use gdb to catch this SIGTRAP\n",
    file, line, errno, strerror(errno));
  __asm__("int3");
  exit(errno);
}
