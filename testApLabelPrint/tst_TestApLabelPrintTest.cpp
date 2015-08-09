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

#include <QString>
#include <QtTest>

#include "apUnQLite.h"

class TestApLabelPrintTest : public QObject
{
	Q_OBJECT

public:
	TestApLabelPrintTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();
	void testCase1_data();
	void testCase1();
	void testKeys();
    void testGroups();

private:
	apUnQLite* unqlite;
};

TestApLabelPrintTest::TestApLabelPrintTest() : unqlite(NULL)
{
}

void TestApLabelPrintTest::initTestCase()
{
	QString fileName = QDir::homePath() + "/TestApLabelPrintText.unqlite";
	if (QFile(fileName).exists())
	{
		QVERIFY2(QFile(fileName).remove(), "RemoveFailure");
	}
	this->unqlite = new apUnQLite(fileName);
	QVERIFY2(NULL != this->unqlite, "Failure");
	QVERIFY2(this->unqlite->isValid(), "Not Valid");
	QVERIFY2(this->unqlite->clear(), "CleaFailure");
}

void TestApLabelPrintTest::cleanupTestCase()
{
	delete this->unqlite;
}

void TestApLabelPrintTest::testCase1_data()
{
	QTest::addColumn<QString>("data");
	QTest::newRow("0") << QString();
}

void TestApLabelPrintTest::testCase1()
{
	QFETCH(QString, data);
	QVERIFY2(true, "Failure");
}

void TestApLabelPrintTest::testKeys()
{
	QVERIFY2(this->unqlite->absoluteGroup() == "/", "GroupFailure");
	QVERIFY2(this->unqlite->groupName() == "", "GroupNameFailure");
	QVERIFY2(this->unqlite->setText("test", QStringLiteral("test1")), "SetText1Failure");
	QVERIFY2(this->unqlite->text("test1", "test") == "test", "TextDefaultFailure");
	QVERIFY2(this->unqlite->value("test2", "test2") == "test2", "valueDefaultFailure");
	QVERIFY2(this->unqlite->setText("test1", "newTest1"), "SetText2Failure");
	QVERIFY2(this->unqlite->setValue("test2", "newTest2"), "SetValueFailure");
	QVERIFY2(this->unqlite->text("test1", "test") == "newTest1", "TextFailure");
	QVERIFY2(this->unqlite->value("test2", "test2") == "newTest2", "ValueFailure");
    QVERIFY2(this->unqlite->flush(), "FlushFailure");
}

void TestApLabelPrintTest::testGroups()
{
    QVERIFY2(this->unqlite->setText("testGroup1/test1", "g1"), "CreateGroupKeyFailed");
    QVERIFY2(this->unqlite->groups().join(';') == "testGroup1/", QString("FirstGroupFailed: " + this->unqlite->groups().join(';')).toLatin1().constData());
    apUnQLite group(*this->unqlite, "testGroup2");
    QVERIFY2(group.setValue("g2_1", "Gruppe2_1 Täst"), "g2_1Failed");
    QVERIFY2(group.keys().join(';') == "g2_1", "keys1Failed");
    QVERIFY2(this->unqlite->groups().join(';') == "testGroup1/;testGroup2/", "Groups2Failed");
    QVERIFY2(this->unqlite->removeKey("/testGroup1/test1"), "RemoveKeyFailed");
    QVERIFY2(this->unqlite->groups().join(';') == "testGroup2/", "Groups3Failed");
    QVERIFY2(group.setText("subGroupä1/key1", "sgä1"), "SubGroupFailed");
    QVERIFY2(this->unqlite->groups().join(';') == "testGroup2/", "GroupsBaseFailed");
    QVERIFY2(group.keys().join(";") == "g2_1", "SubGroups1Failed");
    QVERIFY2(group.groups().join(",") == "subGroupä1/", "SubGroupGroupsFailed");
    QVERIFY2(group.setText("subGroup2/key1", "sk2"), "SubGroup2Failed");
    QVERIFY2(group.text("/testGroup2/subGroup2/key1", "x") == "sk2", "SubGroupKeyReadFailed");
    apUnQLite subGroup2(group, "subGroup2");
    QVERIFY2(subGroup2.keys().join(';') == "key1", "SubSubGroupKey1Failed");
    QVERIFY2(subGroup2.clear(), "ClearFailed");

    QVERIFY2(this->unqlite->groups().join(';') == "testGroup2/", "GroupsBaseFailed");
    QVERIFY2(group.keys().join(";") == "g2_1", "SubGroups1Failed");
    QVERIFY2(group.groups().join(",") == "subGroupä1/", "SubGroupGroupsFailed");
    QVERIFY2(group.removeKey("g2_1"), "RemoveSubKey1Failed");
    QVERIFY2(group.removeKey("subGroupä1/key1"), "RemoveSubSubKey1Failed");
    QVERIFY2(this->unqlite->groups().isEmpty(), "LastRemoveFailed");
}

QTEST_APPLESS_MAIN(TestApLabelPrintTest)

#include "tst_TestApLabelPrintTest.moc"
