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
#include "TimFile.h"
#include "PsColor.h"

TimFile::TimFile() :
	TextureFile(), bpp(1), palX(0), palY(0), palW(0), palH(0), imgX(0), imgY(0)
{
}

TimFile::TimFile(const TextureFile &texture, quint16 palX, quint16 palY, quint16 imgX, quint16 imgY) :
	TextureFile(texture), palX(palX), palY(palY), imgX(imgX), imgY(imgY)
{
	setDepth(texture.depth());
	setPaletteSize(texture.paletteSize());
}

bool TimFile::open(const QByteArray &data)
{
	//	QTime t;t.start();

	quint32 palSize=0, imgSize=0, color=0;
	quint16 w, h;
	const char *constData = data.constData();
	bool hasPal;
	int dataSize = data.size();
	
	if(!data.startsWith(QByteArray("\x10\x00\x00\x00", 4)) || dataSize < 8) {
		qWarning() << "Invalid TIM header";
		return false;
	}

//	quint8 tag = (quint8)data.at(0);
#ifdef TIMFILE_EXTRACT_UNUSED_DATA
	_version = (quint8)data.at(1);
	memcpy(&_headerUnused1, constData + 2, 2);
	memcpy(&_headerUnused2, constData + 4, 4);
	_headerUnused2 &= 0xFFF8;
#endif
	bpp = (quint8)data.at(4) & 3;
	hasPal = ((quint8)data.at(4) >> 3) & 1;

//	qDebug() << QString("=== Apercu TIM ===");
//	qDebug() << QString("version = %1, reste = %2").arg(version).arg(QString(data.mid(2,2).toHex()));
//	qDebug() << QString("bpp = %1, hasPal = %2, flag = %3, reste = %4").arg(bpp).arg(hasPal).arg((quint8)data.at(4),0,2).arg(QString(data.mid(5,3).toHex()));
	
	if(hasPal && bpp > 1) {
		qWarning() << "Bits Per Pixel is 16 and there are palettes";
		return false;
	}

	_colorTables.clear();
	_alphaBits.clear();

	if(hasPal)
	{
		if(dataSize < 20) {
			qWarning() << "File too short to have palettes";
			return false;
		}

		memcpy(&palSize, constData + 8, 4);
		memcpy(&palX, constData + 12, 2);
		memcpy(&palY, constData + 14, 2);
		memcpy(&palW, constData + 16, 2);
		memcpy(&palH, constData + 18, 2);

//		qDebug() << QString("-Palette-");
//		qDebug() << QString("Size = %1, w = %2, h = %3").arg(palSize).arg(palW).arg(palH);
//		qDebug() << QString("x = %1, y = %2").arg(palX).arg(palY);
		
		if((quint32)dataSize < 8+palSize/* || palSize != (quint32)palW*palH*2+12*/) {
			qWarning() << "File too short for the size of palettes section" << palSize;
			return false;
		}

		quint16 onePalSize = (bpp==0 ? 16 : 256);
		int nbPal = (palSize-12)/(onePalSize*2);

		if((palSize-12)%(onePalSize*2) != 0 && palSize == quint32(12 + onePalSize * nbPal * 4)) {
			nbPal *= 2;
		}

		if(nbPal > 0) {
			int pos=0;
			for(int i=0 ; i<nbPal ; ++i) {
				QVector<QRgb> pal;
				QBitArray alphaBits(onePalSize);

				for(quint16 j=0 ; j<onePalSize ; ++j) {
					memcpy(&color, constData + 20 + pos*2 + j*2, 2);
					pal.append(PsColor::fromPsColor(color, true));
					alphaBits.setBit(j, psColorAlphaBit(color));
				}

				_colorTables.append(pal);
				_alphaBits.append(alphaBits);

				pos += pos % palW == 0 ? onePalSize : palW - onePalSize;
			}
		} else {
			qWarning() << "TimFile::open nbPal <= 0" << nbPal;
			return false;
		}

		_currentColorTable = 0;
//		qDebug() << QString("NbPal = %1 (valid : %2)").arg(nbPal).arg((palSize-12)%(onePalSize*2));
	}
	
	if((quint32)dataSize < 20+palSize) {
		qWarning() << "File too short";
		return false;
	}

	memcpy(&imgSize, constData + 8 + palSize, 4);
	memcpy(&imgX, constData + 12 + palSize, 2);
	memcpy(&imgY, constData + 14 + palSize, 2);
	memcpy(&w, constData + 16 + palSize, 2);
	memcpy(&h, constData + 18 + palSize, 2);
	if(bpp==0)		w*=4;
	else if(bpp==1)	w*=2;

//	qDebug() << QString("-Image-");
//	qDebug() << QString("Size = %1, w = %2, h = %3").arg(imgSize).arg(w).arg(h);
//	qDebug() << QString("TIM Size = %1").arg(8+palSize+imgSize);

	_image = QImage(w, h, hasPal ? QImage::Format_Indexed8 : QImage::Format_ARGB32);
	if(hasPal) {
		_image.setColorTable(_colorTables.first());
	}
	//_image.fill(QColor(0, 0, 0, 0));
	QRgb *pixels = (QRgb *)_image.bits();

	int size, i=0;
	quint32 x=0, y=0;

	if(bpp!=0) {
		size = qMin((quint32)(12 + w*h*bpp), dataSize - 8 - palSize);
	} else {
		size = qMin((quint32)(12 + w/2*h), dataSize - 8 - palSize);
	}

	if(8 + palSize + size > (quint32)dataSize) {
		qWarning() << "TimFile::open 8 + palSize + size > dataSize" << palSize << size << dataSize;
		return false;
	}

	if(bpp==0)//mag176, icon
	{
		while(i<size && y<h)
		{
			_image.setPixel(x, y, (quint8)data.at(20+palSize+i) & 0xF);
			++x;
			if(x==w)
			{
				x = 0;
				++y;
			}

			_image.setPixel(x, y, (quint8)data.at(20+palSize+i) >> 4);
			++x;
			if(x==w)
			{
				x = 0;
				++y;
			}
			++i;
		}
	}
	else if(bpp==1)
	{
		while(i<size && y<h)
		{
			_image.setPixel(x, y, (quint8)data.at(20+palSize+i));

			++x;
			if(x==w)
			{
				x = 0;
				++y;
			}
			++i;
		}
	}
	else if(bpp==2)
	{
		QBitArray alphaBits(w * h);

		while(i<size && y<h)
		{
			memcpy(&color, constData + 20 + palSize + i, 2);
			pixels[x + y*w] = PsColor::fromPsColor(color, true);
			alphaBits.setBit(i / 2, psColorAlphaBit(color));

			++x;
			if(x==w)
			{
				x = 0;
				++y;
			}
			i+=2;
		}

		_alphaBits.append(alphaBits);
	}
	else if(bpp==3)
	{
		while(i<size && y<h)
		{
			memcpy(&color, constData + 20 + palSize + i, 3);
			pixels[x + y*w] = qRgb(color >> 16, (color >> 8) & 0xFF, color & 0xFF);

			++x;
			if(x==w)
			{
				x = 0;
				++y;
			}
			i+=3;
		}
	}

//	qDebug() << t.elapsed();
	return true;
}

