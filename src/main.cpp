/*
 * Copyright (c) 2019, 4Embedded.Systems all rights reserved.
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include "TsFile.hpp"

#define PID_EXCTRACTOR_VERSION    "0.0.1"


int getParams(int argc, char* argv[], uint16_t* pid, char** in, char** out)
{
  int itr = 0;
  int result = 2;

  while (itr < argc)
  {
    if (strcmp(argv[itr], "--pid") == 0)
    {
      itr++;
      if (itr < argc)
      {
        *pid = strtol(argv[itr], NULL, 0);
        result--;
      }
      else
      {
        break;
      }
    }
    if (strcmp(argv[itr], "--in") == 0)
    {
      itr++;
      if (itr < argc)
      {
        *in = argv[itr];
        result--;
      }
      else
      {
        break;
      }
    }
    if (strcmp(argv[itr], "--out") == 0)
    {
      itr++;
      if (itr < argc)
      {
        *out = argv[itr];
        result--;
      }
      else
      {
        break;
      }
    }
    itr++;
  }
  return result;
}

int parseTsStream(uint16_t pid, char* in, char* out)
{
  TsFileBase  tsfile(in);
  int result = 1;

  if (tsfile.parse())
  {
    result = tsfile.extractPid(pid, out ? out : "") ? 0 : 1;
  }
  return result;
}

int main(int argc, char* argv[])
{
  char* in = nullptr;
  char* out = nullptr;
  uint16_t  pid;

  int result = 1;

  printf("PidExtractor ver. %s\n", PID_EXCTRACTOR_VERSION);
  if (argc <= 1)
  {
    printf("   Usage: PidExtractor --pid <pid to extract> --in <input TS file> --out <extracted pid file>\n");
  }
  else
  {
    result = getParams(argc, argv, &pid, &in, &out);
    if (result == 0)
    {
      result = parseTsStream(pid, in, out);
    }
  }
  printf("PidExtractor DONE: %d\n", result);
  return result;
}
