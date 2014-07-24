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