bool TimFile::save(QByteArray &data) const
{
	Q_ASSERT(_colorTables.size() == _alphaBits.size());

	bool hasPal = isPaletted();
	quint32 flag = (hasPal << 3) | (bpp & 3);

	// Header
	data.append("\x10\x00\x00\x00", 4);
	data.append((char *)&flag, 4);

	if(hasPal) {
		quint16 colorPerPal = this->colorPerPal();
		quint32 sizePalSection = 12 + _colorTables.size() * colorPerPal * 2;

		data.append((char *)&sizePalSection, 4);
		data.append((char *)&palX, 2);
		data.append((char *)&palY, 2);
		data.append((char *)&palW, 2);
		data.append((char *)&palH, 2);

		int colorTableId = 0;
		foreach(const QVector<QRgb> &colorTable, _colorTables) {
			const QBitArray &alphaBit = _alphaBits.at(colorTableId);
			int i;

			Q_ASSERT(colorTable.size() == colorPerPal);
			Q_ASSERT(alphaBit.size() == colorPerPal);

			for(i=0 ; i<colorPerPal ; ++i) {
				quint16 psColor = PsColor::toPsColor(colorTable.at(i));
				psColor = setPsColorAlphaBit(psColor, alphaBit.at(i));
				data.append((char *)&psColor, 2);
			}

			++colorTableId;
		}

		quint16 width = _image.width(), height = _image.height();
		quint32 sizeImgSection = 12;
		if(bpp==0) {
			width/=4;
			sizeImgSection += _image.width()/2 * height;
		}
		else {
			width/=2;
			sizeImgSection += _image.width() * height;
		}

		data.append((char *)&sizeImgSection, 4);
		data.append((char *)&imgX, 2);
		data.append((char *)&imgY, 2);
		data.append((char *)&width, 2);
		data.append((char *)&height, 2);

		width *= 2;

		for(int y=0 ; y<height ; ++y) {
			for(int x=0 ; x<width ; ++x) {
				if(bpp == 0) {
					quint8 index = (_image.pixelIndex(x*2, y) & 0xF) | ((_image.pixelIndex(x*2+1, y) & 0xF) << 4);
					data.append((char)index);
				} else {
					data.append((char)_image.pixelIndex(x, y));
				}
			}
		}
	} else {
		quint16 width = _image.width(), height = _image.height();
		quint32 sizeImgSection = 12 + width * bpp * height;
		const QBitArray &alphaBit = _alphaBits.first();

		data.append((char *)&sizeImgSection, 4);
		data.append((char *)&imgX, 2);
		data.append((char *)&imgY, 2);
		data.append((char *)&width, 2);
		data.append((char *)&height, 2);

		for(int y=0 ; y<height ; ++y) {
			for(int x=0 ; x<width ; ++x) {
				if(bpp == 2) {
					quint16 color = PsColor::toPsColor(_image.pixel(x, y));
					setPsColorAlphaBit(color, alphaBit.at(y * width + x));
					data.append((char *)&color, 2);
				} else {
					QRgb c = _image.pixel(x, y);
					quint32 color = ((qRed(c) & 0xFF) << 16) | ((qGreen(c) & 0xFF) << 8) | (qBlue(c) & 0xFF);
					data.append((char *)&color, 3);
				}
			}
		}
	}

	return true;
}

