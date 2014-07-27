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
#include "TexFile.h"
#include "PsColor.h"

TexFile::TexFile() :
	TextureFile()
{
	setHeader(One, true);
}

TexFile::TexFile(const TextureFile &textureFile, const TexStruct &header,
		const QVector<quint8> &colorKeyArray) :
	TextureFile(textureFile), _header(header), colorKeyArray(colorKeyArray)
{
}

TexFile::TexFile(const TextureFile &textureFile,
                 Version version, bool hasAlpha, bool fourBitsPerIndex,
                 const QVector<quint8> &colorKeyArray) :
    TextureFile(textureFile), colorKeyArray(colorKeyArray)
{
	setHeader(version, hasAlpha, fourBitsPerIndex);
}

TexFile::TexFile(const TextureFile &texture) :
    TextureFile(texture)
{
	setHeader(One, true, texture.depth() == 4);
}

bool TexFile::open(const QByteArray &data)
{
    const char *constData = data.constData();
	quint32 w, h, headerSize, paletteSectionSize, imageSectionSize, colorKeySectionSize;

	if((quint32)data.size() < sizeof(TexStruct)) {
		qWarning() << "tex size too short!";
		return false;
	}

	memcpy(&_header, constData, sizeof(TexStruct));

	if(_header.version == 1) {
		headerSize = sizeof(TexStruct) - 4;
	} else if(_header.version == 2) {
		headerSize = sizeof(TexStruct);
	} else {
		qWarning() << "unknown tex version!";
		return false;
	}

	w = _header.imageWidth;
	h = _header.imageHeight;
	paletteSectionSize = _header.nbPalettes > 0 ? _header.paletteSize * 4 : 0;
	imageSectionSize = w * h * _header.bytesPerPixel;
	colorKeySectionSize = _header.hasColorKeyArray ? _header.nbPalettes : 0;

	if((quint32)data.size() != headerSize + paletteSectionSize + imageSectionSize + colorKeySectionSize) {
		qWarning() << "tex invalid size!" << data.size() << (headerSize + paletteSectionSize + imageSectionSize + colorKeySectionSize);
		return false;
	}

	quint32 i;

	if(_header.nbPalettes > 0)
	{
		quint32 index, imageStart = headerSize + paletteSectionSize;

		for(quint32 palID=0 ; palID < _header.nbPalettes ; ++palID) {
			quint32 paletteStart = headerSize+_header.nbColorsPerPalette1*4*palID;

			_image = QImage(w, h, QImage::Format_Indexed8);
			QVector<QRgb> colors;

			for(i=0 ; i<_header.nbColorsPerPalette1 ; ++i)
			{
				index = paletteStart + i*4;
				colors.append(qRgba(data.at(index+2), data.at(index+1), data.at(index), data.at(index+3)));
			}

			_colorTables.append(colors);
		}

		_image.setColorTable(_colorTables.first());

		for(i=0 ; i<imageSectionSize ; ++i)
		{
			_image.setPixel(i % w, i / w, (quint8)data.at(imageStart+i));
		}

		if(_header.hasColorKeyArray) {
			quint32 colorKeyStart = imageStart + imageSectionSize;

			colorKeyArray.clear();
			for(quint32 j=0 ; j<_header.nbPalettes ; ++j) {
				colorKeyArray.append(data.at(colorKeyStart+j));
			}
		}
    }
    else
    {
		quint16 color;
		_image = QImage(w, h, QImage::Format_ARGB32);
		QRgb *pixels = (QRgb *)_image.bits();

		for(i=0 ; i<imageSectionSize ; i+=_header.bytesPerPixel) {
			if(_header.bytesPerPixel == 2) {
				memcpy(&color, constData + headerSize + i, 2);
				pixels[i/2] = PsColor::fromPsColor(color);
			} else if(_header.bytesPerPixel == 3) {
				pixels[i/3] = qRgb(constData[headerSize+i], constData[headerSize+i+1], constData[headerSize+i+2]);
			}
        }
	}

    return true;
}

