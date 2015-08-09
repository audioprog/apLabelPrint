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

#ifndef APUNQLITE_H
#define APUNQLITE_H

#include <QObject>
#include <QSharedPointer>

class apUnQLiteData;

class apUnQLite : public QObject
{
	Q_OBJECT
public:
	explicit apUnQLite( const QString& databaseFileName, QObject *parent = 0 );
	apUnQLite( const apUnQLite &, const QString& groupName );
	apUnQLite &operator=( const apUnQLite & );
	~apUnQLite();

	QString absoluteGroup() const { return this->group.isEmpty() ? "/" : "/" + this->group.left(this->group.length() - 1); }

	bool clear();

	bool flush();

	const QString& groupName() const { return this->m_groupName; }

    QStringList groups() const;

	bool isValid() const;

    QStringList keys() const;

    bool removeGroup( const QString& groupName );

    bool removeKey( const QString& key);

	QString text( const QString& key, const QString& defaultValue = QString() );

	bool setValue( const QString& key, const QByteArray& value );

	bool setText( const QString& key, const QString& value );

	QByteArray value( const QString& key, const QByteArray& defaultValue = QByteArray() );

signals:

public slots:

private:
	QSharedPointer<apUnQLiteData> data;

	QString m_groupName;

	QString group;
};

#endif // APUNQLITE_H
