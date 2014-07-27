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
#ifndef TEXFILE_H
#define TEXFILE_H

#include <QtCore>
#include "TextureFile.h"

typedef struct {
	// Header
	quint32 version; // 1=FF7 | 2=FF8
	quint32 unknown1;  // Always 0
	quint32 hasColorKey; // [bpp = 16] 0 [else] 0 or 1
	quint32 unknown2; // 0 or 1 // hasAlphaBits? related to minAlphaBits
	quint32 unknown3; // [bpp = 16] 3 [else] Varies (0, 3, 5, 6, 7... 31)
	quint32 minBitsPerColor; // Always 4
	quint32 maxBitsPerColor;// Always 8
	quint32 minAlphaBits; // 0 or 4
	quint32 maxAlphaBits; // Always 8
	quint32 minBitsPerPixel; // [bpp = 16] 32 [else] 8
	quint32 maxBitsPerPixel; // Always 32
	quint32 unknown4; // Always 0
	quint32 nbPalettes; // [bpp = 16] 0 [else] varies (1 -> 31)
	quint32 nbColorsPerPalette1; // [bpp = 16] 0 [else] 16 or 256
	quint32 bitDepth; // [bpp = 16] 16 [bpp = 8] 8 ([bpp = 4] 4)
	quint32 imageWidth;
	quint32 imageHeight;
	quint32 pitch; // Always 0
	quint32 unknown5; // Always 0
	quint32 hasPal; // [bpp = 16] 0 [else] 1
	quint32 bitsPerIndex; // [bpp = 16] 0 [else] 8
	quint32 indexedTo8bit; // [bpp = 16] 0 [else] 1
	quint32 paletteSize; // [bpp = 16] 0 [else] varies (16, 32, 48 ... 2048)
	quint32 nbColorsPerPalette2; // [bpp = 16] 0 [else] 16 or 256 // may be 0 sometimes
	quint32 runtimeData1; // [bpp = 16] 0 [else] varies
	quint32 bitsPerPixel; // [bpp = 16] 16 [else] 8
	quint32 bytesPerPixel; // [bpp = 16] 2 [else] 1
	// Pixel format
	quint32 nbRedBits1; // [bpp = 16] 5 [else] 0
	quint32 nbGreenBits1; // [bpp = 16] 5 [else] 0
	quint32 nbBlueBits1; // [bpp = 16] 5 [else] 0
	quint32 nbAlphaBits1; // [bpp = 16] 1 [else] 0
	quint32 redBitmask; // [bpp = 16] 31 [else] 0
	quint32 greenBitmask; // [bpp = 16] 992 [else] 0
	quint32 blueBitmask; // [bpp = 16] 31744 [else] 0
	quint32 alphaBitmask; // [bpp = 16] 32768 [else] 0
	quint32 redShift; // [bpp = 16] 0 [else] 0
	quint32 greenShift; // [bpp = 16] 5 [else] 0
	quint32 blueShift; // [bpp = 16] 10 [else] 0
	quint32 alphaShift; // [bpp = 16] 15 [else] 0
	quint32 nbRedBits2;  // [bpp = 16] 3 [else] 0
	quint32 nbGreenBits2; // [bpp = 16] 3 [else] 0
	quint32 nbBlueBits2; // [bpp = 16] 3 [else] 0
	quint32 nbAlphaBits2; // [bpp = 16] 7 [else] 0
	quint32 redMax; // [bpp = 16] 31 [else] 0
	quint32 greenMax; // [bpp = 16] 31 [else] 0
	quint32 blueMax; // [bpp = 16] 31 [else] 0
	quint32 alphaMax; // [bpp = 16] 1 [else] 0
	// /Pixel format
	quint32 hasColorKeyArray; // Always 0
	quint32 runtimeData2; // Always 0
	quint32 referenceAlpha; // Always 255
	quint32 runtimeData3; // Always 4
	quint32 unknown6; // Always 0
	quint32 paletteIndex; // Always 0
	quint32 runtimeData4; // Varies, sometimes 0
	quint32 runtimeData5; // Varies, sometimes 0
	quint32 unknown7; // [bpp = 16] 0 [else] Varies (0, 16, 32, 48 ... 768)
	quint32 unknown8; // [bpp = 16] 0 [else] Varies (0, 128, 129 ... 511, 512)
	quint32 unknown9; // [bpp = 16] 0 or 896 [else] Varies (0, 216 ... 1020)
	quint32 unknown10; // Varies (16, 32, 48 ... 960)
	quint32 unknown11; // Varies (0, 128, 192 or 256) // only on ff8! (version >= 2)
} TexStruct;

class TexFile : public TextureFile
{
public:
	enum Version {
		One = 1, Two = 2
	};

	TexFile();
	TexFile(const TextureFile &textureFile, const TexStruct &header,
			const QVector<quint8> &colorKeyArray=QVector<quint8>());
	TexFile(const TextureFile &textureFile, Version version, bool hasAlpha, bool fourBitsPerIndex=false,
			const QVector<quint8> &colorKeyArray=QVector<quint8>());
	TexFile(const TextureFile &texture);
	bool open(const QByteArray &data);
	bool save(QByteArray &data) const;
	inline quint8 depth() const {
		return _header.bitDepth;
	}
	void debug();
	
	ExtraData extraData() const;
	bool setExtraData(const ExtraData &extraData);

	virtual inline quint16 colorPerPal() const {
		return _header.nbColorsPerPalette1;
	}
	inline TexStruct header() const {
		return _header;
	}
	void setHeader(Version version, bool hasAlpha, bool fourBitsPerIndex=false);
private:
	TexStruct _header;
	QVector<quint8> colorKeyArray;
};

#endif // TEXFILE_H
