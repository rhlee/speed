#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>

long inputSampleFloor;

void sigInt(int signal);

int main(int argc, char *argv[])
{
  double factor = (double)1001 / (double)960;
  
  FILE *inputFile, *outputFile;

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
  double inputTime;
  short channel[2], previousChannel[2];
  for(outputSample = 0; 1; outputSample++)
  {
    inputTime = factor * ((long)outputSample + 0.5);
    inputSampleFloor = floor(inputTime);
    while(inputSample < inputSampleFloor)
    {
      if(fread(channel, 2, 2, inputFile) != 2)
      {
        fclose(inputFile);
        fclose(outputFile);
        exit(0);
      }
      previousChannel[0] = channel[0];
//      previousChannel[1] = channel[1];
      inputSample++;
    }
    fwrite(channel, 2, 2, outputFile);
//    printf("%ld\n", inputSample);
  }

  fclose(inputFile);
  fclose(outputFile);
  return 0;
}

void sigInt(int signal)
{
  fprintf(stderr, "hello\n");
  exit(0);
}
