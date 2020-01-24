/*
 * Copyright (c) 2019, 4Embedded.Systems all rights reserved.
 */

#ifndef TSDBG_HPP_INCLUDED
#define TSDBG_HPP_INCLUDED

#include <cstdio>
#include <cstdint>


#define DBG_S     (1 << 0)
#define DBG_R     (1 << 1)
#define DBG_E     (1 << 2)
#define DBG_W     (1 << 3)
#define DBG_1     (1 << 4)
#define DBG_2     (1 << 5)
#define DBG_3     (1 << 6)

#define DBG_ALL   (0xFF)

#ifndef DBG_LEVEL
  #define DBG_LEVEL     0
#endif

#if (DBG_LEVEL & DBG_S)
  #define DBGS(x)       x
#else
  #define DBGS(x)
#endif

#if DBG_LEVEL & DBG_R
  #define DBGR(x)       x
#else
  #define DBGR(x)
#endif

#if DBG_LEVEL & DBG_E
  #define DBGE(x)       x
#else
  #define DBGE(x)
#endif

#if DBG_LEVEL & DBG_W
  #define DBGW(x)       x
#else
  #define DBGW(x)
#endif

#if DBG_LEVEL & DBG_1
  #define DBG1(x)       x
#else
  #define DBG1(x)
#endif

#if DBG_LEVEL & DBG_2
  #define DBG2(x)       x
#else
  #define DBG2(x)
#endif

#if DBG_LEVEL & DBG_3
  #define DBG3(x)       x
#else
  #define DBG3(x)
#endif


#define DbgWrite        printf


void DbgMemory(const uint8_t* data_ptr, const uint32_t data_len);

#endif // TSDBG_HPP_INCLUDED
