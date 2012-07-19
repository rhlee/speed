sample lowerInputChannel[2], upperInputChannel[2], outputChannel[2];

while(!complete)
{
  inputTime = factor * (outputSample + 0.5);
  inputLowerSample = floor(inputTime - 0.5);
  while(inputSample < (inputLowerSample + 1))
  {
    lowerInputChannel[0] = upperInputChannel[0];
    lowerInputChannel[1] = upperInputChannel[1];
    if(fread(upperInputChannel, sizeof(sample), 2, inputFile) != 2)
    {
      complete = 1;
      break;
    }

    if(inputSample > percentThreshold)
    {
      bars = round(percent / 2.0);
      printf("[");
      for(i = 0; i < bars; i++) printf("=");
      for(i = bars; i < 50; i++) printf("-");
      printf("] %i%%\n", percent);
      fflush(stdout);
      percentThreshold = inputSamples * (percent + 0.5) / 100;
      percent++;
    }
    
    inputSample++;
  }
  outputChannel[0] =
    ((upperInputChannel[0] - lowerInputChannel[0]) *
      (inputTime - inputLowerSample - 0.5)) + lowerInputChannel[0];
  outputChannel[1] =
    ((upperInputChannel[1] - lowerInputChannel[1]) *
      (inputTime - inputLowerSample - 0.5)) + lowerInputChannel[1];
  fwrite(outputChannel, sizeof(sample), 2, outputFile);

  outputSample++;
}
