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
#ifndef TEXTUREFILE_H
#define TEXTUREFILE_H

#include <QImage>
#include "ExtraData.h"

#define setExtraDataFieldUsed(fields)
#define setExtraDataField(var, key) \
	if (fields.contains(key)) { \
		var = fields[key].toUInt(&ok); \
		if (!ok) return false; \
	}

class TextureFile
{
public:
	static TextureFile *factory(const QString &format);
	TextureFile();
	TextureFile(const QImage &image);
	TextureFile(const QImage &image, const QList< QVector<QRgb> > &colorTables);
	virtual ~TextureFile() {}
	bool openFromFile(const QString &filename);
	virtual bool open(const QByteArray &data)=0;
	bool saveToFile(const QString &filename) const;
	virtual bool save(QByteArray &data) const=0;
	virtual ExtraData extraData() const;
	virtual bool setExtraData(const ExtraData &extraData);
	bool isValid() const;
	void clear();
	const QImage &image() const;
	QImage *imagePtr();
	bool isPaletted() const;
	const QList< QVector<QRgb> > &colorTables() const;
	int currentColorTable() const;
	QVector<QRgb> colorTable(int id) const;
	void setCurrentColorTable(int id);
	void setColorTable(int id, const QVector<QRgb> &colorTable);
	int colorTableCount() const;
	virtual quint8 depth() const=0;
	QImage palette() const;
	void setPalette(const QImage &image);
	virtual QSize paletteSize() const;
	void debug() const;
	static QStringList supportedTextureFormats();
protected:
	virtual void setPaletteSize(const QSize &size);
	virtual inline quint16 colorPerPal() const {
		return 256;
	}
	QImage _image;
	QList< QVector<QRgb> > _colorTables;
	int _currentColorTable;
};

#endif // TEXTUREFILE_H
