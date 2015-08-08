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

QTEST_APPLESS_MAIN(TestApLabelPrintTest)

#include "tst_TestApLabelPrintTest.moc"
