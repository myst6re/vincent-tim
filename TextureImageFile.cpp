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
#include "TextureImageFile.h"
#include <QBuffer>

TextureImageFile::TextureImageFile(const char *format) :
    _format(format)
{
}

bool TextureImageFile::open(const QByteArray &data)
{
	bool ret = _image.loadFromData(data, _format);
	_colorTables.clear();
	if (_image.format() == QImage::Format_Mono || _image.format() == QImage::Format_Mono || _image.format() == QImage::Format_Mono) {
		_colorTables.append(_image.colorTable());
	}
	return ret;
}

bool TextureImageFile::save(QByteArray &data) const
{
	QBuffer buff;

	bool ret = _image.save(&buff, _format);

	data = buff.data();

	return ret;
}
