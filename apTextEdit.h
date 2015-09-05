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
#ifndef LSTEXTEDIT_H
#define LSTEXTEDIT_H

#include <QObject>

#include <QColor>
#include <QFont>
#include <QRect>
#include <QRegion>
#include <QVariant>

class QContextMenuEvent;
class QKeyEvent;
class QPainter;

struct STextSection
{
    STextSection() : ascent(0), yInLine(0), isMeasured(false) {}

    uint ascent;

	QColor color;

	QFont font;

	bool isMeasured;

    QSize minSize;

    QPoint pos;

    uint yInLine;

    QSize size;

	QVariant sourceId;

	QString text;
};

class apTextParagraph
{
	friend class apTextEdit;

public:
	apTextParagraph() : alignment(Qt::AlignLeft) {}

    bool isMeasured() const { return m_isMeasured; }

    inline void setMeasureDirty() { for (int i = 0; i < sections.count(); i++) { sections[i].isMeasured = false; } m_isMeasured = false; }

	Qt::Alignment alignment;

	QList<STextSection> sections;

private:
    bool m_isMeasured;
};

class apTextEdit : public QObject
{
    Q_OBJECT

public:
	explicit apTextEdit(QObject *parent = 0);
	~apTextEdit();

	bool isVisible() const { return this->m_isVisible; }

	void paint( QPainter* painter );

	void setVisible( bool visible ) {
		this->m_isVisible = visible;
	}

	QRectF usedArea() const { return this->m_usedArea; }

protected:
	void mousePressEvent( const QPoint& pos );
	void mouseReleaseEvent( const QPoint& pos );
	void mouseDoubleClickEvent( const QPoint& pos );
	void mouseMoveEvent( const QPoint& pos );
	void keyPressEvent(QKeyEvent*);
	void keyReleaseEvent(QKeyEvent*);
	void enterEvent();
	void leaveEvent();
	void contextMenuEvent(QContextMenuEvent*);

    QRect detWordRect(const QRect& regionRect, const QPoint& startPoint, const QSize& wordSize, QRect& lineRect, QRegion& lineRegion, QRect& innerLineRect);

private:
    QSize detWordSize(const QList<STextSection>& sections, int sectionIndexStart, int& wordLastSectionIndex);

    void measure(QPainter* painter, apTextParagraph& paragraph, int countSections);

    QRect searchLineFullHeight(const QRect &regionRect, QRect &lineRect, QRegion &lineRegion);

private:

	int cursorPos;

	bool m_isVisible;

	bool m_isRegionDirty;

    bool m_isEmptyRegion;

    QRect m_usedArea;

	QList<apTextParagraph> paragraphs;

    QRect rect;

	QRegion region;
};

#endif // LSTEXTEDIT_H