bool TexFile::save(QByteArray &data) const
{
	data.append((char *)&_header, _header.version>=2 ? sizeof(TexStruct) : sizeof(TexStruct) - 4);

	qDebug() << "texSize header" << data.size();

	if(isPaletted()) {
		quint32 palID;

		for(palID=0 ; palID < (quint32)_colorTables.size() ; ++palID) {
			const QVector<QRgb> &palette = _colorTables.at(palID);
			quint32 colorID;
			for(colorID=0 ; colorID < (quint32)palette.size() ; ++colorID) {
				const QRgb &color = palette.at(colorID);
				data.append((char)qBlue(color));
				data.append((char)qGreen(color));
				data.append((char)qRed(color));
				data.append((char)qAlpha(color));
			}
		}

		qDebug() << "texSize palettes" << data.size() << _image.height() << _image.width();

		for(int y=0 ; y<_image.height() ; ++y) {
			for(int x=0 ; x<_image.width() ; ++x) {
				data.append((char)_image.pixelIndex(x, y));
			}
		}

		qDebug() << "texSize data" << data.size();

		data.append((char *)colorKeyArray.data(), colorKeyArray.size());

		qDebug() << "texSize colorKey" << data.size();
	} else {
		QRgb *pixels = (QRgb *)_image.bits();
		for(int i=0 ; i<_image.width()*_image.height() ; ++i) {
			quint16 color = PsColor::toPsColor(pixels[i]);
			data.append((char *)&color, 2);
		}

		qDebug() << "texSize data" << data.size();
	}

	return true;
}

QVector<quint8> TexFile::alpha() const
{
	QVector<quint8> ret;
	foreach (quint8 colorKey, colorKeyArray) {
		ret.append(colorKey);
	}
	return ret;
}

void TexFile::setAlpha(const QVector<quint8> &alpha)
{
	colorKeyArray.clear();
	foreach (quint8 a, alpha) {
		colorKeyArray.append(a);
	}
}

void TexFile::setHeader(Version version, bool hasAlpha, bool fourBitsPerIndex)
{
	_header = TexStruct();
	_header.version = quint32(version);
	// _header.unknown1 = 0;
	_header.hasColorKey = !colorKeyArray.isEmpty();
	_header.unknown2 = hasAlpha;
	// _header.unknown3 = 0; // FIXME: find what is that
	_header.minBitsPerColor = 4;
	_header.maxBitsPerColor = 8;
	_header.minAlphaBits = hasAlpha ? 4 : 0;
	_header.maxAlphaBits = 8;
	_header.minBitsPerPixel = !isPaletted() ? 32 : 8;
	_header.maxBitsPerPixel = 32;
	// _header.unknown4 = 0;
	_header.nbPalettes = colorTableCount();
	if (isPaletted()) {
		_header.nbColorsPerPalette1 = !fourBitsPerIndex ? 256 : 16;
	}
	_header.bitDepth = !isPaletted() ? 16 : (!fourBitsPerIndex ? 8 : 4);
	_header.imageWidth = _image.width();
	_header.imageHeight = _image.height();
	// _header.pitch = 0;
	// _header.unknown5 = 0;
	_header.hasPal = isPaletted();
	_header.bitsPerIndex = isPaletted() ? 8 : 0;
	_header.indexedTo8bit = isPaletted();
	_header.paletteSize = colorTableCount() * _header.nbColorsPerPalette1;
	_header.nbColorsPerPalette2 = _header.nbColorsPerPalette1;
	// _header.runtimeData1 = 0;
	_header.bitsPerPixel = !isPaletted() ? 16 : 8;
	_header.bytesPerPixel = !isPaletted() ? 2 : 1;
	// Pixel format
	if (!isPaletted()) {
		_header.nbRedBits1 = 5;
		_header.nbGreenBits1 = 5;
		_header.nbBlueBits1 = 5;
		_header.nbAlphaBits1 = 1;
		_header.redBitmask = 0x1F;
		_header.greenBitmask = 0x3E0;
		_header.blueBitmask = 0x7C00;
		_header.alphaBitmask = 0x8000;
		_header.redShift = 0;
		_header.greenShift = 5;
		_header.blueShift = 10;
		_header.alphaShift = 15;
		_header.nbRedBits2 = 3;
		_header.nbGreenBits2 = 3;
		_header.nbBlueBits2 = 3;
		_header.nbAlphaBits2 = 7;
		_header.redMax = 31;
		_header.greenMax = 31;
		_header.blueMax = 31;
		_header.alphaMax = 1;
	}
	// /Pixel format
	// _header.hasColorKeyArray = 0;
	// _header.runtimeData2 = 0;
	_header.referenceAlpha = 255;
	_header.runtimeData3 = 4;
	// _header.unknown6 = 0;
	// _header.paletteIndex = 0;
	// _header.runtimeData4 = 0;
	// _header.runtimeData5 = 0;
	// _header.unknown7 = 0; // FIXME: find what is that
	// _header.unknown8 = 0; // FIXME: find what is that
	// _header.unknown9 = 0; // FIXME: find what is that
	// _header.unknown10 = 0; // FIXME: find what is that

	if (_header.version >= 2) {
		// _header.unknown11 = 0; // FIXME: find what is that
	}
}

