/*
 * Copyright (c) 2019, 4Embedded.Systems all rights reserved.
 */

#include "TsDbg.hpp"

#define TS_DBG_BUFFER_MAX     256
#define TS_DBG_COLUMNS        16


static uint8_t    ts_dbg_buffer[TS_DBG_BUFFER_MAX];


void DbgMemory(const uint8_t* data_ptr, const uint32_t data_len)
{
  char* line  = (char*) ts_dbg_buffer;
  char* ascii = (char*) &ts_dbg_buffer[TS_DBG_BUFFER_MAX / 2];
  uint32_t stuffing = ((data_len % TS_DBG_COLUMNS) * TS_DBG_COLUMNS) - data_len;

  line[0] = 0;
  ascii[0] = 0;
  for (uint32_t data_idx = 0; data_idx < data_len; data_idx++)
  {
    // insert HEX
    {
      sprintf(line, "%s %02X", line, data_ptr[data_idx]);
      sprintf(ascii, "%s %c", ascii, ((data_ptr[data_idx] >= 32) && (data_ptr[data_idx] <= 127)) ? data_ptr[data_idx] : '.');
    }
    // end HEX line
    if (data_idx && ((data_idx + 1) % TS_DBG_COLUMNS) == 0)
    {
      DbgWrite("[%8d]\t%s\t%s\n", (data_idx + 1) - TS_DBG_COLUMNS, line, ascii);
      line[0] = 0;
      ascii[0] = 0;
    }
  }

  if (stuffing)
  {
    for (uint32_t idx = 0; idx < stuffing; idx++)
    {
      sprintf(line, "%s %s", line, "  ");
      sprintf(ascii, "%s %c", ascii, ' ');
    }
    DbgWrite("[%8d]\t%s\t%s\n", data_len - TS_DBG_COLUMNS, line, ascii);
  }
}
