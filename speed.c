#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

  long inputSample = -1, outputSample = 0, inputSampleFloor;
  double inputTime;
  factor = 2;
  for(outputSample = 0; outputSample < 10; outputSample++)
  {
    inputTime = factor * ((long)outputSample + 0.5);
    printf("%f\n", inputTime);
  }

  fclose(inputFile);
  fclose(outputFile);
  return 0;
}
