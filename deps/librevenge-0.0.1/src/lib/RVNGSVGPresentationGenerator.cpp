/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* librevenge
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Lesser General Public License Version 2.1 or later
 * (LGPLv2.1+), in which case the provisions of the LGPLv2.1+ are
 * applicable instead of those above.
 */

#include <locale.h>

#include <map>
#include <sstream>
#include <string>

#include <librevenge-generators/librevenge-generators.h>

#include "librevenge_internal.h"

namespace librevenge
{

namespace
{

static std::string doubleToString(const double value)
{
	RVNGProperty *prop = RVNGPropertyFactory::newDoubleProp(value);
	std::string retVal = prop->getStr().cstr();
	delete prop;
	return retVal;
}

static unsigned stringToColour(const RVNGString &s)
{
	std::string str(s.cstr());
	if (str[0] == '#')
	{
		if (str.length() != 7)
			return 0;
		else
			str.erase(str.begin());
	}
	else
		return 0;

	std::istringstream istr(str);
	unsigned val = 0;
	istr >> std::hex >> val;
	return val;
}

}

struct RVNGSVGPresentationGeneratorImpl
{
	RVNGSVGPresentationGeneratorImpl(RVNGStringVector &m_vec);

	std::map<int, RVNGPropertyList> m_idSpanMap;

	RVNGPropertyListVector m_gradient;
	RVNGPropertyList m_style;
	int m_gradientIndex;
	int m_patternIndex;
	int m_shadowIndex;

	void writeStyle(bool isClosed=true);
	void drawPolySomething(const RVNGPropertyListVector &vertices, bool isClosed);