void TimFile::setDepth(quint8 depth)
{
	if (depth < 8) {
		bpp = 0;
	} else {
		bpp = depth / 8;
	}
}

ExtraData TimFile::extraData() const
{
	QMap<QString, QVariant> ret;
	ret["depth"] = depth();
	ret["paletteX"] = paletteX();
	ret["paletteY"] = paletteY();
	ret["imageX"] = imageX();
	ret["imageY"] = imageY();

#ifdef TIMFILE_EXTRACT_UNUSED_DATA
	ret["version"] = _version;
	ret["headerUnused1"] = _headerUnused1;
	ret["headerUnused2"] = _headerUnused2;
#endif

	return ExtraData(ret);
}

bool TimFile::setExtraData(const ExtraData &extraData)
{
	bool ok;
	QMap<QString, QVariant> fields = extraData.fields();
	quint8 depth = 255;

	setExtraDataFieldUsed(fields);

	setExtraDataField(depth, "depth");
	if (depth != 255) {
		setDepth(depth);
	}
	setExtraDataField(palX, "paletteX");
	setExtraDataField(palY, "paletteY");
	setExtraDataField(imgX, "imageX");
	setExtraDataField(imgY, "imageY");
#ifdef TIMFILE_EXTRACT_UNUSED_DATA
	setExtraDataField(_version, "version");
	setExtraDataField(_headerUnused1, "headerUnused1");
	setExtraDataField(_headerUnused2, "headerUnused2");
#endif

	return ok;
}

QSize TimFile::paletteSize() const
{
	return QSize(palW, palH);
}

void TimFile::setPaletteSize(const QSize &size)
{
	palW = size.width();
	palH = size.height();
}

