/*
 * (C) 2016 audioprog@users.noreply.github.com
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

#ifndef APTEXTDOCUMENTLAYOUT_H
#define APTEXTDOCUMENTLAYOUT_H

#include <QAbstractTextDocumentLayout>

class apTextDocumentLayout : public QAbstractTextDocumentLayout
{
public:
    apTextDocumentLayout(QTextDocument* document);

public:
    void draw(QPainter *painter, const PaintContext &context);
    int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;
    int pageCount() const;
    QSizeF documentSize() const;
    QRectF frameBoundingRect(QTextFrame *frame) const;
    QRectF blockBoundingRect(const QTextBlock &block) const;

protected:
    void documentChanged(int from, int charsRemoved, int charsAdded);
};

#endif // APTEXTDOCUMENTLAYOUT_H
