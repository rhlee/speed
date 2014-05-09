#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>


#define CASSERT(expn) typedef char __C_ASSERT__[(expn)?1:-1]


#define WAVE_FORMAT_PCM		0x0001
#define WAVE_FORMAT_IEEE_FLOAT	0x0003
#define WAVE_FORMAT_EXTENSIBLE	0xfffe

#define RIFF_CHUNK_SIZE 0xc
#define FMT_CHUNK_SIZE 0x18
#define EXTENSION_SIZE_SIZE 0x2
#define EXTENSION_CHUNK_SIZE 0x16
#define DATA_CHUNK_HEADER_SIZE 0x8


struct __attribute__((packed)) riff {
  char chunkID[4];
  uint32_t chunkSize;
  char format[4];
};
CASSERT(sizeof(struct riff) == RIFF_CHUNK_SIZE);

struct __attribute__((packed)) fmt {
  char chunkID[4];
  uint32_t chunkSize;
  uint16_t format;
  uint16_t channels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
};
CASSERT(sizeof(struct fmt) == FMT_CHUNK_SIZE);

struct __attribute__((packed)) extension {
  uint16_t validBitsPerSample;
  uint32_t channelMask;
  uint16_t guid2;
  char guid14[14];
};
CASSERT(sizeof(struct extension) == EXTENSION_CHUNK_SIZE);