ExtraData TexFile::extraData() const
{
	QMap<QString, QVariant> ret;

	ret["version"] = _header.version;
	ret["unknown1"] = _header.unknown1;
	ret["hasColorKey"] = _header.hasColorKey;
	ret["unknown2"] = _header.unknown2;
	ret["unknown3"] = _header.unknown3;
	ret["minBitsPerColor"] = _header.minBitsPerColor;
	ret["maxBitsPerColor"] = _header.maxBitsPerColor;
	ret["minAlphaBits"] = _header.minAlphaBits;
	ret["maxAlphaBits"] = _header.maxAlphaBits;
	ret["minBitsPerPixel"] = _header.minBitsPerPixel;
	ret["maxBitsPerPixel"] = _header.maxBitsPerPixel;
	ret["unknown4"] = _header.unknown4;
	ret["nbPalettes"] = _header.nbPalettes;
	ret["nbColorsPerPalette1"] = _header.nbColorsPerPalette1;
	ret["bitDepth"] = _header.bitDepth;
	ret["pitch"] = _header.pitch;
	ret["unknown5"] = _header.unknown5;
	ret["hasPal"] = _header.hasPal;
	ret["bitsPerIndex"] = _header.bitsPerIndex;
	ret["indexedTo8bit"] = _header.indexedTo8bit;
	ret["paletteSize"] = _header.paletteSize;
	ret["nbColorsPerPalette2"] = _header.nbColorsPerPalette2;
	ret["runtimeData1"] = _header.runtimeData1;
	ret["bitsPerPixel"] = _header.bitsPerPixel;
	ret["bytesPerPixel"] = _header.bytesPerPixel;
	ret["nbRedBits1"] = _header.nbRedBits1;
	ret["nbGreenBits1"] = _header.nbGreenBits1;
	ret["nbBlueBits1"] = _header.nbBlueBits1;
	ret["nbAlphaBits1"] = _header.nbAlphaBits1;
	ret["redBitmask"] = _header.redBitmask;
	ret["greenBitmask"] = _header.greenBitmask;
	ret["blueBitmask"] = _header.blueBitmask;
	ret["alphaBitmask"] = _header.alphaBitmask;
	ret["redShift"] = _header.redShift;
	ret["greenShift"] = _header.greenShift;
	ret["blueShift"] = _header.blueShift;
	ret["alphaShift"] = _header.alphaShift;
	ret["nbRedBits2"] = _header.nbRedBits2;
	ret["nbGreenBits2"] = _header.nbGreenBits2;
	ret["nbBlueBits2"] = _header.nbBlueBits2;
	ret["nbAlphaBits2"] = _header.nbAlphaBits2;
	ret["redMax"] = _header.redMax;
	ret["greenMax"] = _header.greenMax;
	ret["blueMax"] = _header.blueMax;
	ret["alphaMax"] = _header.alphaMax;
	ret["hasColorKeyArray"] = _header.hasColorKeyArray;
	ret["runtimeData2"] = _header.runtimeData2;
	ret["referenceAlpha"] = _header.referenceAlpha;
	ret["runtimeData3"] = _header.runtimeData3;
	ret["unknown6"] = _header.unknown6;
	ret["paletteIndex"] = _header.paletteIndex;
	ret["runtimeData4"] = _header.runtimeData4;
	ret["runtimeData5"] = _header.runtimeData5;
	ret["unknown7"] = _header.unknown7;
	ret["unknown8"] = _header.unknown8;
	ret["unknown9"] = _header.unknown9;
	ret["unknown10"] = _header.unknown10;

	if (_header.version >= 2) {
		ret["unknown11"] = _header.unknown11;
	}

	return ExtraData(ret);
}

