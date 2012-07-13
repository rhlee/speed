#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

void error(int line, char * file);

void *progress(void *ptr);

long inputLowerSample;

int main(int argc, char *argv[])
{
  FILE *inputFile, *outputFile;
  pthread_t progressThread;
  double factor = 1001.0 / 960.0;
  factor *= (1 + (0.5 / (90.0 * 60.0)));
  
  if((inputFile = fopen("sample.wav", "r")) == NULL)
    error(__LINE__, __FILE__);
  if((outputFile = fopen("test.wav", "w")) == NULL)
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
