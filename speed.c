#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>

FILE *inputFile, *outputFile;
long inputLowerSample;
long double lastInterrupt;

void sigInt(int signal);
void finally();

int main(int argc, char *argv[])
{
  double factor = 1001.0 / 960.0;
  factor *= (1 + (0.5 / (90.0 * 60.0)));
  
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
  short lowerInputChannel[2], upperInputChannel[2], outputChannel[2];
  for(outputSample = 0; 1; outputSample++)
  {
    inputTime = factor * (outputSample + 0.5);
    inputLowerSample = floor(inputTime - 0.5);
    while(inputSample < (inputLowerSample + 1))
    {
      lowerInputChannel[0] = upperInputChannel[0];
      lowerInputChannel[1] = upperInputChannel[1];
      if(fread(upperInputChannel, 2, 2, inputFile) != 2) finally();
      inputSample++;
    }
    outputChannel[0] = ((upperInputChannel[0] - lowerInputChannel[0]) *
      (inputTime - inputLowerSample - 0.5)) + lowerInputChannel[0];
    outputChannel[1] = ((upperInputChannel[1] - lowerInputChannel[1]) *
      (inputTime - inputLowerSample - 0.5)) + lowerInputChannel[1];
    fwrite(outputChannel, 2, 2, outputFile);
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

  fprintf(stderr, "\nProcessed %.1fM (Press CTRL+C twice to exit)\n",
    inputLowerSample / 262144.0);
}

void finally()
{
  fclose(inputFile);
  fclose(outputFile);
  exit(0);
}
