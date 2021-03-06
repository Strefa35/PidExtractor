/*
 * Copyright (c) 2019, 4Embedded.Systems all rights reserved.
  */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include "TsFile.hpp"

#define PID_EXCTRACTOR_VERSION    "0.0.5"

#define UNKNOWN_PID   0xFFFF

bool getParams(int argc, char* argv[], uint16_t* pid, char** in, char** out)
{
  int itr = 0;
  bool result = false;

  while (itr < argc)
  {
    if (strcmp(argv[itr], "--pid") == 0)
    {
      itr++;
      if (itr < argc)
      {
        *pid = strtol(argv[itr], NULL, 0);
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
        result = true;
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

void showTsPids(std::map<uint16_t, ts_pid_t>& pids)
{
  uint32_t  pids_cnt = pids.size();

  printf("List of pids (%u)\n\r", pids_cnt);
  for (std::map<uint16_t,ts_pid_t>::iterator it = pids.begin(); it != pids.end(); it++)
  {
    printf("    0x%04X [%4d] - %u\n\r", it->second.pid, it->second.pid, (uint32_t) it->second.count);
  }
}

bool parseTsStream(uint16_t pid, char* in, char* out)
{
  std::string fin(in ? in : "");
  std::string fout(out ? out : "");
  TsFileBase  tsfile(fin);
  bool result = false;

  if (tsfile.parse())
  {
    if (pid <= 0x1FFF)
    {
      if (tsfile.extractPid(pid, fout))
      {
        result = true;
      }
      else
      {
        printf("No Pid: 0x%04X [%d] in the stream.\n\r", pid, pid);
      }
    }
    else
    {
      std::map<uint16_t, ts_pid_t>* pids;

      if (tsfile.getTsPids(&pids))
      {
        showTsPids(*pids);
        result = true;
      }
    }
  }
  else
  {
    printf("Wrong file name: %s\n\r", in);
  }
  return result;
}

int main(int argc, char* argv[])
{
  char* in = nullptr;
  char* out = nullptr;
  uint16_t  pid = 0xFFFF;

  bool result = false;

  printf("PidExtractor ver. %s\n\r", PID_EXCTRACTOR_VERSION);
  if (argc <= 1)
  {
    printf("   Usage: PidExtractor --pid <pid to extract> --in <input TS file> --out <extracted pid file>\n\r");
  }
  else
  {
    if (getParams(argc, argv, &pid, &in, &out))
    {
      result = parseTsStream(pid, in, out);
    }
  }
  printf("PidExtractor: %s\n\r", result ? "Ok" : "Fail");
  return result;
}
