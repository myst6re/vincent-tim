/****************************************************************************
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "ExtraData.h"
#include <QFile>
#include <QRegExp>
#include <QStringList>
#include <QDebug>

#define EXTRA_DATA_MAX_LINE_WIDTH	128

ExtraData::ExtraData(const QMap<QString, QVariant> &fields) :
    _fields(fields)
{
}

bool ExtraData::open(const QString &filename)
{
	QFile f(filename);
	return open(&f);
}

bool ExtraData::open(QIODevice *device)
{
	if (!device->open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}
	QRegExp re("(\\w+)\\s*=\\s*(.+)");

	forever {
		QString line = QString(device->readLine(EXTRA_DATA_MAX_LINE_WIDTH)).trimmed();
		if (line.isEmpty()) {
			break;
		}
		if (line.startsWith('#')) {
			continue;
		}

		if (re.exactMatch(line)) {
			QStringList capturedTexts = re.capturedTexts();
			QString key = capturedTexts.at(1);
			QString value = capturedTexts.at(2);
			_fields.insert(key, QVariant(value));
		} else {
			return false;
		}
	}

	device->close();

	return true;
}

bool ExtraData::save(const QString &filename) const
{
	QFile f(filename);
	return save(&f);
}

bool ExtraData::save(QIODevice *device) const
{
	if (!device->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		return false;
	}

	QMapIterator<QString, QVariant> it(_fields);
	while (it.hasNext()) {
		it.next();
		const QString &key = it.key();
		const QVariant &value = it.value();

		device->write(key.toLatin1());
		device->write("=");
		device->write(value.toString().toLatin1());
		device->write("\n");
	}

	device->close();

	return true;
}

