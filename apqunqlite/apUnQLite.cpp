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

#include "apUnQLite.h"

#include "qunqlite.h"
#include "qunqlitecursor.h"

#include <QFile>
#include <QStringList>

class apUnQLiteData
{
public:
	apUnQLiteData( const QString& databaseFileName )
		: isValid(false)
	{
		isValid = unQLite.open(databaseFileName, QFile(databaseFileName).exists() ? QUnQLite::ReadWrite : QUnQLite::Create);

		QUnQLiteCursor* cursor = unQLite.cursor();

		if (isValid) {
			isValid = cursor->first();

			if ( ! isValid) {
				isValid = unQLite.store(QStringLiteral("test"), QStringLiteral("test"));

				if (isValid) {
					isValid = unQLite.commit();

					if (isValid) {
						isValid = unQLite.remove(QStringLiteral("test"));
					}
				}
			}
		}

		if (isValid)
		{
			if (cursor->first())
			{
				while (cursor->isValid())
				{
					storeKey(cursor->key());
					cursor->next();
				}
			}
			delete cursor;
		}
	}

	~apUnQLiteData() { unQLite.close(); }

	bool clear( const QString& group )
	{
		bool isCleared = true;

		const QStringList allGroups = this->groups.keys();

		const QString groupName = group.section('/', -2, -2);

		if ( ! groupName.isEmpty())
		{
			const QString parentGroup = group.left(group.length() - groupName.length());
			if (groups.contains(parentGroup))
			{
				groups[parentGroup].removeAll(groupName);
			}
		}

		foreach (const QString& groupName, allGroups)
		{
			if (groupName.startsWith(group))
			{
				const QStringList groupKeys = this->keys.value(groupName);
				foreach (const QString& key, groupKeys)
				{
					isCleared = unQLite.remove(groupName + key) & isCleared;
				}

				unQLite.remove(groupName);

				this->keys.remove(groupName);
				this->groups.remove(groupName);
			}
		}

		const QStringList groupKeys = this->keys.value(group);
		foreach (const QString& key, groupKeys)
		{
			isCleared = unQLite.remove(group + key) & isCleared;
		}

		return isCleared;
	}

	void createGroupPath(QString group)
	{
		while ( ! group.isEmpty())
		{
			QString subGroup = group.section('/', -1);
			group.chop(subGroup.length() + 1);

			if (groups.contains(group))
			{
				if ( ! groups[group].contains(subGroup))
				{
					groups[group].append(subGroup);
				}
			}
			else
			{
				groups[group].append(subGroup);
			}
		}
	}

	const QStringList& getGroups( const QString& group ) const {
		if (this->groups.contains(group)) {
			return this->groups[group];
		}
		else {
			return empty;
		}
	}

	const QStringList& getKeys( const QString& group ) const {
		if (this->keys.contains(group)) {
			return this->keys[group];
		}
		else {
			return empty;
		}
	}

	void storeKey( const QString& key )
	{
		QString endKey = key.section('/', -1);
		QString group = key.left(key.length() - endKey.length());

		if ( ! group.isEmpty())
		{
			createGroupPath(group);
		}

		if (keys.contains(group))
		{
			if ( ! keys[group].contains(endKey) )
			{
				keys[group].append(endKey);
			}
		}
		else
		{
			keys[group].append(endKey);
		}
	}

	QUnQLite unQLite;

	bool isValid;

private:
	QHash<QString,QStringList> groups;
	QHash<QString,QStringList> keys;

	QStringList empty;
};

#define KEY (key.startsWith('/') ? key.mid(1) : this->group + key)

apUnQLite::apUnQLite(const QString& databaseFileName, QObject *parent)
	: QObject(parent)
	, data(new apUnQLiteData(databaseFileName))
	, group("")
	, m_groupName("")
{
}

apUnQLite::apUnQLite(const apUnQLite &rhs, const QString& groupName)
	: data(rhs.data)
	, group(groupName.startsWith('/')
			? (groupName.endsWith('/') ? groupName.mid(1) : groupName.mid(1) + "/")
			: rhs.group + (groupName.endsWith('/') ? groupName : groupName + "/"))
	, m_groupName(groupName.endsWith('/') ? groupName.section('/', -2, -2) : groupName.section('/', -1))
{
}

apUnQLite::~apUnQLite()
{
}

apUnQLite &apUnQLite::operator=(const apUnQLite &rhs)
{
	if (this != &rhs) {
		data.operator=(rhs.data);
		this->group = rhs.group;
		this->m_groupName = rhs.m_groupName;
	}
	return *this;
}

bool apUnQLite::clear()
{
	return this->data->clear(this->group);
}

bool apUnQLite::flush()
{
	return this->data->unQLite.commit();
}

const QStringList&apUnQLite::groups() const
{
	return this->data->getGroups(this->group);
}

bool apUnQLite::isValid() const
{
	return this->data->isValid;
}

const QStringList&apUnQLite::keys() const
{
	return this->data->getKeys(this->group);
}

bool apUnQLite::setValue(const QString& key, const QByteArray& value)
{
	const QString realKey = KEY;
	this->data->storeKey(realKey);
	return this->data->unQLite.store(realKey, value);
}

bool apUnQLite::setText(const QString& key, const QString& value)
{
	const QString realKey = KEY;
	this->data->storeKey(realKey);
	return this->data->unQLite.store(realKey, value);
}

QString apUnQLite::text(const QString& key, const QString& defaultValue)
{
	QString result = this->data->unQLite.fetchText(KEY);

	if (result.isEmpty() && this->data->unQLite.lastErrorCode() != QUnQLite::Ok)
	{
		return defaultValue;
	}

	return result;
}

QByteArray apUnQLite::value(const QString& key, const QByteArray& defaultValue)
{
	QByteArray result = this->data->unQLite.fetch(KEY);

	if (result.isEmpty() && this->data->unQLite.lastErrorCode() != QUnQLite::Ok)
	{
		return defaultValue;
	}

	return result;
}

