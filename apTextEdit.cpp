/*
 * (C) 2015 audioprog@users.noreply.github.com
 *
 ***************************************************************************
 *   This is free software; you can redistribute it and/or modify		   *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This software is distributed in the hope that it will be useful, but  *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include "apTextEdit.h"

#include <QPainter>
#include <QTextDocument>
#include <QTextLayout>

apTextEdit::apTextEdit(QObject *parent)
	: QObject(parent)
    , m_isEmptyRegion(false)
	, m_isRegionDirty(true)
	, m_isVisible(true)
{
}

apTextEdit::~apTextEdit()
{
}

void apTextEdit::measure(QPainter* painter, apTextParagraph& paragraph, int countSections)
{
    for (int iSection = 0; iSection < countSections; iSection++)
    {
        STextSection& section = paragraph.sections[iSection];

        if ( ! section.isMeasured)
        {
            section.isMeasured = true;

            QPainterPath path;
            path.addText(0.0, 0.0, section.font, section.text);
            path.boundingRect();
            painter->setFont(section.font);

            section.size = painter->boundingRect(this->rect, Qt::TextSingleLine, section.text).size();
            section.minSize = painter->boundingRect(this->rect, Qt::TextSingleLine, section.text.trimmed()).size();
            section.spaceAfter = section.size.width() - section.minSize.width();
            section.ascent = QFontMetrics(painter->font()).ascent();
        }
    }

    paragraph.m_isMeasured = true;
}

QRect apTextEdit::searchLineFullHeight(const QRect& regionRect, QRect& lineRect, QRegion& lineRegion)
{
    QRect innerLineRect = lineRegion.boundingRect();
    while (lineRect.bottom() <= regionRect.bottom()
           && innerLineRect.height() < lineRect.height())
    {
        int y = innerLineRect.bottom() == lineRect.bottom() ? innerLineRect.top() : lineRect.bottom();
        lineRect = QRect(this->rect.x(), y, this->rect.width(), lineRect.height());
        lineRegion = this->region.intersected(lineRect);
        innerLineRect = lineRegion.boundingRect();
    }

    return innerLineRect;
}

QSize apTextEdit::detWordSize(const QList<STextSection>& sections, int sectionIndexStart, int& wordLastSectionIndex)
{
    QSize wordSize = sections.at(sectionIndexStart).minSize;
    if ( !sections.at(sectionIndexStart).text.endsWith(' '))
    {
        for (int i = sectionIndexStart + 1; i < sections.count(); i++)
        {
            wordSize.setWidth(wordSize.width() + sections.at(i).minSize.width());
            if (wordSize.height() < sections.at(i).size.height())
            {
                wordSize.setHeight(sections.at(i).size.height());
            }
            wordLastSectionIndex = i;
            if (sections.at(i).text.endsWith(' '))
            {
                break;
            }
        }
    }

    return wordSize;
}

QRect apTextEdit::detWordRect(const QRect &regionRect, const QPoint& startPoint, const QSize& wordSize, QRect &lineRect, QRegion &lineRegion, QRect &innerLineRect)
{
    QRect wordRect(startPoint, wordSize);
    QRegion wordRegion = lineRegion.intersected(wordRect);
    QRegion testRegion = wordRegion.xored(wordRect);
    while ( ! testRegion.isEmpty())
    {
        wordRect.setX(testRegion.boundingRect().right());
        wordRegion = lineRegion.intersected(wordRect);
        testRegion = wordRegion.xored(wordRect);
        if (wordRect.right() > innerLineRect.right())
        {
            lineRect = QRect(this->rect.x(), lineRect.y() + 1, this->rect.width(), wordSize.height());
            if (lineRect.bottom() > regionRect.bottom())
            {
                this->m_isEmptyRegion = true;
                break;
            }
            lineRegion = this->region.intersected(lineRect);

            innerLineRect = searchLineFullHeight(regionRect, lineRect, lineRegion);
        }
    }

    return wordRect;
}

void apTextEdit::paint(QPainter* painter)
{
    QTextDocument* doc = NULL;

    QTextBlock block = doc->begin();

    for (; block.isValid() && block != doc->end(); )
    {
        QTextLayout* textLayout = block.layout();
        textLayout->beginLayout();

        QTextLine line = textLayout->createLine();
        line.setLeadingIncluded(false);

        while (line.isValid()) {

            // We use lines at the top and bottom of the planned
            // location for the text.
            QLineF topLine = QLineF(0, y, width(), y);
            QLineF bottomLine = topLine;
            bottomLine.translate(0, line.height());

            QRegion topLineRegion(this->rect.x(), this->rect.y(), line.rect().width(), 1);
            QRegion topLineRegions = this->region.intersected(topLineRegion);

            QRegion bottomLineRegion(this->rect.x(), this->rect.y() + line.height(), line.rect().width(), 1);
            QRegion bottomLineRegions = this->region.intersected(bottomLineRegion);

            QRegion mergedRegion = topLineRegions.intersected(bottomLineRegions.translated(0, -line.height()));
            if ( ! mergedRegion.isEmpty())
            {
                QVector<QRect> rects = mergedRegion.rects();

                QRect currentRect = rects.first();
                for (int i = 1; i < rects.count(); i++)
                {
                    const QRect& rect = rects[i];
                    if (currentRect.right() + 1 == rect.x())
                    {
                        currentRect |= rect;
                    }
                    else
                    {
                        line.setLineWidth(currentRect.width());

                        QRegion lineRegion(currentRect.x(), currentRect.y(), line.width(), line.height());
                        QRegion intersectionLineRegion = this->region.intersected(lineRegion);
                        QRegion xoredRegion = intersectionLineRegion.xored(lineRegion);
                        if ( ! xoredRegion.isEmpty())
                        {
                            //
                        }
                        currentRect = rect;
                    }
                }
            // Obtain all x-coordinates where intersections occur.
            QVector<qreal> xCoords;

            foreach (QPolygonF polygon, polygons) {

                for (int i = 0; (i+1) < polygon.size(); ++i) {
                    QLineF pLine = QLineF(polygon[i], polygon[i+1]);
                    QPointF p;
                    if (pLine.intersect(topLine, &p) == QLineF::BoundedIntersection)
                        xCoords.append(p.x());
                    if (pLine.intersect(bottomLine, &p) == QLineF::BoundedIntersection)
                        xCoords.append(p.x());
                }
            }

            // If intersections occurred, sort them and use the innermost
            // x-coordinates as horizontal delimiters to mark the area in
            // which text can be written.
            qreal left;
            qreal right;

            if (xCoords.size() > 0 && (xCoords.size() % 2) == 0) {
                qSort(xCoords.begin(), xCoords.end());

                left = xCoords[xCoords.size()/2 - 1] + margin;
                right = xCoords[xCoords.size()/2] - margin;

                line.setPosition(QPointF(left, y));
                line.setLineWidth(right - left);

                y += line.height();

                // If the text is wider than the available space, move the
                // text onto the next line if there is space.
                if (line.naturalTextWidth() <= (right - left) && y <= ymax)
                    line = textLayout->createLine();
            } else
                y += lineHeight;

            // Break if there isn't enough space for another line.
            if (y + lineHeight > ymax)
                break;
        }

        if (line.isValid())
        {
            line.setPosition(QPointF(xScale * shapeWidth, yScale * shapeHeight));

            textLayout->endLayout();
            layouts.append(textLayout);

            // Since we are going to be laying out the next paragraph, we can
            // increase the y-coordinate to insert some extra space.
            y += line.leading();
        }
        else
        {
            if (y + lineHeight > ymax)
            break;
        }

        block = block.next();
    }
}

void apTextEdit::paintSimple()
{
    int startSection = 0;
    if (startSection < countSections)
    {
        int sectLineWidth = 0;
        int indexSection = 0;
        QRegion lineRegion;
        QRegion wordRegion;
        int lineY = y;
        int maxAscent = 0;
        int wordMaxAscent = 0;
        int wordWidth = 0;
        int wordStartIndex = 0;

        // eine Zeile ohne RegionTest
        for (indexSection = startSection; indexSection < countSections; indexSection++)
        {
            const STextSection& section = paragraph.sections.at(indexSection);

            int lineWidth = sectLineWidth + wordWidth + section.minSize.width();

            if (sectLineWidth < regionRect.width())
            {
                sectLineWidth = lineWidth;
                indexSection--;
                break;
            }
            else
            {
                if (section.minSize.width() == section.size.width())
                {
                    if (wordMaxAscent == 0)
                    {
                        wordMaxAscent = qMax(maxAscent, section.ascent);

                        section.pos = QPoint(regionRect.x() + sectLineWidth, lineY + wordMaxAscent - section.ascent);

                        wordRegion = QRegion(section.pos.x(), section.pos.y(), section.size.width(), section.size.height());

                        wordWidth = section.size.width();

                        section.yInLine = wordMaxAscent;

                        wordStartIndex = indexSection;
                    }
                    else if (section.ascent > wordMaxAscent)
                    {
                        int yTranslate = section.ascent - wordMaxAscent;

                        wordRegion.translate(0, yTranslate);

                        for (int i = wordStartIndex; i < indexSection; i++)
                        {
                            paragraph.sections.at(i).yInLine += yTranslate;
                            paragraph.sections.at(i).pos.ry() += yTranslate;
                        }

                        wordMaxAscent = section.ascent;

                        section.pos = QPoint(regionRect.x() + sectLineWidth + wordWidth, lineY);

                        wordRegion += QRect(section.pos.x(), lineY, section.size.width(), section.size.height());

                        wordWidth += section.size.width();
                    }
                    else
                    {
                        section.pos = QPoint(regionRect.x() + sectLineWidth + wordWidth, lineY + wordMaxAscent - section.ascent);

                        wordRegion += QRect(section.pos.x(), section.pos.y(), section.size.width(), section.size.height());

                        wordWidth += section.size.width();
                    }

                    section.yInLine = wordMaxAscent;
                }
                else if (wordWidth > 0)
                {
                    if (section.ascent > wordMaxAscent)
                    {
                        int yTranslate = section.ascent - wordMaxAscent;

                        wordRegion.translate(0, yTranslate);

                        for (int i = wordStartIndex; i < indexSection; i++)
                        {
                            paragraph.sections.at(i).yInLine += yTranslate;
                            paragraph.sections.at(i).pos.ry() += yTranslate;
                        }

                        wordMaxAscent = section.ascent;

                        section.pos = QPoint(regionRect.x() + sectLineWidth + wordWidth, lineY);

                        wordRegion += QRect(section.pos.x(), lineY, section.minSize.width(), section.size.height());
                    }
                    else
                    {
                        section.pos = QPoint(regionRect.x() + sectLineWidth + wordWidth, lineY + wordMaxAscent - section.ascent);

                        wordRegion += QRect(section.pos.x(), section.pos.y(), section.minSize.width(), section.size.height());
                    }

                    wordWidth += section.size.width();

                    section.yInLine = wordMaxAscent;

                    if (maxAscent < wordMaxAscent)
                    {
                        int yTranslate = wordMaxAscent - maxAscent;

                        lineRegion.translate(0, yTranslate);

                        for (int i = startSection; i <= indexSection; i++)
                        {
                            paragraph.sections.at(i).yInLine += yTranslate;
                            paragraph.sections.at(i).pos.ry() += yTranslate;
                        }

                        lineRegion += wordRegion;

                        maxAscent = wordMaxAscent;
                    }
                    else
                    {
                        lineRegion += wordRegion;
                    }

                    sectLineWidth += wordWidth + section.spaceAfter;

                    wordWidth = 0;
                }
                else if (section.ascent < maxAscent)
                {
                    int yTranslate = section.ascent - maxAscent;

                    maxAscent = section.ascent;

                    lineRegion.translate(0, yTranslate);

                    for (int i = startSection; i < indexSection; i++)
                    {
                        paragraph.sections.at(i).yInLine += yTranslate;
                        paragraph.sections.at(i).pos.ry() += yTranslate;
                    }

                    section.yInLine = maxAscent;

                    section.pos = QPoint(regionRect.x() + sectLineWidth, lineY);

                    lineRegion += QRect(section.pos.x(), lineY, section.minSize.width(), section.minSize.height());

                    sectLineWidth += section.size.width();
                }
                else
                {
                    section.yInLine = maxAscent;

                    section.pos = QPoint(regionRect.x() + sectLineWidth, lineY + maxAscent - section.ascent);

                    lineRegion += QRect(section.pos.x(), section.pos.y(), section.minSize.width(), section.minSize.height());

                    sectLineWidth += section.size.width();
                }
            }
        }
    }
}