QList< QVector<QRgb> > TimFile::exportColorTables() const
{
	QList< QVector<QRgb> > ret;
	QListIterator<QBitArray> alphaIt(_alphaBits);

	Q_ASSERT(_alphaBits.size() == _colorTables.size());

	foreach (QVector<QRgb> colorTable, _colorTables) {
		const QBitArray &alphaBits = alphaIt.next();

		Q_ASSERT(alphaBits.size() == colorTable.size());

		for (int i=0; i<colorTable.size(); ++i) {
			const QRgb &color = colorTable[i];
			int alpha = qAlpha(color);
			if (alpha == 255) { // Opaque
				if (alphaBits.at(i)) {
					alpha = 127; // Semi-transparent
				}
			}
			colorTable[i] = qRgba(qRed(color), qGreen(color), qBlue(color), alpha);
		}

		ret << colorTable;
	}

	return ret;
}

void TimFile::importColorTables(const QList< QVector<QRgb> > &colorTables)
{
	_colorTables.clear();

	foreach (QVector<QRgb> colorTable, colorTables) {
		QBitArray alphaBits(colorTable.size(), false);

		for (int i=0; i<colorTable.size(); ++i) {
			QRgb color = colorTable[i];
			int alpha = qAlpha(color);

			if (alpha == 0) { // Fully-transparent
				color = qRgba(0, 0, 0, 0);
			} else if (alpha == 127) { // Semi-transparent
				color = qRgba(qRed(color), qGreen(color), qBlue(color), 255);
				alphaBits.setBit(i);
			} else {
				color = qRgba(qRed(color), qGreen(color), qBlue(color), 255);
			}

			colorTable[i] = color;
		}

		_colorTables << colorTable;
		_alphaBits << alphaBits;
	}
}

TimFile TimFile::fromTexture(TextureFile *texture, const ExtraData &meta, const QImage &palette)
{
	TimFile tim(*texture);
	tim.setExtraData(meta);
	if (!palette.isNull()) {
		tim.setPalette(palette);
	}
	return tim;
} 

bool TimFile::nextTim(QIODevice *device, qint64 limit)
{
	QByteArray data, find = QByteArray("\x10\x00\x00\x00", 4);
	qint64 offset, bufferSize;
	int index;

	forever {
		offset = device->pos();
		if (limit > 0) {
			bufferSize = qMin(TIMFILE_EXPLORE_BUFFER_SIZE, limit);
		} else {
			bufferSize = TIMFILE_EXPLORE_BUFFER_SIZE;
		}
		data = device->read(bufferSize);

		if (data.size() < find.size()) {
			return false;
		}

		index = data.indexOf(find);

		if (index != -1) {
			device->seek(offset + index);
			return true;
		} else {
			// back to pos - (pattern size - 1)
			device->seek(device->pos() - (find.size() - 1));
			if (limit > 0) {
				limit -= device->pos() - offset; // bytes read
				if (limit <= 0) {
					return false;
				}
			}
		}
	}
}

QList<PosSize> TimFile::findTims(QIODevice *device, int limit)
{
	qint64 index;
	quint32 palSize, imgSize;
	quint16 w, h;
	quint8 bpp;
	QList<PosSize> positions;

	while (nextTim(device, limit)) {
		index = device->pos();

		if (!device->seek(device->pos() + 4)) {
			break;
		}

		if (device->read((char *)&bpp, 1) != 1) {
			break;
		}

		if (!device->seek(device->pos() + 3)) {
			break;
		}

		if (bpp == 8 || bpp == 9) {
			if (device->read((char *)&palSize, 4) != 4) {
				break;
			}

			// X and Y
			if (!device->seek(device->pos() + 4)) {
				break;
			}

			if (device->read((char *)&w, 2) != 2) {
				break;
			}

			if (device->read((char *)&h, 2) != 2) {
				break;
			}

			if (palSize != quint32(w * h * 2 + 12)) {
				device->seek(index + 1);
				continue;
			}

			device->seek(device->pos() + palSize - 12);
		} else if (bpp != 2 && bpp != 3) {
			device->seek(index + 1);
			continue;
		}

		if (device->read((char *)&imgSize, 4) != 4) {
			break;
		}

		// X and Y
		if (!device->seek(device->pos() + 4)) {
			break;
		}

		if (device->read((char *)&w, 2) != 2) {
			break;
		}

		if (device->read((char *)&h, 2) != 2) {
			break;
		}

		if (imgSize != quint32(w * 2 * h + 12)) {
			device->seek(index + 1);
			continue;
		}

		positions.append(PosSize(index, 8 + palSize + imgSize));
	}

	return positions;
}
