long inputSample = -1, outputSample = 0;
double inputTime;
sample lowerInputChannel[2], upperInputChannel[2], outputChannel[2];

for(outputSample = 0; 1; outputSample++)
{
  inputTime = factor * (outputSample + 0.5);
  inputLowerSample = floor(inputTime - 0.5);
  while(inputSample < (inputLowerSample + 1))
  {
    lowerInputChannel[0] = upperInputChannel[0];
    lowerInputChannel[1] = upperInputChannel[1];
    if(fread(upperInputChannel, sizeof(sample), 2, inputFile) != 2)
      exit(0);
    inputSample++;
  }
  outputChannel[0] =
    ((upperInputChannel[0] - lowerInputChannel[0]) *
      (inputTime - inputLowerSample - 0.5)) + lowerInputChannel[0];
  outputChannel[1] =
    ((upperInputChannel[1] - lowerInputChannel[1]) *
      (inputTime - inputLowerSample - 0.5)) + lowerInputChannel[1];
  fwrite(outputChannel, sizeof(sample), 2, outputFile);
}
