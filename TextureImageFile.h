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
#ifndef TEXTUREIMAGEFILE_H
#define TEXTUREIMAGEFILE_H

#include "TextureFile.h"

class TextureImageFile : public TextureFile
{
public:
	TextureImageFile(const char *format);
	TextureImageFile(const TextureFile &textureFile);
	bool open(const QByteArray &data);
	bool save(QByteArray &data) const;
	inline quint8 depth() const {
		return image().depth();
	}
private:
	const char *_format;
};

#endif // TEXTUREIMAGEFILE_H
