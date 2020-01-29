/*
 * Copyright (c) 2019, 4Embedded.Systems all rights reserved.
 */

#ifndef TSFILE_HPP_INCLUDED
#define TSFILE_HPP_INCLUDED

#include <cstdint>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>


typedef struct ts_packet_s
{
  uint64_t  file_offset;

  uint8_t   sync_byte;
  uint8_t   tei;              // Transport error indicator (TEI)
  uint8_t   pusi;             // Payload unit start indicator (PUSI)
  uint8_t   tsp;              // Transport priority
  uint16_t  pid;
  uint8_t   tsc;              // Transport scrambling control (TSC)
  uint8_t   afc;              // Adaptation field control
  uint8_t   cc;               // Continuity counter
} ts_packet_t;

typedef struct ts_pid_s
{
  uint16_t  pid;
  uint64_t  count;

  std::vector<ts_packet_t> packets;

} ts_pid_t;

class TsFileBase
{
  public:
    TsFileBase(std::string fileName);
    ~TsFileBase();

    bool parse(void);

    bool getTsPids(std::map<uint16_t, ts_pid_t>** pids);
    bool extractPid(uint16_t pid, std::string fileName);

  private:
    uint64_t getSize(void);

    void readTsStream(void);

    bool findTsPacketSize(void);

    void parseTsHeader(uint64_t offset, uint8_t* header_ptr, uint32_t header_size);

    bool setSeek(uint64_t offset);
    void showTsHeader(uint64_t idx, ts_packet_t* packet_ptr);

    bool writePid(uint16_t pid, std::vector<ts_packet_t>& packets, std::string fileName);

    std::string     m_filename;

    std::ifstream   m_fid;
    uint64_t        m_fSize;
    uint64_t        m_fidx;
    uint16_t        m_packet_size;
    uint8_t*        m_ts_packet;

    std::map<uint16_t, uint32_t>  m_ts_size;
    std::map<uint16_t, ts_pid_t>  m_ts_pids;

};

#endif // TSFILE_HPP_INCLUDED