bool TexFile::setExtraData(const ExtraData &extraData)
{
	bool ok;
	QMap<QString, QVariant> fields = extraData.fields();
	setExtraDataFieldUsed(fields);

	_header.version = 1;
	quint32 hasAlpha = 0;
	quint32 fourBitsPerIndex = 0;
	setExtraDataField(_header.version, "version");
	setExtraDataField(hasAlpha, "hasAlpha");
	setExtraDataField(fourBitsPerIndex, "fourBitsPerIndex");

	// Set default values
	setHeader(Version(_header.version), hasAlpha, fourBitsPerIndex);

	setExtraDataField(_header.unknown1, "unknown1");
	setExtraDataField(_header.unknown2, "unknown2");
	setExtraDataField(_header.hasColorKey, "hasColorKey");
	setExtraDataField(_header.unknown3, "unknown3");
	setExtraDataField(_header.minBitsPerColor, "minBitsPerColor");
	setExtraDataField(_header.maxBitsPerColor, "maxBitsPerColor");
	setExtraDataField(_header.minAlphaBits, "minAlphaBits");
	setExtraDataField(_header.maxAlphaBits, "maxAlphaBits");
	setExtraDataField(_header.minBitsPerPixel, "minBitsPerPixel");
	setExtraDataField(_header.maxBitsPerPixel, "maxBitsPerPixel");
	setExtraDataField(_header.unknown4, "unknown4");
	setExtraDataField(_header.nbPalettes, "nbPalettes");
	setExtraDataField(_header.nbColorsPerPalette1, "nbColorsPerPalette1");
	setExtraDataField(_header.bitDepth, "bitDepth");
	setExtraDataField(_header.pitch, "pitch");
	setExtraDataField(_header.unknown5, "unknown5");
	setExtraDataField(_header.hasPal, "hasPal");
	setExtraDataField(_header.bitsPerIndex, "bitsPerIndex");
	setExtraDataField(_header.indexedTo8bit, "indexedTo8bit");
	setExtraDataField(_header.paletteSize, "paletteSize");
	setExtraDataField(_header.nbColorsPerPalette2, "nbColorsPerPalette2");
	setExtraDataField(_header.runtimeData1, "runtimeData1");
	setExtraDataField(_header.bitsPerPixel, "bitsPerPixel");
	setExtraDataField(_header.bytesPerPixel, "bytesPerPixel");
	setExtraDataField(_header.nbRedBits1, "nbRedBits1");
	setExtraDataField(_header.nbGreenBits1, "nbGreenBits1");
	setExtraDataField(_header.nbBlueBits1, "nbBlueBits1");
	setExtraDataField(_header.nbAlphaBits1, "nbAlphaBits1");
	setExtraDataField(_header.redBitmask, "redBitmask");
	setExtraDataField(_header.greenBitmask, "greenBitmask");
	setExtraDataField(_header.blueBitmask, "blueBitmask");
	setExtraDataField(_header.alphaBitmask, "alphaBitmask");
	setExtraDataField(_header.redShift, "redShift");
	setExtraDataField(_header.greenShift, "greenShift");
	setExtraDataField(_header.blueShift, "blueShift");
	setExtraDataField(_header.alphaShift, "alphaShift");
	setExtraDataField(_header.nbRedBits2, "nbRedBits2");
	setExtraDataField(_header.nbGreenBits2, "nbGreenBits2");
	setExtraDataField(_header.nbBlueBits2, "nbBlueBits2");
	setExtraDataField(_header.nbAlphaBits2, "nbAlphaBits2");
	setExtraDataField(_header.redMax, "redMax");
	setExtraDataField(_header.greenMax, "greenMax");
	setExtraDataField(_header.blueMax, "blueMax");
	setExtraDataField(_header.alphaMax, "alphaMax");
	setExtraDataField(_header.hasColorKeyArray, "hasColorKeyArray");
	setExtraDataField(_header.runtimeData2, "runtimeData2");
	setExtraDataField(_header.referenceAlpha, "referenceAlpha");
	setExtraDataField(_header.runtimeData3, "runtimeData3");
	setExtraDataField(_header.unknown6, "unknown6");
	setExtraDataField(_header.paletteIndex, "paletteIndex");
	setExtraDataField(_header.runtimeData4, "runtimeData4");
	setExtraDataField(_header.runtimeData5, "runtimeData5");
	setExtraDataField(_header.unknown7, "unknown7");
	setExtraDataField(_header.unknown8, "unknown8");
	setExtraDataField(_header.unknown9, "unknown9");
	setExtraDataField(_header.unknown10, "unknown10");

	if (_header.version >= 2) {
		setExtraDataField(_header.unknown11, "unknown11");
	}

	// TODO: colorKeyArray

	return ok;
}

