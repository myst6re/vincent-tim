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
#ifndef EXTRADATA_H
#define EXTRADATA_H

#include <QMap>
#include <QVariant>
#include <QIODevice>

class ExtraData
{
public:
	explicit ExtraData(const QMap<QString, QVariant> &fields = QMap<QString, QVariant>());
	bool open(const QString &filename);
	bool open(QIODevice *device);
	bool save(const QString &filename) const;
	bool save(QIODevice *device) const;
	inline const QMap<QString, QVariant> &fields() const {
		return _fields;
	}
private:
	QMap<QString, QVariant> _fields;
};

#endif // EXTRADATA_H
