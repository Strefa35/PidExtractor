/*
 * Copyright (c) 2019, 4Embedded.Systems all rights reserved.
  */

#include <cstdio>
#include <cstring>
#include <string>
#include <cassert>
#include <sstream>
#include <iomanip>

#include "TsFile.hpp"

#include "options.dbg"
#ifdef DBG_LVL_TSFILE
  #define DBG_LEVEL   DBG_LVL_TSFILE
#endif // DBG_LVL_TSFILE
#include "TsDbg.hpp"


#define TS_HEADER_SIZE      (4)
#define TS_PACKET_SIZE      (188)
#define TS_BUFFER           (TS_PACKET_SIZE + 48) // + 48 bytes of safe area

#define TS_SYNC_BYTE        0x47

#define TS_AFC_PAYLOAD      (0x1)
#define TS_AFC_ADAPTATION   (0x2)


TsFileBase::TsFileBase(std::string& fileName) :
      m_packet_size(TS_PACKET_SIZE),
      m_ts_packet(nullptr)
{
  DBGS(DbgWrite("++%s(fileName: %s)\n", __func__,
          fileName.empty() ? "-" : fileName.c_str());)

  m_fSize = 0;
  m_filename = fileName;

  m_fid.open(m_filename, std::ifstream::in | std::ifstream::binary);
  if (m_fid.is_open())
  {
    if (getSize())
    {
    }
  }
  DBGR(DbgWrite("--%s()\n", __func__);)
}

TsFileBase::~TsFileBase()
{
  DBGS(DbgWrite("++%s()\n", __func__);)
  if (m_ts_packet)
  {
    delete[] m_ts_packet;
  }
  if (m_fid.is_open())
  {
    m_fid.close();
  }
  DBGR(DbgWrite("--%s()\n", __func__);)
}

uint64_t TsFileBase::getSize(void)
{
  DBGS(DbgWrite("++%s()\n", __func__);)
  m_fid.seekg(0, m_fid.end);
  m_fSize = (uint64_t) m_fid.tellg();
  DBG1(DbgWrite("[%s] m_fSize: %u\n", __func__, m_fSize);)
  m_fid.seekg(0, m_fid.beg);
  DBGR(DbgWrite("--%s()\n", __func__);)
  return m_fSize;
}

bool TsFileBase::setSeek(uint64_t offset)
{
  bool result = false;

  DBGS(DbgWrite("++%s(offset: %lu)\n", __func__, offset);)
  if (offset < m_fSize)
  {
    m_fid.seekg(offset, m_fid.beg);
    result = true;
  }
  DBGR(DbgWrite("--%s() - result: %d\n", __func__, result);)
  return result;
}

