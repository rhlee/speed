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
  pthread_t progressThread;

  if(argc == 1 ||
    (argc == 2 && (
      !strncmp(argv[1], "-h", 3) ||
      !strncmp(argv[1], "--help", 3) )))
  {
    printf("%s", usage);
    exit(0);
  }

  while(getopt(argc, argv, "f:") != -1)
  {
    if(optarg == NULL) exit(1);
    if((sscanf(optarg, "%lf", &fiddleFactor) != 1) ||
      (fiddleFactor <= 0))
    {
      printf("%s", err_argf);
      exit(1);
    }
  }

  if(argc - optind != 3)
  {
    printf("%s", usage);
    exit(1);
  }

  if((sscanf(argv[optind++], "%i/%i", &nom, &denom) != 2) ||
    (nom <= 0) || (denom <= 0))
  {
    printf("%s", usage);
    exit(1);
  }

  factor = nom / denom * fiddleFactor;
  
  if((inputFile = fopen(argv[optind++], "r")) == NULL)
    error(__LINE__, __FILE__);
  if((outputFile = fopen(argv[optind], "w")) == NULL)
    error(__LINE__, __FILE__);
    
  if(setvbuf(inputFile, NULL, _IOFBF, 4096) ||
    setvbuf(outputFile, NULL, _IOFBF, 4096))
    error(__LINE__, __FILE__);

  if(sizeof(short) != 2)
  {
    fprintf(stderr, "Short is not 2 bytes\n");
    exit(1);
  }

  printf("Press enter to see progress, ctrl-c to quit");
  if(pthread_create(&progressThread, NULL, progress, NULL) != 0)
    error(__LINE__, __FILE__);

  long inputSample = -1, outputSample = 0;
  double inputTime;
  short lowerInputChannel[2], upperInputChannel[2], outputChannel[2];
  for(outputSample = 0; 1; outputSample++)
  {
    inputTime = factor * (outputSample + 0.5);
    inputLowerSample = floor(inputTime - 0.5);
    while(inputSample < (inputLowerSample + 1))
    {
      lowerInputChannel[0] = upperInputChannel[0];
      lowerInputChannel[1] = upperInputChannel[1];
      if(fread(upperInputChannel, 2, 2, inputFile) != 2)
        exit(0);
      inputSample++;
    }
    outputChannel[0] =
      ((upperInputChannel[0] - lowerInputChannel[0]) *
        (inputTime - inputLowerSample - 0.5)) + lowerInputChannel[0];
    outputChannel[1] =
      ((upperInputChannel[1] - lowerInputChannel[1]) *
        (inputTime - inputLowerSample - 0.5)) + lowerInputChannel[1];
    fwrite(outputChannel, 2, 2, outputFile);
  }
}

void *progress(void *ptr)
{
  char c;
  while(c = getchar())
  {
    if(c == '\n')
      printf("Processed %.1fM", inputLowerSample / 262144.0);
  }
  return NULL;
}

void error(int line, char * file)
{
  printf("[%s:%i] Last set error code is %i: %s\n"
    "Use gdb to catch this SIGTRAP\n",
    file, line, errno, strerror(errno));
  __asm__("int3");
  exit(errno);
}