void TexFile::debug()
{
	TexStruct h = _header;

	QFile f("debugTex.txt");
	f.open(QIODevice::WriteOnly);
	f.write(QString("version= %1 | unknown1= %2 | hasColorKey= %3 | unknown2= %4 | unknown3= %5\n")
			.arg(h.version).arg(h.unknown1).arg(h.hasColorKey).arg(h.unknown2).arg(h.unknown3).toLatin1());
	f.write(QString("minBitsPerColor= %1 | maxBitsPerColor= %2 | minAlphaBits= %3 | maxAlphaBits= %4 | minBitsPerPixel= %5\n")
			.arg(h.minBitsPerColor).arg(h.maxBitsPerColor).arg(h.minAlphaBits).arg(h.maxAlphaBits).arg(h.minBitsPerPixel).toLatin1());
	f.write(QString("maxBitsPerPixel= %1 | unknown4= %2 | nbPalettes= %3 | nbColorsPerPalette1= %4 | bitDepth= %5\n")
			.arg(h.maxBitsPerPixel).arg(h.unknown4).arg(h.nbPalettes).arg(h.nbColorsPerPalette1).arg(h.bitDepth).toLatin1());
	f.write(QString("imageWidth= %1 | imageHeight= %2 | pitch= %3 | unknown5= %4 | hasPal= %5\n")
			.arg(h.imageWidth).arg(h.imageHeight).arg(h.pitch).arg(h.unknown5).arg(h.hasPal).toLatin1());
	f.write(QString("bitsPerIndex= %1 | indexedTo8bit= %2 | paletteSize= %3 | nbColorsPerPalette2= %4 | runtimeData1= %5\n")
			.arg(h.bitsPerIndex).arg(h.indexedTo8bit).arg(h.paletteSize).arg(h.nbColorsPerPalette2).arg(h.runtimeData1).toLatin1());
	f.write(QString("bitsPerPixel= %1 | bytesPerPixel= %2 | nbRedBits1= %3 | nbGreenBits1= %4 | nbBlueBits1= %5\n")
			.arg(h.bitsPerPixel).arg(h.bytesPerPixel).arg(h.nbRedBits1).arg(h.nbGreenBits1).arg(h.nbBlueBits1).toLatin1());
	f.write(QString("nbAlphaBits1= %1 | redBitmask= %2 | greenBitmask= %3 | blueBitmask= %4 | alphaBitmask= %5\n")
			.arg(h.nbAlphaBits1).arg(h.redBitmask).arg(h.greenBitmask).arg(h.blueBitmask).arg(h.alphaBitmask).toLatin1());
	f.write(QString("redShift= %1 | greenShift= %2 | blueShift= %3 | alphaShift= %4 | nbRedBits2= %5\n")
			.arg(h.redShift).arg(h.greenShift).arg(h.blueShift).arg(h.alphaShift).arg(h.nbRedBits2).toLatin1());
	f.write(QString("nbGreenBits2= %1 | nbBlueBits2= %2 | nbAlphaBits2= %3 | redMax= %4 | greenMax= %5\n")
			.arg(h.nbGreenBits2).arg(h.nbBlueBits2).arg(h.nbAlphaBits2).arg(h.redMax).arg(h.greenMax).toLatin1());
	f.write(QString("blueMax= %1 | alphaMax= %2 | hasColorKeyArray= %3 | runtimeData2= %4 | referenceAlpha= %5\n")
			.arg(h.blueMax).arg(h.alphaMax).arg(h.hasColorKeyArray).arg(h.runtimeData2).arg(h.referenceAlpha).toLatin1());
	f.write(QString("runtimeData3= %1 | unknown6= %2 | paletteIndex= %3 | runtimeData4= %4 | runtimeData5= %5\n")
			.arg(h.runtimeData3).arg(h.unknown6).arg(h.paletteIndex).arg(h.runtimeData4).arg(h.runtimeData5).toLatin1());
	f.write(QString("unknown7= %1 | unknown8= %2 | unknown9= %3 | unknown10= %4 | unknown11= %5\n")
			.arg(h.unknown7).arg(h.unknown8).arg(h.unknown9).arg(h.unknown10).arg(h.unknown11).toLatin1());

	for(int i=0 ; i<_colorTables.size() ; ++i) {
		f.write(QString("Pal %1 ").arg(i).toLatin1());
		foreach(const QRgb &color, _colorTables.at(i)) {
			f.write(QString("(r=%1, g=%2, b=%3, a=%4) ")
					.arg(qRed(color)).arg(qGreen(color)).arg(qBlue(color)).arg(qAlpha(color))
					.toLatin1());
		}
		f.write("\n");
	}

	for(int i=0 ; i<colorKeyArray.size() ; ++i) {
		f.write(QString("%1, ")
				.arg((quint8)colorKeyArray.at(i)).toLatin1());
	}

	f.close();
}