bool TsFileBase::findTsPacketSize(void)
{
  bool      ts_sync_found = false;
  bool      ts_error = false;
  uint64_t  first_packet = 0;
  uint64_t  current_idx = 0;
  uint64_t  packet_cnt = 0;
  uint16_t  packet_size = TS_PACKET_SIZE;
  uint16_t  last_packet_size = 0;
  uint8_t   one_byte;

  DBGS(DbgWrite("++%s()\n", __func__);)
  setSeek(0);
  while(!m_fid.eof() && ((current_idx + TS_PACKET_SIZE) <= m_fSize))
  {
    DBG1(DbgWrite("\tcurrent_idx: %d\n", current_idx);)
    setSeek(current_idx);
    m_fid.read((char*) &one_byte, 1);
    if (one_byte == TS_SYNC_BYTE)
    {
      DBG1(DbgWrite("\tTS SYNC BYTE - %u\n", current_idx);)
      packet_cnt++;
      if (ts_sync_found)
      {
        if (last_packet_size && (packet_size != (current_idx - first_packet)))
        {
          last_packet_size = current_idx - first_packet;
          ts_error = true;
          DBGW(DbgWrite("Packet size is different: %d <--> %d\n", packet_size, last_packet_size);)
          break; // We have to STOP analysing due to different packet's size in the stream
        }
        last_packet_size = current_idx - first_packet;
        if (last_packet_size != packet_size)
        {
          packet_size = last_packet_size;
        }
        m_ts_size[packet_size]++;
        DBG1(DbgWrite("\tpacket_size: %d\n", packet_size);)
      }
      else
      {
        ts_sync_found = true;
      }
      first_packet = current_idx;
      current_idx += packet_size;
      if (current_idx >= m_fSize)
      {
        m_ts_size[m_fSize - first_packet]++;
      }
    }
    else
    {
      current_idx++;
    }
    printf("Checking packets: %d%%\r", (uint8_t) ((100 * current_idx) / m_fSize));
  }

  printf("TS file checked: ");
  if (!ts_error)
  {
    printf("OK    \n\r");
    printf("  Packet count: %lu\n\r", packet_cnt);
    printf("   Packet size: %d\n\r", packet_size);

    m_packet_size = packet_size;

  }
  else
  {
    printf("FAIL\n\r");
    printf("  Stream has wrong packet size: %d <--> %d\n\r", packet_size, last_packet_size);
    m_packet_size = 0;
  }

  printf("Packets:\n\r");
  for (std::map<uint16_t, uint32_t>::iterator it = m_ts_size.begin(); it != m_ts_size.end(); it++)
  {
    printf("    %3d - %d\n\r", it->first, it->second);
  }
  DBGR(DbgWrite("--%s() - packet_size: %d, last_packet_size: %d\n", __func__, packet_size, last_packet_size);)
  return !ts_error;
}

void TsFileBase::readTsStream(void)
{
  uint8_t  header[TS_HEADER_SIZE];
  uint16_t packet_size = m_packet_size;
  uint64_t current_idx = 0;

  DBGS(DbgWrite("++%s()\n", __func__);)
  setSeek(0);
  while(!m_fid.eof() && ((current_idx + packet_size) <= m_fSize))
  {
    DBG1(DbgWrite("[%s] idx: %lu\n", __func__, current_idx);)
    setSeek(current_idx);
    m_fid.read((char*) &header[0], 1);
    if (header[0] == TS_SYNC_BYTE)
    {
      m_fid.read((char*) &header[1], TS_HEADER_SIZE - 1);
      parseTsHeader(current_idx, header, TS_HEADER_SIZE);
      current_idx += packet_size;
    }
    else
    {
      current_idx++;
    }
    printf("Reading packets: %d%%\r", (uint8_t) ((100 * current_idx) / m_fSize));
  }
  printf("Reading packets: DONE\n\r");
  DBGR(DbgWrite("--%s()\n", __func__);)
}

void TsFileBase::parseTsHeader(uint64_t offset, uint8_t* header_ptr, uint32_t header_size)
{
  ts_packet_t packet;

  DBGS(DbgWrite("++%s(offset: %lu, header: 0x %02x %02x %02x %02x)\n", __func__,
            offset,
            header_ptr[0], header_ptr[1], header_ptr[2], header_ptr[3]);)

  packet.file_offset  = offset;
  packet.sync_byte    = header_ptr[0];
  packet.tei          = (header_ptr[1] & 0x80) >> 7;
  packet.pusi         = (header_ptr[1] & 0x40) >> 6;
  packet.tsp          = (header_ptr[1] & 0x20) >> 5;
  packet.pid          = ((header_ptr[1] & 0x1f) << 8) | header_ptr[2];
  packet.tsc          = (header_ptr[3] & 0xc0) >> 6;
  packet.afc          = (header_ptr[3] & 0x30) >> 4;
  packet.cc           = (header_ptr[3] & 0x0f);

  // list of pids
  m_ts_pids[packet.pid].pid = packet.pid;
  m_ts_pids[packet.pid].count++;
  m_ts_pids[packet.pid].packets.push_back(packet);

  DBGR(DbgWrite("--%s()\n", __func__);)
}

