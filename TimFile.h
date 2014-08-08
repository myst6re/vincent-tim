/****************************************************************************
 ** Deling Final Fantasy VIII Field Editor
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
#ifndef TIMFILE_H
#define TIMFILE_H

#include <QtCore>
#include "TextureFile.h"

typedef QPair<int, int> PosSize;

class TimFile : public TextureFile
{
public:
	TimFile();
	TimFile(const TextureFile &texture,
	        quint16 palX=0, quint16 palY=0,
	        quint16 imgX=0, quint16 imgY=0);
	bool open(const QByteArray &data);
	bool save(QByteArray &data) const;
	inline quint8 depth() const {
		if (bpp == 0) {
			return 4;
		}
		return bpp * 8;
	}
	void setDepth(quint8 depth);
	inline quint16 paletteX() const {
		return palX;
	}
	inline quint16 paletteY() const {
		return palY;
	}
	inline quint16 paletteW() const {
		return palW;
	}
	inline quint16 paletteH() const {
		return palH;
	}
	inline quint16 imageX() const {
		return imgX;
	}
	inline quint16 imageY() const {
		return imgY;
	}

	ExtraData extraData() const;
	bool setExtraData(const ExtraData &extraData);

	QSize paletteSize() const;

	static TimFile fromTexture(TextureFile *texture, const ExtraData &meta, const QImage &palette = QImage());
	static QList<PosSize> findTims(const QByteArray &data, int limit = 0);
private:
	void setPaletteSize(const QSize &size);
	QList< QVector<QRgb> > exportColorTables() const;
	void importColorTables(const QList< QVector<QRgb> > &colorTables);

	quint8 bpp;
	quint16 palX, palY;
	quint16 palW, palH;
	quint16 imgX, imgY;
	QList<QBitArray> _alphaBits;
};

#endif // TIMFILE_H