struct __attribute__((packed)) data {
  char chunkID[4];
  uint32_t chunkSize;
};
CASSERT(sizeof(struct data) == DATA_CHUNK_HEADER_SIZE);

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

  struct riff riff;
  if(read(fileno(inputFile), &riff, RIFF_CHUNK_SIZE) != RIFF_CHUNK_SIZE)
    error(__LINE__, __FILE__);
  
  if(strncmp(riff.chunkID, "RIFF", 04))
  {
    printf("Can't find RIFF string.\n");
    exit(1);
  }

  printf("RIFF size: %u\n", riff.chunkSize);
  
  if(strncmp(riff.format, "WAVE", 04))
  {
    printf("Can't find EAVE string.\n");
    exit(1);
  }

  struct fmt fmt;
  if(read(fileno(inputFile), &fmt, FMT_CHUNK_SIZE) != FMT_CHUNK_SIZE)
    error(__LINE__, __FILE__);

  if(strncmp(fmt.chunkID, "fmt ", 04))
  {
    printf("Can't find fmt string\n");
    exit(1);
  }

  short format;
  struct extension extension;

  switch (fmt.format) {
    case WAVE_FORMAT_PCM:
      printf("encoding: PCM\n");
      format = fmt.format;
      break;
    case WAVE_FORMAT_IEEE_FLOAT:
      printf("encoding: float\n");
      format = fmt.format;
      break;
    case WAVE_FORMAT_EXTENSIBLE:
      printf("encoding: extended\n");
      uint16_t extensionSize;
      if(read(fileno(inputFile), &extensionSize, EXTENSION_SIZE_SIZE) != \
          EXTENSION_SIZE_SIZE)
        error(__LINE__, __FILE__);
      if (extensionSize != 0x16)
      {
        printf("Invalid extension size\n");
        exit(1);
      }
      if(read(fileno(inputFile), &extension, EXTENSION_CHUNK_SIZE) != \
          EXTENSION_CHUNK_SIZE)
        error(__LINE__, __FILE__);
      format = extension.guid2;
      if(strncmp(extension.guid14,
        "\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71", 14))
      {
        printf("Can't find GUID string\n");
        exit(1);
      }
      switch (format) {
        case WAVE_FORMAT_PCM:
          printf("extended encoding: PCM\n");
          break;
        case WAVE_FORMAT_IEEE_FLOAT:
          printf("extended encoding: float\n");
          break;
        default:
          printf("Extended encoding not supported.\n");
          exit(1);
          break;
        }
      break;
    default:
      printf("Encoding not supported.\n");
      exit(1);
      break;
  }
  
  if(fmt.channels == 2)
    printf("channels: 2\n");
  else
  {
    printf("Only stereo channels are supported.\n");
    exit(1);
  }

  if((fmt.bitsPerSample == 16) || (fmt.bitsPerSample == 32))
    printf("bps: %i\n", fmt.bitsPerSample);
  else
  {
    printf("Only 16 and 32 bps are supported.\n");
    exit(1);
  }

  struct data data;

  if(read(fileno(inputFile), &data, DATA_CHUNK_HEADER_SIZE) != \
      DATA_CHUNK_HEADER_SIZE)
    error(__LINE__, __FILE__);
  if(strncmp(data.chunkID, "data", 04))
  {
    printf("Can't find data string\n");
    exit(1);
  }
  unsigned int inputSamples = data.chunkSize / 4;
  if(fmt.bitsPerSample == 32) inputSamples /= 2;
  printf("DATA size: %u\n", data.chunkSize);
  
  if(infoMode) exit(0);

  lseek(fileno(inputFile), 0, SEEK_SET);
  
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

  int totalHeaderSize = RIFF_CHUNK_SIZE + FMT_CHUNK_SIZE + \
    (fmt.format == WAVE_FORMAT_EXTENSIBLE ? \
      EXTENSION_SIZE_SIZE + EXTENSION_CHUNK_SIZE : 0) + \
    DATA_CHUNK_HEADER_SIZE;

  int ofd = fileno(outputFile);
  if(lseek(ofd, totalHeaderSize, SEEK_SET) == -1)
    error(__LINE__, __FILE__);

  long inputSample = -1, outputSample = 0;
  double inputTime;
  int percent = 0, percentThreshold = -1, bars, i, complete = 0;

  if(format == 3)
  {
    #define sample float
    #include "macro.c"
  }
  else if(fmt.bitsPerSample == 16)
  {
    #undef sample
    #define sample short
    #include "macro.c"
  }
  else
  {
    #undef sample
    #define sample int
    #include "macro.c"
  }

  fflush(outputFile);
  
  /*int outputDataSize = outputSample * (bps == 16 ? 4 : 8),
    outputRiffSize = outputDataSize + 36;
  printf("RIFF size: %i\n", outputRiffSize);
  printf("DATA size: %i\n", outputDataSize);
  
  unsigned char *headerByte;
  headerByte = &header[04];
  *(headerByte++) = (outputRiffSize >> 000) & 0xff;
  *(headerByte++) = (outputRiffSize >> 010) & 0xff;
  *(headerByte++) = (outputRiffSize >> 020) & 0xff;
  *(headerByte++) = (outputRiffSize >> 030) & 0xff;

  headerByte = &header[050];
  *(headerByte++) = (outputDataSize >> 000) & 0xff;
  *(headerByte++) = (outputDataSize >> 010) & 0xff;
  *(headerByte++) = (outputDataSize >> 020) & 0xff;
  *(headerByte++) = (outputDataSize >> 030) & 0xff;*/

  data.chunkSize = outputSample * (fmt.bitsPerSample == 16 ? 4 : 8);
  riff.chunkSize = data.chunkSize + totalHeaderSize - 8;
  
  if(lseek(ofd, 00, SEEK_SET) != 00)
    error(__LINE__, __FILE__);

  if(fwrite(&riff, sizeof(unsigned char), RIFF_CHUNK_SIZE, outputFile) != \
      RIFF_CHUNK_SIZE)
    error(__LINE__, __FILE__);
  if(fwrite(&fmt, sizeof(unsigned char), FMT_CHUNK_SIZE, outputFile) != \
      FMT_CHUNK_SIZE)
    error(__LINE__, __FILE__);
  if(fmt.format == WAVE_FORMAT_EXTENSIBLE) {
    uint16_t extension_chunk_size_data = EXTENSION_CHUNK_SIZE;
    if(fwrite(&extension_chunk_size_data, sizeof(unsigned char), \
          EXTENSION_SIZE_SIZE,  outputFile) != \
        EXTENSION_SIZE_SIZE)
      error(__LINE__, __FILE__);
    if(fwrite(&extension, sizeof(unsigned char), EXTENSION_CHUNK_SIZE, \
          outputFile) != \
        EXTENSION_CHUNK_SIZE)
      error(__LINE__, __FILE__);
  }
  if(fwrite(&data, sizeof(unsigned char), DATA_CHUNK_HEADER_SIZE, \
        outputFile) != \
      DATA_CHUNK_HEADER_SIZE)
    error(__LINE__, __FILE__);
  
  fclose(inputFile);
  fclose(outputFile);
  
  return 0;
}

void error(int line, char * file)
{
  printf("[%s:%i] Last set error code is %i: %s\n"
    "Use gdb to catch this SIGTRAP\n",
    file, line, errno, strerror(errno));
  __asm__("int3");
  exit(errno);
}
