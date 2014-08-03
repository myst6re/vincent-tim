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
#include "TextureFile.h"
#include "TextureImageFile.h"
#include "TexFile.h"
#include "TimFile.h"

TextureFile *TextureFile::factory(const QString &format)
{
	if (format.compare("tex", Qt::CaseInsensitive) == 0) {
		return new TexFile();
	} else if (format.compare("tim", Qt::CaseInsensitive) == 0) {
		return new TimFile();
	}
	return new TextureImageFile(format.toUpper().toLocal8Bit().constData());
}

QStringList TextureFile::supportedTextureFormats()
{
	return QStringList() << "tim" << "tex";
}

TextureFile::TextureFile() :
	_currentColorTable(0)
{
}

TextureFile::TextureFile(const QImage &image) :
	_image(image), _currentColorTable(0)
{
	QVector<QRgb> colorTable = _image.colorTable();
	if(!colorTable.empty()) {
		_colorTables.append(colorTable);
	}
}

TextureFile::TextureFile(const QImage &image, const QList< QVector<QRgb> > &colorTables) :
	_image(image), _colorTables(colorTables), _currentColorTable(0)
{
}

bool TextureFile::openFromFile(const QString &filename)
{
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly)) {
		return false;
	}
	return open(f.readAll());
}

bool TextureFile::saveToFile(const QString &filename) const
{
	QFile f(filename);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}
	QByteArray data;
	if (!save(data)) {
		return false;
	}
	f.write(data);
	return true;
}

ExtraData TextureFile::extraData() const
{
	return ExtraData();
}

bool TextureFile::setExtraData(const ExtraData &extraData)
{
	Q_UNUSED(extraData);
	return true;
}

bool TextureFile::isValid() const
{
	return !_image.isNull();
}

void TextureFile::clear()
{
	_image = QImage();
	_colorTables.clear();
	_currentColorTable = 0;
}

const QImage &TextureFile::image() const
{
	return _image;
}

QImage *TextureFile::imagePtr()
{
	return &_image;
}

bool TextureFile::isPaletted() const
{
	return !_colorTables.isEmpty();
}

const QList< QVector<QRgb> > &TextureFile::colorTables() const
{
	return _colorTables;
}

int TextureFile::currentColorTable() const
{
	return _currentColorTable;
}

QVector<QRgb> TextureFile::colorTable(int id) const
{
	return _colorTables.value(id);
}

void TextureFile::setCurrentColorTable(int id)
{
	if(id < _colorTables.size() && _currentColorTable != id) {
		_image.setColorTable(_colorTables.at(_currentColorTable = id));
	}
}

void TextureFile::setColorTable(int id, const QVector<QRgb> &colorTable)
{
	if(id < _colorTables.size()) {
		int nbColors = _colorTables.at(id).size();
		QVector<QRgb> ct = colorTable;
		if (nbColors != colorTable.size()) {
			ct.resize(nbColors);
		}
		_colorTables.replace(id, ct);
	}
}

int TextureFile::colorTableCount() const
{
	return _colorTables.size();
}

QImage TextureFile::palette() const
{
	if (depth() >= 16) {
		return QImage();
	}

	QImage image(paletteSize(), QImage::Format_ARGB32);
	int x, y;
	const int maxWidth = image.width() - 1,
	        maxHeight = image.height() - 1;

	image.fill(Qt::black);

	y = 0;
	foreach (const QVector<QRgb> &colorTable, _colorTables) {
		x = 0;
		foreach (const QRgb &color, colorTable) {
			if (y > maxHeight) {
				qWarning() << "More color tables than palette height";
				return image;
			}
			image.setPixel(x, y, color);

			if (x == maxWidth) {
				x = 0;
				++y;
			} else {
				++x;
			}
		}
	}

	return image;
}