bool TsFileBase::parse(void)
{
  bool result = false;

  DBGS(DbgWrite("++%s()\n", __func__);)
  m_packet_size = 0;
  if (m_packet_size || findTsPacketSize())
  {
    m_ts_packet = new uint8_t [TS_BUFFER];
    if (m_ts_packet)
    {
      std::memset(m_ts_packet, 0x00, TS_BUFFER);
      readTsStream();
      result = true;
    }
  }
  DBGR(DbgWrite("--%s() - result: %d\n", __func__, result);)
  return result;
}

bool TsFileBase::extractPath(std::string& filename, std::string& path, std::string& name)
{
  std::size_t found = filename.rfind('\\');
  bool result = false;

  DBGS(DbgWrite("++%s()\n", __func__);)
  if (found != std::string::npos)
  {
    path = filename.substr(0, found + 1);
    name = filename.substr(found + 1, filename.length() - (found + 1));
    result = true;
  }
  else
  {
    name = filename;
  }
  DBG1(DbgWrite("   filename: '%s'\n", filename.c_str());)
  DBG1(DbgWrite("       path: '%s'\n", path.c_str());)
  DBG1(DbgWrite("       name: '%s'\n", name.c_str());)
  DBGR(DbgWrite("--%s() - result: %d\n", __func__, result);)
  return result;
}

bool TsFileBase::writePid(uint16_t pid, std::vector<ts_packet_t>& packets, std::string& fileName)
{
  uint16_t packet_size = m_packet_size;

  std::stringstream stream;

  std::string fpath = "";
  std::string fname = "";

  std::ofstream fid;
  bool result = true;

  DBGS(DbgWrite("++%s(pid: 0x%04X [%d], fileName: %s)\n", __func__, pid, pid,
          fileName.empty() ? "-" : fileName.c_str());)
  if (fileName.empty())
  {
    if (extractPath(m_filename, fpath, fname))
    {
      DBG1(DbgWrite("   path: %s\n", fpath.c_str());)
      DBG1(DbgWrite("   name: %s\n", fname.c_str());)
    }
    stream << fpath << "Pid_0x" << std::setfill('0') << std::setw(4) << std::hex << pid << "_" << fname;
    fileName.assign(stream.str());
  }

  DBG1(DbgWrite("   %s() filename: %s\n", __func__, fileName.c_str());)

  fid.open(fileName, std::ofstream::out | std::ifstream::binary);
  if (fid.is_open())
  {
    auto packets_cnt = packets.size();

    for (uint64_t i = 0; i < packets_cnt; i++)
    {
      DBG1(DbgWrite("   [%d] setSeek(%ul)\n", i, packets[i].file_offset);)
      printf("Writing PID: 0x%04X [%d] to file: %d%%\r", pid, pid, (uint8_t) ((100 * i) / packets_cnt));
      setSeek(packets[i].file_offset);
      m_fid.read((char*) m_ts_packet, packet_size);
      if (m_fid.gcount() == packet_size)
      {
        fid.write((char*) m_ts_packet, packet_size);
      }
    }
    printf("Writing PID: 0x%04X [%d] to file: DONE\n\r", pid, pid);
    fid.close();
  }
  DBGR(DbgWrite("--%s() - result: %d\n", __func__, result);)
  return result;
}

bool TsFileBase::extractPid(uint16_t pid, std::string& fileName)
{
  std::map<uint16_t, ts_pid_t>::iterator it;
  bool result = false;

  DBGS(DbgWrite("++%s(pid: 0x%04X [%d], fileName: %s)\n", __func__, pid, pid,
          fileName.empty() ? "-" : fileName.c_str());)
  it = m_ts_pids.find(pid);
  if (it != m_ts_pids.end())
  {
    ts_pid_t* pids = &it->second;

    writePid(pid, pids->packets, fileName);
    result = true;
  }
  DBGR(DbgWrite("--%s() - result: %d\n", __func__, result);)
  return result;
}

bool TsFileBase::getTsPids(std::map<uint16_t, ts_pid_t>** pids)
{
  bool result = true;

  DBGS(DbgWrite("++%s()\n", __func__);)
  *pids = &m_ts_pids;
  DBGR(DbgWrite("--%s()\n", __func__);)
  return result;
}