	std::ostringstream m_outputSink;
	RVNGStringVector &m_vec;
};

RVNGSVGPresentationGeneratorImpl::RVNGSVGPresentationGeneratorImpl(RVNGStringVector &vec)
	: m_idSpanMap()
	, m_gradient()
	, m_style()
	, m_gradientIndex(1)
	, m_patternIndex(1)
	, m_shadowIndex(1)
	, m_outputSink()
	, m_vec(vec)
{
}

RVNGSVGPresentationGenerator::RVNGSVGPresentationGenerator(RVNGStringVector &vec)
	: m_impl(new RVNGSVGPresentationGeneratorImpl(vec))
{
}

RVNGSVGPresentationGenerator::~RVNGSVGPresentationGenerator()
{
	delete m_impl;
}

void RVNGSVGPresentationGenerator::startDocument(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::endDocument()
{
}

void RVNGSVGPresentationGenerator::setDocumentMetaData(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::defineEmbeddedFont(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::startSlide(const RVNGPropertyList &propList)
{
	m_impl->m_outputSink << "<svg:svg version=\"1.1\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" ";
	if (propList["svg:width"])
		m_impl->m_outputSink << "width=\"" << doubleToString(72*(propList["svg:width"]->getDouble())) << "\" ";
	if (propList["svg:height"])
		m_impl->m_outputSink << "height=\"" << doubleToString(72*(propList["svg:height"]->getDouble())) << "\"";
	m_impl->m_outputSink << " >\n";
}

void RVNGSVGPresentationGenerator::endSlide()
{
	m_impl->m_outputSink << "</svg:svg>\n";
	m_impl->m_vec.append(m_impl->m_outputSink.str().c_str());
	m_impl->m_outputSink.str("");
}

void RVNGSVGPresentationGenerator::startMasterSlide(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::endMasterSlide()
{
	// we don't do anything with master slides yet, so just reset the content
	m_impl->m_outputSink.str("");
}

void RVNGSVGPresentationGenerator::setSlideTransition(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::setStyle(const RVNGPropertyList &propList)
{
	m_impl->m_style.clear();
	m_impl->m_style = propList;

	const librevenge::RVNGPropertyListVector *gradient = propList.child("svg:linearGradient");
	if (!gradient)
		gradient = propList.child("svg:radialGradient");
	m_impl->m_gradient = gradient ? *gradient : librevenge::RVNGPropertyListVector();
	if (m_impl->m_style["draw:shadow"] && m_impl->m_style["draw:shadow"]->getStr() == "visible")
	{
		double shadowRed = 0.0;
		double shadowGreen = 0.0;
		double shadowBlue = 0.0;
		if (m_impl->m_style["draw:shadow-color"])
		{
			const unsigned shadowColour = stringToColour(m_impl->m_style["draw:shadow-color"]->getStr());
			shadowRed = (double)((shadowColour & 0x00ff0000) >> 16)/255.0;
			shadowGreen = (double)((shadowColour & 0x0000ff00) >> 8)/255.0;
			shadowBlue = (double)(shadowColour & 0x000000ff)/255.0;
		}
		m_impl->m_outputSink << "<svg:defs>\n";
		m_impl->m_outputSink << "<svg:filter filterUnits=\"userSpaceOnUse\" id=\"shadow" << m_impl->m_shadowIndex++ << "\">";
		m_impl->m_outputSink << "<svg:feOffset in=\"SourceGraphic\" result=\"offset\" ";
		m_impl->m_outputSink << "dx=\"" << doubleToString(72*m_impl->m_style["draw:shadow-offset-x"]->getDouble()) << "\" ";
		m_impl->m_outputSink << "dy=\"" << doubleToString(72*m_impl->m_style["draw:shadow-offset-y"]->getDouble()) << "\"/>";
		m_impl->m_outputSink << "<svg:feColorMatrix in=\"offset\" result=\"offset-color\" type=\"matrix\" values=\"";
		m_impl->m_outputSink << "0 0 0 0 " << doubleToString(shadowRed) ;
		m_impl->m_outputSink << " 0 0 0 0 " << doubleToString(shadowGreen);
		m_impl->m_outputSink << " 0 0 0 0 " << doubleToString(shadowBlue);
		if (m_impl->m_style["draw:opacity"] && m_impl->m_style["draw:opacity"]->getDouble() < 1)
			m_impl->m_outputSink << " 0 0 0 "   << doubleToString(m_impl->m_style["draw:shadow-opacity"]->getDouble()/m_impl->m_style["draw:opacity"]->getDouble()) << " 0\"/>";
		else
			m_impl->m_outputSink << " 0 0 0 "   << doubleToString(m_impl->m_style["draw:shadow-opacity"]->getDouble()) << " 0\"/>";
		m_impl->m_outputSink << "<svg:feMerge><svg:feMergeNode in=\"offset-color\" /><svg:feMergeNode in=\"SourceGraphic\" /></svg:feMerge></svg:filter></svg:defs>";
	}

	if (m_impl->m_style["draw:fill"] && m_impl->m_style["draw:fill"]->getStr() == "gradient")
	{
		double angle = (m_impl->m_style["draw:angle"] ? m_impl->m_style["draw:angle"]->getDouble() : 0.0);
		angle *= -1.0;
		while (angle < 0)
			angle += 360;
		while (angle > 360)
			angle -= 360;

		if (!m_impl->m_gradient.count())
		{
			if (m_impl->m_style["draw:style"] &&
			        (m_impl->m_style["draw:style"]->getStr() == "radial" ||
			         m_impl->m_style["draw:style"]->getStr() == "rectangular" ||
			         m_impl->m_style["draw:style"]->getStr() == "square" ||
			         m_impl->m_style["draw:style"]->getStr() == "ellipsoid"))
			{
				m_impl->m_outputSink << "<svg:defs>\n";
				m_impl->m_outputSink << "  <svg:radialGradient id=\"grad" << m_impl->m_gradientIndex++ << "\"";

				if (m_impl->m_style["svg:cx"])
					m_impl->m_outputSink << " cx=\"" << m_impl->m_style["svg:cx"]->getStr().cstr() << "\"";
				else if (m_impl->m_style["draw:cx"])
					m_impl->m_outputSink << " cx=\"" << m_impl->m_style["draw:cx"]->getStr().cstr() << "\"";

				if (m_impl->m_style["svg:cy"])
					m_impl->m_outputSink << " cy=\"" << m_impl->m_style["svg:cy"]->getStr().cstr() << "\"";
				else if (m_impl->m_style["draw:cy"])
					m_impl->m_outputSink << " cy=\"" << m_impl->m_style["draw:cy"]->getStr().cstr() << "\"";
				m_impl->m_outputSink << " r=\"" << (1 - (m_impl->m_style["draw:border"] ? m_impl->m_style["draw:border"]->getDouble() : 0))*100.0 << "%\" >\n";
				m_impl->m_outputSink << " >\n";

				if (m_impl->m_style["draw:start-color"] && m_impl->m_style["draw:end-color"])
				{
					m_impl->m_outputSink << "    <svg:stop offset=\"0%\"";
					m_impl->m_outputSink << " stop-color=\"" << m_impl->m_style["draw:end-color"]->getStr().cstr() << "\"";
					m_impl->m_outputSink << " stop-opacity=\"" << (m_impl->m_style["libwpg:end-opacity"] ? m_impl->m_style["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;

					m_impl->m_outputSink << "    <svg:stop offset=\"100%\"";
					m_impl->m_outputSink << " stop-color=\"" << m_impl->m_style["draw:start-color"]->getStr().cstr() << "\"";
					m_impl->m_outputSink << " stop-opacity=\"" << (m_impl->m_style["libwpg:start-opacity"] ? m_impl->m_style["libwpg:start-opacity"]->getDouble() : 1) << "\" />" << std::endl;
				}
				m_impl->m_outputSink << "  </svg:radialGradient>\n";
				m_impl->m_outputSink << "</svg:defs>\n";
			}
			else if (m_impl->m_style["draw:style"] && m_impl->m_style["draw:style"]->getStr() == "linear")
			{
				m_impl->m_outputSink << "<svg:defs>\n";
				m_impl->m_outputSink << "  <svg:linearGradient id=\"grad" << m_impl->m_gradientIndex++ << "\" >\n";

				if (m_impl->m_style["draw:start-color"] && m_impl->m_style["draw:end-color"])
				{
					m_impl->m_outputSink << "    <svg:stop offset=\"0%\"";
					m_impl->m_outputSink << " stop-color=\"" << m_impl->m_style["draw:start-color"]->getStr().cstr() << "\"";
					m_impl->m_outputSink << " stop-opacity=\"" << (m_impl->m_style["libwpg:start-opacity"] ? m_impl->m_style["libwpg:start-opacity"]->getDouble() : 1) << "\" />" << std::endl;

					m_impl->m_outputSink << "    <svg:stop offset=\"100%\"";
					m_impl->m_outputSink << " stop-color=\"" << m_impl->m_style["draw:end-color"]->getStr().cstr() << "\"";
					m_impl->m_outputSink << " stop-opacity=\"" << (m_impl->m_style["libwpg:end-opacity"] ? m_impl->m_style["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;
				}
				m_impl->m_outputSink << "  </svg:linearGradient>\n";

				// not a simple horizontal gradient
				if (angle < 270 || angle > 270)
				{
					m_impl->m_outputSink << "  <svg:linearGradient xlink:href=\"#grad" << m_impl->m_gradientIndex-1 << "\"";
					m_impl->m_outputSink << " id=\"grad" << m_impl->m_gradientIndex++ << "\" ";
					m_impl->m_outputSink << "x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\" ";
					m_impl->m_outputSink << "gradientTransform=\"rotate(" << angle << " .5 .5)\" ";
					m_impl->m_outputSink << "gradientUnits=\"objectBoundingBox\" >\n";
					m_impl->m_outputSink << "  </svg:linearGradient>\n";
				}

				m_impl->m_outputSink << "</svg:defs>\n";
			}
			else if (m_impl->m_style["draw:style"] && m_impl->m_style["draw:style"]->getStr() == "axial")
			{
				m_impl->m_outputSink << "<svg:defs>\n";
				m_impl->m_outputSink << "  <svg:linearGradient id=\"grad" << m_impl->m_gradientIndex++ << "\" >\n";

				if (m_impl->m_style["draw:start-color"] && m_impl->m_style["draw:end-color"])
				{
					m_impl->m_outputSink << "    <svg:stop offset=\"0%\"";
					m_impl->m_outputSink << " stop-color=\"" << m_impl->m_style["draw:end-color"]->getStr().cstr() << "\"";
					m_impl->m_outputSink << " stop-opacity=\"" << (m_impl->m_style["libwpg:end-opacity"] ? m_impl->m_style["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;

					m_impl->m_outputSink << "    <svg:stop offset=\"50%\"";
					m_impl->m_outputSink << " stop-color=\"" << m_impl->m_style["draw:start-color"]->getStr().cstr() << "\"";
					m_impl->m_outputSink << " stop-opacity=\"" << (m_impl->m_style["libwpg:start-opacity"] ? m_impl->m_style["libwpg:start-opacity"]->getDouble() : 1) << "\" />" << std::endl;

					m_impl->m_outputSink << "    <svg:stop offset=\"100%\"";
					m_impl->m_outputSink << " stop-color=\"" << m_impl->m_style["draw:end-color"]->getStr().cstr() << "\"";
					m_impl->m_outputSink << " stop-opacity=\"" << (m_impl->m_style["libwpg:end-opacity"] ? m_impl->m_style["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;
				}
				m_impl->m_outputSink << "  </svg:linearGradient>\n";

				// not a simple horizontal gradient
				if (angle < 270 || angle > 270)
				{
					m_impl->m_outputSink << "  <svg:linearGradient xlink:href=\"#grad" << m_impl->m_gradientIndex-1 << "\"";
					m_impl->m_outputSink << " id=\"grad" << m_impl->m_gradientIndex++ << "\" ";
					m_impl->m_outputSink << "x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\" ";
					m_impl->m_outputSink << "gradientTransform=\"rotate(" << angle << " .5 .5)\" ";
					m_impl->m_outputSink << "gradientUnits=\"objectBoundingBox\" >\n";
					m_impl->m_outputSink << "  </svg:linearGradient>\n";
				}

				m_impl->m_outputSink << "</svg:defs>\n";
			}
		}
		else
		{
			if (m_impl->m_style["draw:style"] && m_impl->m_style["draw:style"]->getStr() == "radial")
			{
				m_impl->m_outputSink << "<svg:defs>\n";
				m_impl->m_outputSink << "  <svg:radialGradient id=\"grad" << m_impl->m_gradientIndex++ << "\" cx=\"" << m_impl->m_style["svg:cx"]->getStr().cstr() << "\" cy=\"" << m_impl->m_style["svg:cy"]->getStr().cstr() << "\" r=\"" << m_impl->m_style["svg:r"]->getStr().cstr() << "\" >\n";
				for (unsigned c = 0; c < m_impl->m_gradient.count(); c++)
				{
					m_impl->m_outputSink << "    <svg:stop offset=\"" << m_impl->m_gradient[c]["svg:offset"]->getStr().cstr() << "\"";

					m_impl->m_outputSink << " stop-color=\"" << m_impl->m_gradient[c]["svg:stop-color"]->getStr().cstr() << "\"";
					m_impl->m_outputSink << " stop-opacity=\"" << m_impl->m_gradient[c]["svg:stop-opacity"]->getDouble() << "\" />" << std::endl;

				}
				m_impl->m_outputSink << "  </svg:radialGradient>\n";
				m_impl->m_outputSink << "</svg:defs>\n";
			}
			else
			{
				m_impl->m_outputSink << "<svg:defs>\n";
				m_impl->m_outputSink << "  <svg:linearGradient id=\"grad" << m_impl->m_gradientIndex++ << "\" >\n";
				for (unsigned c = 0; c < m_impl->m_gradient.count(); c++)
				{
					m_impl->m_outputSink << "    <svg:stop offset=\"" << m_impl->m_gradient[c]["svg:offset"]->getStr().cstr() << "\"";

					m_impl->m_outputSink << " stop-color=\"" << m_impl->m_gradient[c]["svg:stop-color"]->getStr().cstr() << "\"";
					m_impl->m_outputSink << " stop-opacity=\"" << m_impl->m_gradient[c]["svg:stop-opacity"]->getDouble() << "\" />" << std::endl;

				}
				m_impl->m_outputSink << "  </svg:linearGradient>\n";

				// not a simple horizontal gradient
				if (angle < 270 || angle > 270)
				{
					m_impl->m_outputSink << "  <svg:linearGradient xlink:href=\"#grad" << m_impl->m_gradientIndex-1 << "\"";
					m_impl->m_outputSink << " id=\"grad" << m_impl->m_gradientIndex++ << "\" ";
					m_impl->m_outputSink << "x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\" ";
					m_impl->m_outputSink << "gradientTransform=\"rotate(" << angle << " .5 .5)\" ";
					m_impl->m_outputSink << "gradientUnits=\"objectBoundingBox\" >\n";
					m_impl->m_outputSink << "  </svg:linearGradient>\n";
				}

				m_impl->m_outputSink << "</svg:defs>\n";
			}
		}
	}
	else if (m_impl->m_style["draw:fill"] && m_impl->m_style["draw:fill"]->getStr() == "bitmap")
	{
		if (m_impl->m_style["draw:fill-image"] && m_impl->m_style["libwpg:mime-type"])
		{
			m_impl->m_outputSink << "<svg:defs>\n";
			m_impl->m_outputSink << "  <svg:pattern id=\"img" << m_impl->m_patternIndex++ << "\" patternUnits=\"userSpaceOnUse\" ";
			if (m_impl->m_style["svg:width"])
				m_impl->m_outputSink << "width=\"" << doubleToString(72*(m_impl->m_style["svg:width"]->getDouble())) << "\" ";
			else
				m_impl->m_outputSink << "width=\"100\" ";

			if (m_impl->m_style["svg:height"])
				m_impl->m_outputSink << "height=\"" << doubleToString(72*(m_impl->m_style["svg:height"]->getDouble())) << "\">" << std::endl;
			else
				m_impl->m_outputSink << "height=\"100\">" << std::endl;
			m_impl->m_outputSink << "<svg:image ";

			if (m_impl->m_style["svg:x"])
				m_impl->m_outputSink << "x=\"" << doubleToString(72*(m_impl->m_style["svg:x"]->getDouble())) << "\" ";
			else
				m_impl->m_outputSink << "x=\"0\" ";

			if (m_impl->m_style["svg:y"])
				m_impl->m_outputSink << "y=\"" << doubleToString(72*(m_impl->m_style["svg:y"]->getDouble())) << "\" ";
			else
				m_impl->m_outputSink << "y=\"0\" ";

			if (m_impl->m_style["svg:width"])
				m_impl->m_outputSink << "width=\"" << doubleToString(72*(m_impl->m_style["svg:width"]->getDouble())) << "\" ";
			else
				m_impl->m_outputSink << "width=\"100\" ";

			if (m_impl->m_style["svg:height"])
				m_impl->m_outputSink << "height=\"" << doubleToString(72*(m_impl->m_style["svg:height"]->getDouble())) << "\" ";
			else
				m_impl->m_outputSink << "height=\"100\" ";

			m_impl->m_outputSink << "xlink:href=\"data:" << m_impl->m_style["libwpg:mime-type"]->getStr().cstr() << ";base64,";
			m_impl->m_outputSink << m_impl->m_style["draw:fill-image"]->getStr().cstr();
			m_impl->m_outputSink << "\" />\n";
			m_impl->m_outputSink << "  </svg:pattern>\n";
			m_impl->m_outputSink << "</svg:defs>\n";
		}
	}
}

void RVNGSVGPresentationGenerator::startLayer(const RVNGPropertyList &propList)
{
	m_impl->m_outputSink << "<svg:g id=\"Layer" << propList["svg:id"]->getInt() << "\"";
	if (propList["svg:fill-rule"])
		m_impl->m_outputSink << " fill-rule=\"" << propList["svg:fill-rule"]->getStr().cstr() << "\"";
	m_impl->m_outputSink << " >\n";
}

void RVNGSVGPresentationGenerator::endLayer()
{
	m_impl->m_outputSink << "</svg:g>\n";
}

void RVNGSVGPresentationGenerator::startEmbeddedGraphics(const RVNGPropertyList & /*propList*/)
{
}

void RVNGSVGPresentationGenerator::endEmbeddedGraphics()
{
}

void RVNGSVGPresentationGenerator::openGroup(const RVNGPropertyList &/*propList*/)
{
	// TODO: handle svg:id
	m_impl->m_outputSink << "<svg:g>\n";
}

void RVNGSVGPresentationGenerator::closeGroup()
{
	m_impl->m_outputSink << "</svg:g>\n";
}

void RVNGSVGPresentationGenerator::drawRectangle(const RVNGPropertyList &propList)
{
	m_impl->m_outputSink << "<svg:rect ";
	m_impl->m_outputSink << "x=\"" << doubleToString(72*propList["svg:x"]->getDouble()) << "\" y=\"" << doubleToString(72*propList["svg:y"]->getDouble()) << "\" ";
	m_impl->m_outputSink << "width=\"" << doubleToString(72*propList["svg:width"]->getDouble()) << "\" height=\"" << doubleToString(72*propList["svg:height"]->getDouble()) << "\" ";
	if ((propList["svg:rx"] && propList["svg:rx"]->getInt() !=0) || (propList["svg:ry"] && propList["svg:ry"]->getInt() !=0))
		m_impl->m_outputSink << "rx=\"" << doubleToString(72*propList["svg:rx"]->getDouble()) << "\" ry=\"" << doubleToString(72*propList["svg:ry"]->getDouble()) << "\" ";
	m_impl->writeStyle();
	m_impl->m_outputSink << "/>\n";
}

void RVNGSVGPresentationGenerator::drawEllipse(const RVNGPropertyList &propList)
{
	m_impl->m_outputSink << "<svg:ellipse ";
	m_impl->m_outputSink << "cx=\"" << doubleToString(72*propList["svg:cx"]->getDouble()) << "\" cy=\"" << doubleToString(72*propList["svg:cy"]->getDouble()) << "\" ";
	m_impl->m_outputSink << "rx=\"" << doubleToString(72*propList["svg:rx"]->getDouble()) << "\" ry=\"" << doubleToString(72*propList["svg:ry"]->getDouble()) << "\" ";
	m_impl->writeStyle();
	if (propList["libwpg:rotate"] && propList["libwpg:rotate"]->getDouble() != 0.0)
		m_impl->m_outputSink << " transform=\" translate(" << doubleToString(72*propList["svg:cx"]->getDouble()) << ", " << doubleToString(72*propList["svg:cy"]->getDouble())
		                     << ") rotate(" << doubleToString(-propList["libwpg:rotate"]->getDouble())
		                     << ") translate(" << doubleToString(-72*propList["svg:cx"]->getDouble())
		                     << ", " << doubleToString(-72*propList["svg:cy"]->getDouble())
		                     << ")\" ";
	m_impl->m_outputSink << "/>\n";
}

void RVNGSVGPresentationGenerator::drawPolyline(const RVNGPropertyList &propList)
{
	const RVNGPropertyListVector *vertices = propList.child("svg:points");
	if (vertices && vertices->count())
		m_impl->drawPolySomething(*vertices, false);
}

void RVNGSVGPresentationGenerator::drawPolygon(const RVNGPropertyList &propList)
{
	const RVNGPropertyListVector *vertices = propList.child("svg:points");
	if (vertices && vertices->count())
		m_impl->drawPolySomething(*vertices, true);
}

void RVNGSVGPresentationGeneratorImpl::drawPolySomething(const RVNGPropertyListVector &vertices, bool isClosed)
{
	if (vertices.count() < 2)
		return;

	if (vertices.count() == 2)
	{
		m_outputSink << "<svg:line ";
		m_outputSink << "x1=\"" << doubleToString(72*(vertices[0]["svg:x"]->getDouble())) << "\"  y1=\"" << doubleToString(72*(vertices[0]["svg:y"]->getDouble())) << "\" ";
		m_outputSink << "x2=\"" << doubleToString(72*(vertices[1]["svg:x"]->getDouble())) << "\"  y2=\"" << doubleToString(72*(vertices[1]["svg:y"]->getDouble())) << "\"\n";
		writeStyle();
		m_outputSink << "/>\n";
	}
	else
	{
		if (isClosed)
			m_outputSink << "<svg:polygon ";
		else
			m_outputSink << "<svg:polyline ";

		m_outputSink << "points=\"";
		for (unsigned i = 0; i < vertices.count(); i++)
		{
			m_outputSink << doubleToString(72*(vertices[i]["svg:x"]->getDouble())) << " " << doubleToString(72*(vertices[i]["svg:y"]->getDouble()));
			if (i < vertices.count()-1)
				m_outputSink << ", ";
		}
		m_outputSink << "\"\n";
		writeStyle(isClosed);
		m_outputSink << "/>\n";
	}
}

void RVNGSVGPresentationGenerator::drawPath(const RVNGPropertyList &propList)
{
	const RVNGPropertyListVector *path = propList.child("svg:d");
	if (!path)
		return;
	m_impl->m_outputSink << "<svg:path d=\" ";
	bool isClosed = false;
	unsigned i=0;
	for (i=0; i < path->count(); i++)
	{
		RVNGPropertyList pList((*path)[i]);
		if (pList["libwpg:path-action"] && pList["libwpg:path-action"]->getStr() == "M")
		{
			m_impl->m_outputSink << "\nM";
			m_impl->m_outputSink << doubleToString(72*(pList["svg:x"]->getDouble())) << "," << doubleToString(72*(pList["svg:y"]->getDouble()));
		}
		else if (pList["libwpg:path-action"] && pList["libwpg:path-action"]->getStr() == "L")
		{
			m_impl->m_outputSink << "\nL";
			m_impl->m_outputSink << doubleToString(72*(pList["svg:x"]->getDouble())) << "," << doubleToString(72*(pList["svg:y"]->getDouble()));
		}
		else if (pList["libwpg:path-action"] && pList["libwpg:path-action"]->getStr() == "C")
		{
			m_impl->m_outputSink << "\nC";
			m_impl->m_outputSink << doubleToString(72*(pList["svg:x1"]->getDouble())) << "," << doubleToString(72*(pList["svg:y1"]->getDouble())) << " ";
			m_impl->m_outputSink << doubleToString(72*(pList["svg:x2"]->getDouble())) << "," << doubleToString(72*(pList["svg:y2"]->getDouble())) << " ";
			m_impl->m_outputSink << doubleToString(72*(pList["svg:x"]->getDouble())) << "," << doubleToString(72*(pList["svg:y"]->getDouble()));
		}
		else if (pList["libwpg:path-action"] && pList["libwpg:path-action"]->getStr() == "Q")
		{
			m_impl->m_outputSink << "\nQ";
			m_impl->m_outputSink << doubleToString(72*(pList["svg:x1"]->getDouble())) << "," << doubleToString(72*(pList["svg:y1"]->getDouble())) << " ";
			m_impl->m_outputSink << doubleToString(72*(pList["svg:x"]->getDouble())) << "," << doubleToString(72*(pList["svg:y"]->getDouble()));
		}
		else if (pList["libwpg:path-action"] && pList["libwpg:path-action"]->getStr() == "A")
		{
			m_impl->m_outputSink << "\nA";
			m_impl->m_outputSink << doubleToString(72*(pList["svg:rx"]->getDouble())) << "," << doubleToString(72*(pList["svg:ry"]->getDouble())) << " ";
			m_impl->m_outputSink << doubleToString(pList["libwpg:rotate"] ? pList["libwpg:rotate"]->getDouble() : 0) << " ";
			m_impl->m_outputSink << (pList["libwpg:large-arc"] ? pList["libwpg:large-arc"]->getInt() : 1) << ",";
			m_impl->m_outputSink << (pList["libwpg:sweep"] ? pList["libwpg:sweep"]->getInt() : 1) << " ";
			m_impl->m_outputSink << doubleToString(72*(pList["svg:x"]->getDouble())) << "," << doubleToString(72*(pList["svg:y"]->getDouble()));
		}
		else if ((i >= path->count()-1 && i > 2) && pList["libwpg:path-action"] && pList["libwpg:path-action"]->getStr() == "Z")
		{
			isClosed = true;
			m_impl->m_outputSink << "\nZ";
		}
	}

	m_impl->m_outputSink << "\" \n";
	m_impl->writeStyle(isClosed);
	m_impl->m_outputSink << "/>\n";
}

void RVNGSVGPresentationGenerator::drawGraphicObject(const RVNGPropertyList &propList)
{
	if (!propList["libwpg:mime-type"] || propList["libwpg:mime-type"]->getStr().len() <= 0)
		return;
	if (!propList["office:binary-data"])
		return;
	m_impl->m_outputSink << "<svg:image ";
	if (propList["svg:x"] && propList["svg:y"] && propList["svg:width"] && propList["svg:height"])
		m_impl->m_outputSink << "x=\"" << doubleToString(72*(propList["svg:x"]->getDouble())) << "\" y=\"" << doubleToString(72*(propList["svg:y"]->getDouble())) << "\" ";
	m_impl->m_outputSink << "width=\"" << doubleToString(72*(propList["svg:width"]->getDouble())) << "\" height=\"" << doubleToString(72*(propList["svg:height"]->getDouble())) << "\" ";
	m_impl->m_outputSink << "xlink:href=\"data:" << propList["libwpg:mime-type"]->getStr().cstr() << ";base64,";
	m_impl->m_outputSink << propList["office:binary-data"]->getStr().cstr();
	m_impl->m_outputSink << "\" />\n";
}

void RVNGSVGPresentationGenerator::drawConnector(const RVNGPropertyList &/*propList*/)
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::startTextObject(const RVNGPropertyList &propList)
{
	m_impl->m_outputSink << "<svg:text ";
	if (propList["svg:x"] && propList["svg:y"])
		m_impl->m_outputSink << "x=\"" << doubleToString(72*(propList["svg:x"]->getDouble())) << "\" y=\"" << doubleToString(72*(propList["svg:y"]->getDouble())) << "\"";
	if (propList["libwpg:rotate"] && propList["libwpg:rotate"]->getDouble() != 0.0)
		m_impl->m_outputSink << " transform=\"translate(" << doubleToString(72*propList["svg:x"]->getDouble()) << ", " << doubleToString(72*propList["svg:y"]->getDouble())
		                     << ") rotate(" << doubleToString(-propList["libwpg:rotate"]->getDouble())
		                     << ") translate(" << doubleToString(-72*propList["svg:x"]->getDouble())
		                     << ", " << doubleToString(-72*propList["svg:y"]->getDouble())
		                     << ")\"";
	m_impl->m_outputSink << ">\n";

}

void RVNGSVGPresentationGenerator::endTextObject()
{
	m_impl->m_outputSink << "</svg:text>\n";
}

void RVNGSVGPresentationGenerator::defineCharacterStyle(const RVNGPropertyList &propList)
{
	if (!propList["librevenge:span-id"])
	{
		RVNG_DEBUG_MSG(("RVNGSVGPresentationGenerator::defineCharacterStyle: can not find the span-id\n"));
		return;
	}
	m_impl->m_idSpanMap[propList["librevenge:span-id"]->getInt()]=propList;
}

void RVNGSVGPresentationGenerator::openSpan(const RVNGPropertyList &propList)
{
	RVNGPropertyList pList(propList);
	if (propList["librevenge:span-id"] &&
	        m_impl->m_idSpanMap.find(propList["librevenge:span-id"]->getInt())!=m_impl->m_idSpanMap.end())
		pList=m_impl->m_idSpanMap.find(propList["librevenge:span-id"]->getInt())->second;

	m_impl->m_outputSink << "<svg:tspan ";
	if (pList["style:font-name"])
		m_impl->m_outputSink << "font-family=\"" << pList["style:font-name"]->getStr().cstr() << "\" ";
	if (pList["fo:font-style"])
		m_impl->m_outputSink << "font-style=\"" << pList["fo:font-style"]->getStr().cstr() << "\" ";
	if (pList["fo:font-weight"])
		m_impl->m_outputSink << "font-weight=\"" << pList["fo:font-weight"]->getStr().cstr() << "\" ";
	if (pList["fo:font-variant"])
		m_impl->m_outputSink << "font-variant=\"" << pList["fo:font-variant"]->getStr().cstr() << "\" ";
	if (pList["fo:font-size"])
		m_impl->m_outputSink << "font-size=\"" << doubleToString(pList["fo:font-size"]->getDouble()) << "\" ";
	if (pList["fo:color"])
		m_impl->m_outputSink << "fill=\"" << pList["fo:color"]->getStr().cstr() << "\" ";
	if (pList["fo:text-transform"])
		m_impl->m_outputSink << "text-transform=\"" << pList["fo:text-transform"]->getStr().cstr() << "\" ";
	if (pList["svg:fill-opacity"])
		m_impl->m_outputSink << "fill-opacity=\"" << doubleToString(pList["svg:fill-opacity"]->getDouble()) << "\" ";
	if (pList["svg:stroke-opacity"])
		m_impl->m_outputSink << "stroke-opacity=\"" << doubleToString(pList["svg:stroke-opacity"]->getDouble()) << "\" ";
	m_impl->m_outputSink << ">\n";
}

void RVNGSVGPresentationGenerator::closeSpan()
{
	m_impl->m_outputSink << "</svg:tspan>\n";
}

void RVNGSVGPresentationGenerator::openLink(const RVNGPropertyList & /* propList */)
{
}

void RVNGSVGPresentationGenerator::closeLink()
{
}

void RVNGSVGPresentationGenerator::insertTab()
{
	m_impl->m_outputSink << '\t';
}

void RVNGSVGPresentationGenerator::insertSpace()
{
	m_impl->m_outputSink << ' ';
}

void RVNGSVGPresentationGenerator::insertText(const RVNGString &str)
{
	m_impl->m_outputSink << RVNGString::escapeXML(str).cstr() << "\n";
}

void RVNGSVGPresentationGenerator::insertLineBreak()
{
	m_impl->m_outputSink << '\n';
}

void RVNGSVGPresentationGenerator::insertField(const RVNGPropertyList &/*propList*/)
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::openOrderedListLevel(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::openUnorderedListLevel(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::closeOrderedListLevel()
{
}

void RVNGSVGPresentationGenerator::closeUnorderedListLevel()
{
}

void RVNGSVGPresentationGenerator::openListElement(const RVNGPropertyList &propList)
{
	openParagraph(propList);
}

void RVNGSVGPresentationGenerator::closeListElement()
{
	closeParagraph();
}

void RVNGSVGPresentationGenerator::defineParagraphStyle(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::openParagraph(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::closeParagraph()
{
	m_impl->m_outputSink << '\n';
}

void RVNGSVGPresentationGenerator::startTableObject(const RVNGPropertyList &/*propList*/)
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::openTableRow(const RVNGPropertyList &/*propList*/)
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::closeTableRow()
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::openTableCell(const RVNGPropertyList &/*propList*/)
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::closeTableCell()
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::insertCoveredTableCell(const RVNGPropertyList &/*propList*/)
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::endTableObject()
{
	// TODO: implement me
}


void RVNGSVGPresentationGenerator::startComment(const RVNGPropertyList &/*propList*/)
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::endComment()
{
	// TODO: implement me
}


void RVNGSVGPresentationGenerator::startNotes(const RVNGPropertyList &/*propList*/)
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::endNotes()
{
	// TODO: implement me
}

void RVNGSVGPresentationGenerator::defineChartStyle(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::openChart(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::closeChart()
{
}

void RVNGSVGPresentationGenerator::openChartTextObject(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::closeChartTextObject()
{
}

void RVNGSVGPresentationGenerator::openChartPlotArea(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::closeChartPlotArea()
{
}

void RVNGSVGPresentationGenerator::insertChartAxis(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::openChartSeries(const librevenge::RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::closeChartSeries()
{
}

// create "style" attribute based on current pen and brush
void RVNGSVGPresentationGeneratorImpl::writeStyle(bool /* isClosed */)
{
	m_outputSink << "style=\"";

	if (m_style["svg:stroke-width"])
	{
		double width = m_style["svg:stroke-width"]->getDouble();
		if (width == 0.0 && m_style["draw:stroke"] && m_style["draw:stroke"]->getStr() != "none")
			width = 0.2 / 72.0; // reasonable hairline
		m_outputSink << "stroke-width: " << doubleToString(72*width) << "; ";
	}
	if ((m_style["draw:stroke"] && m_style["draw:stroke"]->getStr() != "none"))
	{
		if (m_style["svg:stroke-color"])
			m_outputSink << "stroke: " << m_style["svg:stroke-color"]->getStr().cstr()  << "; ";
		if (m_style["svg:stroke-opacity"] &&  m_style["svg:stroke-opacity"]->getInt()!= 1)
			m_outputSink << "stroke-opacity: " << doubleToString(m_style["svg:stroke-opacity"]->getDouble()) << "; ";
	}

	if (m_style["draw:stroke"] && m_style["draw:stroke"]->getStr() == "solid")
		m_outputSink << "stroke-dasharray:  solid; ";
	else if (m_style["draw:stroke"] && m_style["draw:stroke"]->getStr() == "dash")
	{
		int dots1 = m_style["draw:dots1"]->getInt();
		int dots2 = m_style["draw:dots2"]->getInt();
		double dots1len = m_style["draw:dots1-length"]->getDouble();
		double dots2len = m_style["draw:dots2-length"]->getDouble();
		double gap = m_style["draw:distance"]->getDouble();
		m_outputSink << "stroke-dasharray: ";
		for (int i = 0; i < dots1; i++)
		{
			if (i)
				m_outputSink << ", ";
			m_outputSink << (int)dots1len;
			m_outputSink << ", ";
			m_outputSink << (int)gap;
		}
		for (int j = 0; j < dots2; j++)
		{
			m_outputSink << ", ";
			m_outputSink << (int)dots2len;
			m_outputSink << ", ";
			m_outputSink << (int)gap;
		}
		m_outputSink << "; ";
	}

	if (m_style["svg:stroke-linecap"])
		m_outputSink << "stroke-linecap: " << m_style["svg:stroke-linecap"]->getStr().cstr() << "; ";

	if (m_style["svg:stroke-linejoin"])
		m_outputSink << "stroke-linejoin: " << m_style["svg:stroke-linejoin"]->getStr().cstr() << "; ";

	if (m_style["draw:fill"] && m_style["draw:fill"]->getStr() == "none")
		m_outputSink << "fill: none; ";
	else if (m_style["svg:fill-rule"])
		m_outputSink << "fill-rule: " << m_style["svg:fill-rule"]->getStr().cstr() << "; ";

	if (m_style["draw:fill"] && m_style["draw:fill"]->getStr() == "gradient")
		m_outputSink << "fill: url(#grad" << m_gradientIndex-1 << "); ";

	if (m_style["draw:fill"] && m_style["draw:fill"]->getStr() == "bitmap")
		m_outputSink << "fill: url(#img" << m_patternIndex-1 << "); ";

	if (m_style["draw:shadow"] && m_style["draw:shadow"]->getStr() == "visible")
		m_outputSink << "filter:url(#shadow" << m_shadowIndex-1 << "); ";

	if (m_style["draw:fill"] && m_style["draw:fill"]->getStr() == "solid")
		if (m_style["draw:fill-color"])
			m_outputSink << "fill: " << m_style["draw:fill-color"]->getStr().cstr() << "; ";
	if (m_style["draw:opacity"] && m_style["draw:opacity"]->getDouble() < 1)
		m_outputSink << "fill-opacity: " << doubleToString(m_style["draw:opacity"]->getDouble()) << "; ";
	m_outputSink << "\""; // style
}

void RVNGSVGPresentationGenerator::openAnimationSequence(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::closeAnimationSequence()
{
}

void RVNGSVGPresentationGenerator::openAnimationGroup(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::closeAnimationGroup()
{
}

void RVNGSVGPresentationGenerator::openAnimationIteration(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::closeAnimationIteration()
{
}

void RVNGSVGPresentationGenerator::insertMotionAnimation(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::insertColorAnimation(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::insertAnimation(const RVNGPropertyList &)
{
}

void RVNGSVGPresentationGenerator::insertEffect(const RVNGPropertyList &)
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