bool TextureFile::setPalette(const QImage &image)
{
	QVector<QRgb> colorTable;
	quint16 colorPerPal = colorPerPalFromDepth();

	if (colorPerPal == 0) {
		return false;
	}

	_colorTables.clear();

	for (int y = 0; y < image.height(); ++y) {
		for (int x = 0; x < image.width(); ++x) {
			colorTable.append(image.pixel(x, y));
			if (colorTable.size() == colorPerPal) {
				_colorTables.append(colorTable);
				colorTable.clear();
			}
		}
	}

	if (!colorTable.isEmpty()) {
		_colorTables.append(colorTable);
	}

	setPaletteSize(image.size());

	return true;
}

void TextureFile::convertToIndexedFormat(int colorTableId)
{
	QVector<QRgb> colors = colorTable(colorTableId);

	// Fixing error with alpha
	QMutableVectorIterator<QRgb> it(colors);
	while(it.hasNext()) {
		QRgb color = it.next();
		if (qAlpha(color) == 0 && color != qRgba(0, 0, 0, 0)) {
			it.setValue(qRgba(0, 0, 0, 0));
		}
	}

	if (_image.format() == QImage::Format_ARGB32) {
		QRgb *pixels = (QRgb *)_image.bits();
		for (int i=0; i<_image.height() * _image.width(); ++i) {
			QRgb color = *pixels;
			if (qAlpha(color) == 0 && color != qRgba(0, 0, 0, 0)) {
				*pixels = qRgba(0, 0, 0, 0);
			}
			pixels++;
		}
	}

	_image = _image.convertToFormat(QImage::Format_Indexed8, colors);
}

QVector<quint8> TextureFile::alpha() const
{
	return QVector<quint8>();
}

void TextureFile::setAlpha(const QVector<quint8> &alpha)
{
	Q_UNUSED(alpha);
}

QImage TextureFile::alphaImage() const
{
	QVector<quint8> a = alpha();

	if (a.isEmpty()) {
		return QImage();
	}

	QImage image(16, a.size() / 16 + a.size() % 16, QImage::Format_Indexed8);
	int x = 0, y = 0, i = 0;

	image.setColor(0, Qt::black);
	image.setColor(1, Qt::white);
	image.setColor(2, Qt::red);
	image.fill(0);

	for (; y < image.height() && i < a.size(); ++y) {
		for (x = 0; x < image.width() && i < a.size(); ++x) {
			if (a.at(i++)) {
				image.setPixel(x, y, 1);
			}
		}
	}

	if (x < image.width() || y < image.height()) {
		image.setPixel(x, y, 2);
	}

	return image;
}

void TextureFile::setAlphaImage(const QImage &image)
{
	QVector<quint8> a;

	for (int y = 0; y < image.height(); ++y) {
		for (int x = 0; x < image.width(); ++x) {
			QRgb pixel = image.pixel(x, y);
			if (pixel == Qt::red) {
				goto setAlphaImageEnd;
			} else {
				a.append(pixel == Qt::white);
			}
		}
	}

setAlphaImageEnd:
	setAlpha(a);
}

quint16 TextureFile::colorPerPal() const
{
	if (_colorTables.isEmpty()) {
		return 0;
	}

	return _colorTables.first().size();
}

quint16 TextureFile::colorPerPalFromDepth() const
{
	switch(depth()) {
	case 4:
		return 16;
	case 8:
		return 256;
	default:
		return 0;
	}
}

QSize TextureFile::paletteSize() const
{
	return QSize(16, (colorPerPal() / 16) * colorTableCount());
}

void TextureFile::setPaletteSize(const QSize &size)
{
	Q_UNUSED(size);
}

void TextureFile::setDepth(quint8 depth)
{
	if (this->depth() != depth) {
		// TODO
	}
}

void TextureFile::debug() const
{
	QImage pal = palette();
	pal.scaled(pal.width() * 4, pal.height() * 4, Qt::KeepAspectRatio)
	        .save("palettes.png");
}
