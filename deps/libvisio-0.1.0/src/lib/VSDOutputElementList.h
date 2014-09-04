/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDOUTPUTELEMENTLIST_H__
#define __VSDOUTPUTELEMENTLIST_H__

#include <map>
#include <list>
#include <vector>
#include <librevenge/librevenge.h>

namespace libvisio
{

class VSDOutputElement;

class VSDOutputElementList
{
public:
  VSDOutputElementList();
  VSDOutputElementList(const VSDOutputElementList &elementList);
  VSDOutputElementList &operator=(const VSDOutputElementList &elementList);
  virtual ~VSDOutputElementList();
  void append(const VSDOutputElementList &elementList);
  void draw(librevenge::RVNGDrawingInterface *painter) const;
  void addStyle(const librevenge::RVNGPropertyList &propList);
  void addPath(const librevenge::RVNGPropertyList &propList);
  void addGraphicObject(const librevenge::RVNGPropertyList &propList);
  void addStartTextObject(const librevenge::RVNGPropertyList &propList);
  void addOpenParagraph(const librevenge::RVNGPropertyList &propList);
  void addOpenSpan(const librevenge::RVNGPropertyList &propList);
  void addInsertText(const librevenge::RVNGString &text);
  void addCloseSpan();
  void addCloseParagraph();
  void addEndTextObject();
  void addStartLayer(const librevenge::RVNGPropertyList &propList);
  void addEndLayer();
  bool empty() const
  {
    return m_elements.empty();
  }
private:
  std::vector<VSDOutputElement *> m_elements;
};


} // namespace libvisio

#endif // __VSDOUTPUTELEMENTLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
