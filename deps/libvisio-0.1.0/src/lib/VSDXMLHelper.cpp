/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef BOOST_LEXICAL_CAST_ASSUME_C_LOCALE
#define BOOST_LEXICAL_CAST_ASSUME_C_LOCALE 1
#endif

#include <sstream>
#include <istream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <libxml/xmlIO.h>
#include <libxml/xmlstring.h>
#include <librevenge-stream/librevenge-stream.h>
#include "VSDXMLHelper.h"
#include "libvisio_utils.h"


namespace
{

extern "C" {

  static int vsdxInputCloseFunc(void *)
  {
    return 0;
  }

  static int vsdxInputReadFunc(void *context, char *buffer, int len)
  {
    librevenge::RVNGInputStream *input = (librevenge::RVNGInputStream *)context;

    if ((!input) || (!buffer) || (len < 0))
      return -1;

    if (input->isEnd())
      return 0;

    unsigned long tmpNumBytesRead = 0;
    const unsigned char *tmpBuffer = input->read(len, tmpNumBytesRead);

    if (tmpBuffer && tmpNumBytesRead)
      memcpy(buffer, tmpBuffer, tmpNumBytesRead);
    return tmpNumBytesRead;
  }

#ifdef DEBUG
  static void vsdxReaderErrorFunc(void *, const char *message, xmlParserSeverities severity, xmlTextReaderLocatorPtr)
#else
  static void vsdxReaderErrorFunc(void *, const char *, xmlParserSeverities severity, xmlTextReaderLocatorPtr)
#endif
  {
    switch (severity)
    {
    case XML_PARSER_SEVERITY_VALIDITY_WARNING:
      VSD_DEBUG_MSG(("Found xml parser severity validity warning %s\n", message));
      break;
    case XML_PARSER_SEVERITY_VALIDITY_ERROR:
      VSD_DEBUG_MSG(("Found xml parser severity validity error %s\n", message));
      break;
    case XML_PARSER_SEVERITY_WARNING:
      VSD_DEBUG_MSG(("Found xml parser severity warning %s\n", message));
      break;
    case XML_PARSER_SEVERITY_ERROR:
      VSD_DEBUG_MSG(("Found xml parser severity error %s\n", message));
      break;
    default:
      break;
    }
  }

} // extern "C"

} // anonymous namespace

// xmlTextReader helper function

xmlTextReaderPtr libvisio::xmlReaderForStream(librevenge::RVNGInputStream *input, const char *URL, const char *encoding, int options)
{
  xmlTextReaderPtr reader = xmlReaderForIO(vsdxInputReadFunc, vsdxInputCloseFunc, (void *)input, URL, encoding, options);
  xmlTextReaderSetErrorHandler(reader, vsdxReaderErrorFunc, 0);
  return reader;
}

libvisio::Colour libvisio::xmlStringToColour(const xmlChar *s)
{
  if (xmlStrEqual(s, BAD_CAST("Themed")))
    return libvisio::Colour();
  std::string str((const char *)s);
  if (str[0] == '#')
  {
    if (str.length() != 7)
    {
      VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
      throw XmlParserException();
    }
    else
      str.erase(str.begin());
  }
  else
  {
    if (str.length() != 6)
    {
      VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
      throw XmlParserException();
    }
  }

  std::istringstream istr(str);
  unsigned val = 0;
  istr >> std::hex >> val;

  return Colour((val & 0xff0000) >> 16, (val & 0xff00) >> 8, val & 0xff, 0);
}

long libvisio::xmlStringToLong(const xmlChar *s)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast;
  if (xmlStrEqual(s, BAD_CAST("Themed")))
    return 0;

  try
  {
    return boost::lexical_cast<long, const char *>((const char *)s);
  }
  catch (const boost::bad_lexical_cast &)
  {
    VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
    throw XmlParserException();
  }
  return 0;
}

double libvisio::xmlStringToDouble(const xmlChar *s)
{
  if (xmlStrEqual(s, BAD_CAST("Themed")))
    return 0.0;

  try
  {
    return boost::lexical_cast<double, const char *>((const char *)s);
  }
  catch (const boost::bad_lexical_cast &)
  {
    VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
    throw XmlParserException();
  }
  return 0.0;
}

bool libvisio::xmlStringToBool(const xmlChar *s)
{
  if (xmlStrEqual(s, BAD_CAST("Themed")))
    return 0;

  bool value = false;
  if (xmlStrEqual(s, BAD_CAST("true")) || xmlStrEqual(s, BAD_CAST("1")))
    value = true;
  else if (xmlStrEqual(s, BAD_CAST("false")) || xmlStrEqual(s, BAD_CAST("0")))
    value = false;
  else
  {
    VSD_DEBUG_MSG(("Throwing XmlParserException\n"));
    throw XmlParserException();
  }
  return value;

}

// VSDXRelationship

