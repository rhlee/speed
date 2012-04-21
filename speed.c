#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>

FILE *inputFile, *outputFile;
long inputSampleFloor;
long double lastInterrupt;

void sigInt(int signal);
void finally();

int main(int argc, char *argv[])
{
  long double factor = 1001.0 / 960.0 * (1 + (0.5 / (90.0 * 60.0)));
  
  inputFile = fopen("sample.wav", "r");
  outputFile = fopen("test.wav", "w");
  if(setvbuf(inputFile, NULL, _IOFBF, 4096) ||
    setvbuf(outputFile, NULL, _IOFBF, 4096))
  {
    perror("Could not set file buffer\n");
    exit(1);
  }

  if(sizeof(short) != 2)
  {
    perror("Short is not 2 bytes\n");
    exit(1);
  }

  signal(SIGINT, sigInt);

  long inputSample = -1, outputSample = 0;
  long double inputTime;
  short channel[2], previousChannel[2];
  for(outputSample = 0; 1; outputSample++)
  {
    inputTime = factor * ((long double)outputSample + 0.5);
    inputSampleFloor = floor(inputTime);
    while(inputSample < inputSampleFloor)
    {
      if(fread(channel, 2, 2, inputFile) != 2) finally();
      previousChannel[0] = channel[0];
//      previousChannel[1] = channel[1];
      inputSample++;
    }
    fwrite(channel, 2, 2, outputFile);
//    printf("%ld\n", inputSample);
  }

  finally();
}

void sigInt(int signal)
{
  struct timeval time;
  gettimeofday(&time, NULL);
  long double interrupt = time.tv_sec + (time.tv_usec / 1000000.0);
  if((interrupt - lastInterrupt) < 1.0) finally();
  lastInterrupt = interrupt;

  fprintf(stderr, "\nProcessed %.1fM (Press CTRL+C twice to exit)\n", inputSampleFloor / 1048576.0);
}

void finally()
{
  fclose(inputFile);
  fclose(outputFile);
  exit(0);
}
