/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>
#include <string>
#include <algorithm> // std::count
#include "VSDInternalStream.h"
#include "libvisio_utils.h"

uint8_t libvisio::readU8(librevenge::RVNGInputStream *input)
{
  if (!input || input->isEnd())
  {
    VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
    throw EndOfStreamException();
  }
  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint8_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint8_t))
    return *(uint8_t const *)(p);
  VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
  throw EndOfStreamException();
}

uint16_t libvisio::readU16(librevenge::RVNGInputStream *input)
{
  if (!input || input->isEnd())
  {
    VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
    throw EndOfStreamException();
  }
  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint16_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint16_t))
    return (uint16_t)p[0]|((uint16_t)p[1]<<8);
  VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
  throw EndOfStreamException();
}

int16_t libvisio::readS16(librevenge::RVNGInputStream *input)
{
  return (int16_t)readU16(input);
}

uint32_t libvisio::readU32(librevenge::RVNGInputStream *input)
{
  if (!input || input->isEnd())
  {
    VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
    throw EndOfStreamException();
  }
  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint32_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint32_t))
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
  VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
  throw EndOfStreamException();
}

int32_t libvisio::readS32(librevenge::RVNGInputStream *input)
{
  return (int32_t)readU32(input);
}

uint64_t libvisio::readU64(librevenge::RVNGInputStream *input)
{
  if (!input || input->isEnd())
  {
    VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
    throw EndOfStreamException();
  }
  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint64_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint64_t))
    return (uint64_t)p[0]|((uint64_t)p[1]<<8)|((uint64_t)p[2]<<16)|((uint64_t)p[3]<<24)|((uint64_t)p[4]<<32)|((uint64_t)p[5]<<40)|((uint64_t)p[6]<<48)|((uint64_t)p[7]<<56);
  VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
  throw EndOfStreamException();
}

double libvisio::readDouble(librevenge::RVNGInputStream *input)
{
  union
  {
    uint64_t u;
    double d;
  } tmpUnion;

  tmpUnion.u = readU64(input);

  return tmpUnion.d;
}

const librevenge::RVNGString libvisio::getColourString(const Colour &c)
{
  librevenge::RVNGString sColour;
  sColour.sprintf("#%.2x%.2x%.2x", c.r, c.g, c.b);
  return sColour;
}



/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
