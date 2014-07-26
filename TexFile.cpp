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
	TextureFile(), header(TexStruct())
{
}

TexFile::TexFile(const TextureFile &textureFile, const TexStruct &header,
		const QVector<quint8> &colorKeyArray) :
	TextureFile(textureFile), header(header), colorKeyArray(colorKeyArray)
{
}

TexFile::TexFile(const TextureFile &texture) :
    TextureFile(texture), header(TexStruct())
{
}

bool TexFile::open(const QByteArray &data)
{
    const char *constData = data.constData();
	quint32 w, h, headerSize, paletteSectionSize, imageSectionSize, colorKeySectionSize;

	if((quint32)data.size() < sizeof(TexStruct)) {
		qWarning() << "tex size too short!";
		return false;
	}

	memcpy(&header, constData, sizeof(TexStruct));

	if(header.version == 1) {
		headerSize = sizeof(TexStruct) - 4;
	} else if(header.version == 2) {
		headerSize = sizeof(TexStruct);
	} else {
		qWarning() << "unknown tex version!";
		return false;
	}

	w = header.imageWidth;
	h = header.imageHeight;
	paletteSectionSize = header.nbPalettes > 0 ? header.paletteSize * 4 : 0;
	imageSectionSize = w * h * header.bytesPerPixel;
	colorKeySectionSize = header.hasColorKeyArray ? header.nbPalettes : 0;

	if((quint32)data.size() != headerSize + paletteSectionSize + imageSectionSize + colorKeySectionSize) {
		qWarning() << "tex invalid size!";
		return false;
	}

	quint32 i;

	if(header.nbPalettes > 0)
	{
		quint32 index, imageStart = headerSize + paletteSectionSize;

		for(quint32 palID=0 ; palID < header.nbPalettes ; ++palID) {
			quint32 paletteStart = headerSize+header.nbColorsPerPalette1*4*palID;

			_image = QImage(w, h, QImage::Format_Indexed8);
			QVector<QRgb> colors;

			for(i=0 ; i<header.nbColorsPerPalette1 ; ++i)
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

		if(header.hasColorKeyArray) {
			quint32 colorKeyStart = imageStart + imageSectionSize;

			for(quint32 j=0 ; j<header.nbPalettes ; ++j) {
				colorKeyArray.append(data.at(colorKeyStart+j));
			}
		}
    }
    else
    {
		quint16 color;
		_image = QImage(w, h, QImage::Format_ARGB32);
		QRgb *pixels = (QRgb *)_image.bits();

		for(i=0 ; i<imageSectionSize ; i+=header.bytesPerPixel) {
			if(header.bytesPerPixel == 2) {
				memcpy(&color, &constData[headerSize+i], 2);
				pixels[i/2] = PsColor::fromPsColor(color);
			} else if(header.bytesPerPixel == 3) {
				pixels[i/3] = qRgb(constData[headerSize+i], constData[headerSize+i+1], constData[headerSize+i+2]);
			}
        }
	}

    return true;
}

bool TexFile::save(QByteArray &data) const
{
	data.append((char *)&header, header.version>=2 ? sizeof(TexStruct) : sizeof(TexStruct) - 4);

	qDebug() << "texSize header" << data.size();

	if(isPaletted()) {
		quint32 palID;

		for(palID=0 ; palID < header.nbPalettes && palID < (quint32)_colorTables.size() ; ++palID) {
			const QVector<QRgb> &palette = _colorTables.at(palID);
			quint32 colorID;
			for(colorID=0 ; colorID < header.nbColorsPerPalette1 && colorID < (quint32)palette.size() ; ++colorID) {
				const QRgb &color = palette.at(colorID);
				data.append((char)qBlue(color));
				data.append((char)qGreen(color));
				data.append((char)qRed(color));
				data.append((char)qAlpha(color));
			}
			for( ; colorID < header.nbColorsPerPalette1 ; ++colorID) {
				const QRgb color = qRgba(0, 0, 0, 0);
				data.append((char *)&color, 4);
			}
		}

		for( ; palID < header.nbPalettes ; ++palID) {
			for(quint32 colorID=0 ; colorID < header.nbColorsPerPalette1 ; ++colorID) {
				const QRgb color = qRgba(0, 0, 0, 0);
				data.append((char *)&color, 4);
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

ExtraData TexFile::extraData() const
{
	QMap<QString, QVariant> ret;

	ret["version"] = header.version;
	ret["unknown1"] = header.unknown1;
	ret["hasColorKey"] = header.hasColorKey;
	ret["unknown2"] = header.unknown2;
	ret["unknown3"] = header.unknown3;
	ret["minBitsPerColor"] = header.minBitsPerColor;
	ret["maxBitsPerColor"] = header.maxBitsPerColor;
	ret["minAlphaBits"] = header.minAlphaBits;
	ret["maxAlphaBits"] = header.maxAlphaBits;
	ret["minBitsPerPixel"] = header.minBitsPerPixel;
	ret["maxBitsPerPixel"] = header.maxBitsPerPixel;
	ret["unknown4"] = header.unknown4;
	ret["nbPalettes"] = header.nbPalettes;
	ret["nbColorsPerPalette1"] = header.nbColorsPerPalette1;
	ret["bitDepth"] = header.bitDepth;
	ret["pitch"] = header.pitch;
	ret["unknown5"] = header.unknown5;
	ret["hasPal"] = header.hasPal;
	ret["bitsPerIndex"] = header.bitsPerIndex;
	ret["indexedTo8bit"] = header.indexedTo8bit;
	ret["paletteSize"] = header.paletteSize;
	ret["nbColorsPerPalette2"] = header.nbColorsPerPalette2;
	ret["runtimeData1"] = header.runtimeData1;
	ret["bitsPerPixel"] = header.bitsPerPixel;
	ret["bytesPerPixel"] = header.bytesPerPixel;
	ret["nbRedBits1"] = header.nbRedBits1;
	ret["nbGreenBits1"] = header.nbGreenBits1;
	ret["nbBlueBits1"] = header.nbBlueBits1;
	ret["nbAlphaBits1"] = header.nbAlphaBits1;
	ret["redBitmask"] = header.redBitmask;
	ret["greenBitmask"] = header.greenBitmask;
	ret["blueBitmask"] = header.blueBitmask;
	ret["alphaBitmask"] = header.alphaBitmask;
	ret["redShift"] = header.redShift;
	ret["greenShift"] = header.greenShift;
	ret["blueShift"] = header.blueShift;
	ret["alphaShift"] = header.alphaShift;
	ret["nbRedBits2"] = header.nbRedBits2;
	ret["nbGreenBits2"] = header.nbGreenBits2;
	ret["nbBlueBits2"] = header.nbBlueBits2;
	ret["nbAlphaBits2"] = header.nbAlphaBits2;
	ret["redMax"] = header.redMax;
	ret["greenMax"] = header.greenMax;
	ret["blueMax"] = header.blueMax;
	ret["hasColorKeyArray"] = header.hasColorKeyArray;
	ret["runtimeData2"] = header.runtimeData2;
	ret["referenceAlpha"] = header.referenceAlpha;
	ret["runtimeData3"] = header.runtimeData3;
	ret["unknown6"] = header.unknown6;
	ret["paletteIndex"] = header.paletteIndex;
	ret["runtimeData4"] = header.runtimeData4;
	ret["runtimeData5"] = header.runtimeData5;
	ret["unknown7"] = header.unknown7;
	ret["unknown8"] = header.unknown8;
	ret["unknown9"] = header.unknown9;
	ret["unknown10"] = header.unknown10;

	if (header.version >= 2) {
		ret["unknown11"] = header.unknown11;
	}

	return ExtraData(ret);
}

bool TexFile::setExtraData(const ExtraData &extraData)
{
	bool ok;
	QMap<QString, QVariant> fields = extraData.fields();
	setExtraDataFieldUsed(fields);

	setExtraDataField(header.version, "version");
	setExtraDataField(header.unknown1, "unknown1");
	setExtraDataField(header.hasColorKey, "hasColorKey");
	setExtraDataField(header.unknown2, "unknown2");
	setExtraDataField(header.unknown3, "unknown3");
	setExtraDataField(header.minBitsPerColor, "minBitsPerColor");
	setExtraDataField(header.maxBitsPerColor, "maxBitsPerColor");
	setExtraDataField(header.minAlphaBits, "minAlphaBits");
	setExtraDataField(header.maxAlphaBits, "maxAlphaBits");
	setExtraDataField(header.minBitsPerPixel, "minBitsPerPixel");
	setExtraDataField(header.maxBitsPerPixel, "maxBitsPerPixel");
	setExtraDataField(header.unknown4, "unknown4");
	setExtraDataField(header.nbPalettes, "nbPalettes");
	setExtraDataField(header.nbColorsPerPalette1, "nbColorsPerPalette1");
	setExtraDataField(header.bitDepth, "bitDepth");
	setExtraDataField(header.pitch, "pitch");
	setExtraDataField(header.unknown5, "unknown5");
	setExtraDataField(header.hasPal, "hasPal");
	setExtraDataField(header.bitsPerIndex, "bitsPerIndex");
	setExtraDataField(header.indexedTo8bit, "indexedTo8bit");
	setExtraDataField(header.paletteSize, "paletteSize");
	setExtraDataField(header.nbColorsPerPalette2, "nbColorsPerPalette2");
	setExtraDataField(header.runtimeData1, "runtimeData1");
	setExtraDataField(header.bitsPerPixel, "bitsPerPixel");
	setExtraDataField(header.bytesPerPixel, "bytesPerPixel");
	setExtraDataField(header.nbRedBits1, "nbRedBits1");
	setExtraDataField(header.nbGreenBits1, "nbGreenBits1");
	setExtraDataField(header.nbBlueBits1, "nbBlueBits1");
	setExtraDataField(header.nbAlphaBits1, "nbAlphaBits1");
	setExtraDataField(header.redBitmask, "redBitmask");
	setExtraDataField(header.greenBitmask, "greenBitmask");
	setExtraDataField(header.blueBitmask, "blueBitmask");
	setExtraDataField(header.alphaBitmask, "alphaBitmask");
	setExtraDataField(header.redShift, "redShift");
	setExtraDataField(header.greenShift, "greenShift");
	setExtraDataField(header.blueShift, "blueShift");
	setExtraDataField(header.alphaShift, "alphaShift");
	setExtraDataField(header.nbRedBits2, "nbRedBits2");
	setExtraDataField(header.nbGreenBits2, "nbGreenBits2");
	setExtraDataField(header.nbBlueBits2, "nbBlueBits2");
	setExtraDataField(header.nbAlphaBits2, "nbAlphaBits2");
	setExtraDataField(header.redMax, "redMax");
	setExtraDataField(header.greenMax, "greenMax");
	setExtraDataField(header.blueMax, "blueMax");
	setExtraDataField(header.hasColorKeyArray, "hasColorKeyArray");
	setExtraDataField(header.runtimeData2, "runtimeData2");
	setExtraDataField(header.referenceAlpha, "referenceAlpha");
	setExtraDataField(header.runtimeData3, "runtimeData3");
	setExtraDataField(header.unknown6, "unknown6");
	setExtraDataField(header.paletteIndex, "paletteIndex");
	setExtraDataField(header.runtimeData4, "runtimeData4");
	setExtraDataField(header.runtimeData5, "runtimeData5");
	setExtraDataField(header.unknown7, "unknown7");
	setExtraDataField(header.unknown8, "unknown8");
	setExtraDataField(header.unknown9, "unknown9");
	setExtraDataField(header.unknown10, "unknown10");

	if (header.version >= 2) {
		setExtraDataField(header.unknown11, "unknown11");
	}

	// TODO: set meta dynamically according to the meta data
	// TODO: colorKeyArray

	return ok;
}

void TexFile::debug()
{
	TexStruct h = header;

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