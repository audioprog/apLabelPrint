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
	, m_isRegionDirty(true)
	, m_isVisible(true)
{
}

apTextEdit::~apTextEdit()
{
}

void apTextEdit::paint(QPainter* painter)
{
	int countParagraphs = this->paragraphs;
	for (int iParagraph = 0; iParagraph < countParagraphs; iParagraph++)
	{
		lsTextParagraph& paragraph = this->paragraphs[iParagraph];

		int countSections = paragraph.sections.count();

		if ( ! paragraph.isMeasured())
		{
			for (int iSection = 0; iSection < countSections; iSection++)
			{
				STextSection& section = paragraph.sections[iSection];

				if ( ! section.isMeasured)
				{
					section.isMeasured = true;

					painter->setFont(section.font);

					section.size = painter->boundingRect(this->rect, Qt::TextSingleLine, section.text);
					section.minSize = painter->boundingRect(this->rect, Qt::TextSingleLine, section.text.trimmed());
				}
			}

			paragraph.isMeasured = true;
		}


		QPointF startPoint;
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

			QRect lineRect(this->rect.x(), y, this->rect.width(), paragraph.sections.first().size.height());
			QRegion lineRegion = this->region.intersected(lineRect);

			QRect innerLineRect = lineRegion.boundingRect();
			if (innerLineRect.height() < lineRect.height())
			{
				//
			}

			double width = 0.0;
			int maxLineCount = 0;
			double maxHeight = innerLineRect.height();

			for (int i = 0; i < paragraph.sections.count(); i++)
			{
				width += paragraph.sections.at(i).size.width();
				if (width > innerLineRect.width())
				{
					maxLineCount = i;
					break;
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
				bool isSearching = true;
				while (isSearching)
				{
					QRect testRect = QRect(QPoint(x, y), paragraph.sections.at(iSection).minSize);
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

