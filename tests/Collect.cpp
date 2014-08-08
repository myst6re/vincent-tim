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
#include "Collect.h"
#include "../TextureFile.h"

Collect::Collect(const QString &dir) :
    _dir(dir)
{
}

void Collect::textureData(const QString &format) const
{
	QStringList texFilenames = _dir.entryList(QStringList("*." + format), QDir::Files);
	QMap<QString, QMap<quint32, int> > occurrences;

	foreach (const QString &texFilename, texFilenames) {
		TextureFile *tex = TextureFile::factory(format);
		if (tex->openFromFile(_dir.filePath(texFilename))) {
			QMap<QString, QVariant> fields = tex->extraData().fields();
			QMapIterator<QString, QVariant> it(fields);
			while (it.hasNext()) {
				it.next();
				bool ok;
				quint32 val = it.value().toUInt(&ok);
				if (!ok) {
					qDebug() << "Collect::texData not an int" << it.key() << it.value();
				} else {
					QMap<quint32, int> counts = occurrences.value(it.key());
					int count = counts.value(val, 0);
					counts.insert(val, count + 1);
					occurrences.insert(it.key(), counts);
				}
			}
		} else {
			qDebug() << texFilename << "cannot open file";
		}
		delete tex;
	}

	QMapIterator<QString, QMap<quint32, int> > it(occurrences);
	while (it.hasNext()) {
		it.next();
		const QString &name = it.key();
		const QMap<quint32, int> &values = it.value();

		qDebug() << name;

		QMapIterator<quint32, int> itValues(values);
		while (itValues.hasNext()) {
			itValues.next();
			qDebug() << "\t" << itValues.key() << "-> x" << itValues.value();
		}
	}
}
