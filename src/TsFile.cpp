/*
 * Copyright (c) 2019, 4Embedded.Systems all rights reserved.
 */

#include <cstdio>
#include <cstring>
#include <string>
#include <cassert>
#include <sstream>

#include "TsFile.hpp"

#include "options.dbg"
#ifdef DBG_LVL_TSFILE
  #define DBG_LEVEL   DBG_LVL_TSFILE
#endif // DBG_LVL_TSFILE
#include "TsDbg.hpp"


#define TS_HEADER_SIZE  4
#define TS_BUFFER       (16 * 1024)
#define TS_SYNC_BYTE    0x47


static uint8_t    ts_packet[TS_PACKET_SIZE];


TsFileBase::TsFileBase(std::string fileName)
{
  DBGS(DbgWrite("++%s(fileName: %s)\n", __func__, fileName.c_str());)
  m_fSize = 0;
  m_filename = fileName;

  m_fid.open(m_filename, std::ifstream::in | std::ifstream::binary);
  if (m_fid.is_open())
  {
    getSize();
  }
  DBGR(DbgWrite("--%s()\n", __func__);)
}

TsFileBase::~TsFileBase()
{
  DBGS(DbgWrite("++%s()\n", __func__);)
  if (m_fid.is_open())
  {
    m_fid.close();
  }
  DBGR(DbgWrite("--%s()\n", __func__);)
}

void TsFileBase::getSize(void)
{
  DBGS(DbgWrite("++%s()\n", __func__);)
  m_fid.seekg(0, m_fid.end);
  m_fSize = m_fid.tellg();
  DBG1(DbgWrite("[%s] m_fSize: %lu\n", __func__, m_fSize);)
  m_fid.seekg(0, m_fid.beg);
  DBGR(DbgWrite("--%s()\n", __func__);)
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

void TsFileBase::findTsSyncByte(void)
{
  uint64_t current_idx = 0;

  DBGS(DbgWrite("++%s()\n", __func__);)
  while(!m_fid.eof() && ((current_idx + TS_PACKET_SIZE) <= m_fSize))
  {
    DBG1(DbgWrite("[%s] idx: %lu\n", __func__, current_idx);)
    m_fid.read((char*) &ts_packet[0], 1);
    if (ts_packet[0] == TS_SYNC_BYTE)
    {
      m_fid.read((char*) &ts_packet[1], TS_PACKET_SIZE - 1);

      parseTsPacket(current_idx, ts_packet, TS_PACKET_SIZE);
      current_idx += TS_PACKET_SIZE;
    }
    else
    {
      current_idx++;
    }
  }
  DBGR(DbgWrite("--%s()\n", __func__);)
}

void TsFileBase::parseTsPacket(uint64_t offset, uint8_t* packet_ptr, uint32_t packet_size)
{
  uint8_t*  header_ptr = packet_ptr;
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

  //if ((packet.pid >= 0) && (packet.pid <= 31))
  {
    memcpy(packet.raw_tab, packet_ptr, packet_size);
    packet.raw_size = packet_size;
  }

  // list of pids
  m_ts_pids[packet.pid].pid = packet.pid;
  m_ts_pids[packet.pid].count++;
  //if ((packet.pid >= 0) && (packet.pid <= 31))
  {
    m_ts_pids[packet.pid].packets.push_back(packet);
  }

  DBGR(DbgWrite("--%s()\n", __func__);)
}

bool TsFileBase::parse(void)
{
  bool result = true;

  DBGS(DbgWrite("++%s()\n", __func__);)
  findTsSyncByte();
  DBGR(DbgWrite("--%s() - result: %d\n", __func__, result);)
  return result;
}

bool TsFileBase::writePid(uint16_t pid, std::vector<ts_packet_t>& packets, std::string fileName)
{
  std::stringstream name;
  std::ofstream fid;
  bool result = true;

  DBGS(DbgWrite("++%s(pid: 0x%04X [%d], fileName: %s)\n", __func__, pid, pid, fileName);)
  if (fileName.empty())
  {
    name << "Pid_0x" << std::hex << pid << "_" << m_filename;
  }
  else
  {
    name << fileName;
  }
  DBG1(DbgWrite("   %s() filename: %s\n", __func__, name.str().c_str());)
  fid.open(name.str().c_str(), std::ofstream::out | std::ifstream::binary);
  if (fid.is_open())
  {
    auto packets_cnt = packets.size();

    for (uint64_t i = 0; i < packets_cnt; i++)
    {
      ts_packet_t* packet = &packets[i];

      fid.write((const char*) packet->raw_tab, packet->raw_size);
    }
    fid.close();
  }
  DBGR(DbgWrite("--%s() - result: %d\n", __func__, result);)
  return result;
}

bool TsFileBase::extractPid(uint16_t pid, std::string fileName)
{
  std::map<uint16_t, ts_pid_t>::iterator it;
  bool result = true;

  DBGS(DbgWrite("++%s(pid: 0x%04X [%d], fileName: %s)\n", __func__, pid, pid, fileName.c_str());)
  it = m_ts_pids.find(pid);
  if (it != m_ts_pids.end())
  {
    std::vector<ts_packet_t> onepid = ((ts_pid_t)(it->second)).packets;

    if (onepid.size())
    {
      writePid(pid, onepid, fileName);
    }
    result = true;
  }

  DBGR(DbgWrite("--%s() - result: %d\n", __func__, result);)
  return result;
}

void TsFileBase::showTsHeader(uint64_t idx, ts_packet_t* header)
{
  DBG3(DbgWrite("Packet id: %lu\n", idx);)
  DBG3(DbgWrite("                         TS file offset: %lu\n", header->file_offset);)
  DBG3(DbgWrite("                           TS Sync Byte: 0x%02x\n", header->sync_byte);)
  DBG3(DbgWrite("        Transport error indicator (TEI): %d\n", header->tei);)
  DBG3(DbgWrite("    Payload unit start indicator (PUSI): %d\n", header->pusi);)
  DBG3(DbgWrite("                     Transport priority: %d\n", header->tsp);)
  DBG3(DbgWrite("                                    PID: 0x%04x [ %d ]\n", header->pid, header->pid);)
  DBG3(DbgWrite("     Transport scrambling control (TSC): 0x%02x\n", header->tsc);)
  DBG3(DbgWrite("               Adaptation field control: 0x%02x\n", header->afc);)
  DBG3(DbgWrite("                     Continuity counter: 0x%02x\n", header->cc);)
  DBG3(DbgWrite("---------------------------------------------------------\n");)
}

bool TsFileBase::getTsPackets(uint16_t pid, std::vector<ts_packet_t>& packets)
{
  std::map<uint16_t, ts_pid_t>::iterator it;
  bool result = false;

  DBGS(DbgWrite("++%s(pid: 0x%04X [%d])\n", __func__, pid, pid);)
  it = m_ts_pids.find(pid);
  if (it != m_ts_pids.end())
  {
    packets = ((ts_pid_t)(it->second)).packets;
    result = true;
  }
  DBGR(DbgWrite("--%s()\n", __func__);)
  return result;
}

bool TsFileBase::getTsPids(std::map<uint16_t, ts_pid_t>& pids)
{
  bool result = true;

  DBGS(DbgWrite("++%s()\n", __func__);)
  pids = m_ts_pids;
  DBGR(DbgWrite("--%s()\n", __func__);)
  return result;
}