libvisio::VSDXRelationship::VSDXRelationship(xmlTextReaderPtr reader)
  : m_id(), m_type(), m_target()
{
  if (reader)
    // TODO: check whether we are actually parsing "Relationship" element
  {
    while (xmlTextReaderMoveToNextAttribute(reader))
    {
      const xmlChar *name = xmlTextReaderConstName(reader);
      const xmlChar *value = xmlTextReaderConstValue(reader);
      if (xmlStrEqual(name, BAD_CAST("Id")))
        m_id = (const char *)value;
      else if (xmlStrEqual(name, BAD_CAST("Type")))
        m_type = (const char *)value;
      else if (xmlStrEqual(name, BAD_CAST("Target")))
        m_target = (const char *)value;
    }
    // VSD_DEBUG_MSG(("Relationship : %s type: %s target: %s\n", m_id.c_str(), m_type.c_str(), m_target.c_str()));
  }
}

libvisio::VSDXRelationship::VSDXRelationship()
  : m_id(), m_type(), m_target()
{
}

libvisio::VSDXRelationship::~VSDXRelationship()
{
}

void libvisio::VSDXRelationship::rebaseTarget(const char *baseDir)
{
  std::string target(baseDir ? baseDir : "");
  if (!target.empty())
    target += "/";
  target += m_target;

  // normalize path by resolving any ".." or "." segments
  // and eliminating duplicate path separators
  std::vector<std::string> segments;
  boost::split(segments, target, boost::is_any_of("/\\"));
  std::vector<std::string> normalizedSegments;

  for (unsigned i = 0; i < segments.size(); ++i)
  {
    if (segments[i] == "..")
      normalizedSegments.pop_back();
    else if (segments[i] != "." && !segments[i].empty())
      normalizedSegments.push_back(segments[i]);
  }

  target.clear();
  for (unsigned j = 0; j < normalizedSegments.size(); ++j)
  {
    if (!target.empty())
      target.append("/");
    target.append(normalizedSegments[j]);
  }

  // VSD_DEBUG_MSG(("VSDXRelationship::rebaseTarget %s -> %s\n", m_target.c_str(), target.c_str()));
  m_target = target;
}


// VSDXRelationships

libvisio::VSDXRelationships::VSDXRelationships(librevenge::RVNGInputStream *input)
  : m_relsByType(), m_relsById()
{
  if (input)
  {
    xmlTextReaderPtr reader = xmlReaderForStream(input, 0, 0, XML_PARSE_NOBLANKS|XML_PARSE_NOENT|XML_PARSE_NONET|XML_PARSE_RECOVER);
    if (reader)
    {
      bool inRelationships = false;
      int ret = xmlTextReaderRead(reader);
      while (ret == 1)
      {
        const xmlChar *name = xmlTextReaderConstName(reader);
        if (name)
        {
          if (xmlStrEqual(name, BAD_CAST("Relationships")))
          {
            if (xmlTextReaderNodeType(reader) == 1)
            {
              // VSD_DEBUG_MSG(("Relationships ON\n"));
              inRelationships = true;
            }
            else if (xmlTextReaderNodeType(reader) == 15)
            {
              // VSD_DEBUG_MSG(("Relationships OFF\n"));
              inRelationships = false;
            }
          }
          else if (xmlStrEqual(name, BAD_CAST("Relationship")))
          {
            if (inRelationships)
            {
              VSDXRelationship relationship(reader);
              m_relsByType[relationship.getType()] = relationship;
              m_relsById[relationship.getId()] = relationship;
            }
          }
        }
        ret = xmlTextReaderRead(reader);
      }
      xmlFreeTextReader(reader);
    }
  }
}

libvisio::VSDXRelationships::~VSDXRelationships()
{
}

void libvisio::VSDXRelationships::rebaseTargets(const char *baseDir)
{
  std::map<std::string, libvisio::VSDXRelationship>::iterator iter;
  for (iter = m_relsByType.begin(); iter != m_relsByType.end(); ++iter)
    iter->second.rebaseTarget(baseDir);
  for (iter = m_relsById.begin(); iter != m_relsById.end(); ++iter)
    iter->second.rebaseTarget(baseDir);
}

const libvisio::VSDXRelationship *libvisio::VSDXRelationships::getRelationshipByType(const char *type) const
{
  if (!type)
    return 0;
  std::map<std::string, libvisio::VSDXRelationship>::const_iterator iter = m_relsByType.find(type);
  if (iter != m_relsByType.end())
    return &(iter->second);
  return 0;
}

const libvisio::VSDXRelationship *libvisio::VSDXRelationships::getRelationshipById(const char *id) const
{
  if (!id)
    return 0;
  std::map<std::string, libvisio::VSDXRelationship>::const_iterator iter = m_relsById.find(id);
  if (iter != m_relsById.end())
    return &(iter->second);
  return 0;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
