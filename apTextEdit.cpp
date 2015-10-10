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
    int countParagraphs = this->paragraphs.count();
	for (int iParagraph = 0; iParagraph < countParagraphs; iParagraph++)
	{
		apTextParagraph& paragraph = this->paragraphs[iParagraph];

		int countSections = paragraph.sections.count();

		if ( ! paragraph.isMeasured())
		{
            this->measure(painter, paragraph, countSections);
		}


        QPoint startPoint;
		if (this->m_isRegionDirty)
		{
			QRegion rectRegion = this->region.intersected(this->rect);
			if (rectRegion.isEmpty())
			{
				this->m_isEmptyRegion = true;
				return;
			}
			QRect regionRect = rectRegion.boundingRect();
			if (regionRect.height() < paragraph.sections.first().size.height())
			{
				this->m_isEmptyRegion = true;
				return;
			}

            int y = regionRect.y();

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

                // An Region anpassen
                switch (paragraph.alignment)
                {
                case Qt::AlignLeft:
                    {
                        QRegion testLine = rectRegion.intersected(lineRegion).xored(lineRegion);

                        if ( ! testLine.isEmpty())
                        {
                            for (int i = indexSection - 1; i >= startSection; i--)
                            {
                                if (paragraph.sections.at(i).spaceAfter == 0)
                                {
                                    const STextSection& section = paragraph.sections.at(i + 1);
                                    wordWidth = section.size.width();
                                    QRegion wordRegion(section.pos.x(), section.pos.y(), section.minSize.width(), section.minSize.height());
                                    for (int j = i + 2; j < indexSection; j++)
                                    {
                                        const QPoint& pos = paragraph.sections.at(j).pos;
                                        const QSize& size = paragraph.sections.at(j).minSize;
                                        wordRegion += QRect(pos.x(), pos.y(), size.width(), size.height());
                                        wordWidth += paragraph.sections.at(j).size.width();
                                    }
                                    if (wordRegion.intersects(testLine))
                                    {
                                        lineRegion -= wordRegion;
                                        indexSection = i;
                                        sectLineWidth -= wordWidth;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }

                            testLine = rectRegion.intersected(lineRegion).xored(lineRegion);

                            if ( ! testLine.isEmpty())
                            {
                                int xTranslate = testLine.boundingRect().right();
                                //
                            }
                        }
                    }
                    break;
                }


			QRect lineRect(this->rect.x(), y, this->rect.width(), paragraph.sections.first().size.height());
			QRegion lineRegion = this->region.intersected(lineRect);

            QRect innerLineRect = searchLineFullHeight(regionRect, lineRect, lineRegion);

            if (lineRect.bottom() > regionRect.bottom())
            {
                this->m_isEmptyRegion = true;
                return;
            }

            y = innerLineRect.y();

            int wordLastSectionIndex = 0;
            QSize firstWordSize = detWordSize(this->paragraphs.first().sections, 0, wordLastSectionIndex);

            QRect wordRect = detWordRect(regionRect, innerLineRect.topLeft(), firstWordSize, lineRect, lineRegion, innerLineRect);
            if (this->m_isEmptyRegion)
            {
                return;
            }

            y = wordRect.y();

            while (!this->m_isEmptyRegion && wordLastSectionIndex < this->paragraphs.first().sections.count())
            {
                wordRect.moveRight(this->paragraphs.first().sections.at(wordLastSectionIndex).size.width() - this->paragraphs.first().sections.at(wordLastSectionIndex).minSize.width());

                for (int i = wordStartSectionIndex; i <= wordLastSectionIndex; i++)
                {

                }

                int wordStartSectionIndex = wordLastSectionIndex;
                QSize wordSize = detWordSize(this->paragraphs.first().sections, wordStartSectionIndex + 1, wordLastSectionIndex);

                QRect nextWordRect = detWordRect(regionRect, QPoint(wordRect.right(), innerLineRect.top()), wordSize, lineRect, lineRegion, innerLineRect);
            }

//////////////////////////////////////////////////////////
            int width = 0;
			int maxLineCount = 0;
            int maxHeight = innerLineRect.height();
            uint maxAscent = 0;

			for (int i = 0; i < paragraph.sections.count(); i++)
			{
				width += paragraph.sections.at(i).size.width();
				if (width > innerLineRect.width())
				{
					maxLineCount = i;
					break;
				}
                else
                {
                    uint ascent = paragraph.sections.at(i).ascent;
                    if (ascent > maxAscent)
                    {
                        maxAscent = ascent;
                    }
                }
				if (maxHeight < paragraph.sections.at(i).size.height())
				{
					maxHeight = paragraph.sections.at(i).size.height();
				}
			}
			lineRect = QRect(this->rect.x(), y, this->rect.width(), maxHeight);
			lineRegion = this->region.intersected(lineRect);
			innerLineRect = lineRegion.boundingRect();

			int x = innerLineRect.x();
			int right = x + innerLineRect.width();

			int lastLineSection = 0;

			for (int iSection = 0; iSection < paragraph.sections.count(); iSection++)
			{
                paragraph.sections.at(iSection).yInLine = maxAscent - paragraph.sections.at(iSection).ascent;
				bool isSearching = true;
				while (isSearching)
				{
                    QRect testRect = QRect(QPoint(x, y + paragraph.sections.at(iSection).yInLine), paragraph.sections.at(iSection).minSize);
					QRegion singleRegion = lineRegion.intersected(testRect);
					if (singleRegion.isEmpty())
					{
						x += paragraph.sections.at(isSearching).minSize;
					}
					else
					{
						QRegion insertedSingleRegion = singleRegion.xored(testRect);
						if (insertedSingleRegion.isEmpty())
						{
							paragraph.sections.at(iSection).pos = testRect.topLeft();

							x += paragraph.sections.at(iSection).size.width();

							isSearching = false;
						}
						else
						{
							x += insertedSingleRegion.boundingRect().width();
						}
					}

					if (x > right)
					{
						if (iSection == lastLineSection)
						{
							y += 5;
						}
						else
						{
							y += lineRect.height();
						}

						lineRect = QRect(this->rect.x(), y, this->rect.width(), paragraph.sections.at(isSearching).size.height());
						lineRegion = this->region.intersected(lineRect);

						innerLineRect = lineRegion.boundingRect();

						x = innerLineRect.x();
						right = x + innerLineRect.width();

						lastLineSection = 0;
					}
				}
			QRect innnerRect = lineRegion.boundingRect();
			if (lineRegion.y() > this->rect)
		}
	}
}

