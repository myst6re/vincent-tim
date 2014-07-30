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
#include <QtCore>
#include "Arguments.h"
#include "TimFile.h"
#include "TexFile.h"
#include "TextureImageFile.h"

//#define TESTS_ENABLED

#ifdef TESTS_ENABLED
#include "tests/Collect.h"
#endif

bool saveTextureTo(TextureFile *texture, const QString &destPath)
{
	if (!texture->image().convertToFormat(QImage::Format_ARGB32).save(destPath)) {
		return false;
	}

	printf("%s\n", qPrintable(QDir::toNativeSeparators(destPath)));
	return true;
}

void fromTexture(TextureFile *texture, const QString &path, const Arguments &args, int num = -1)
{
	QString destPathTexture;
	bool error = false;

	if (args.palette() < 0 || args.palette() >= texture->colorTableCount()) {
		if (texture->colorTableCount() <= 0) {
			destPathTexture = args.destination(path, num);
			if (!saveTextureTo(texture, destPathTexture)) {
				error = true;
			}
		}

		for (int paletteID=0; paletteID<texture->colorTableCount(); ++paletteID) {
			texture->setCurrentColorTable(paletteID);
			destPathTexture = args.destination(path, num, paletteID);
			if (!saveTextureTo(texture, destPathTexture)) {
				error = true;
			}
		}
	} else {
		destPathTexture = args.destination(path, num);
		if (!saveTextureTo(texture, destPathTexture)) {
			error = true;
		}
	}

	if (!error) {
		if (args.exportMeta()) {
			ExtraData meta = texture->extraData();
			if (!meta.fields().isEmpty()) {
				QString destPathMeta = args.destinationMeta(path, num);
				if (!meta.save(destPathMeta)) {
					qWarning() << "Error: Cannot save extra data";
					return;
				} else {
					printf("%s\n", qPrintable(QDir::toNativeSeparators(destPathMeta)));
				}
			}
		}

		if (args.exportPalettes() && texture->depth() < 16) { // Do not use isPaletted for that!
			QImage palette = texture->palette();
			if (!palette.isNull()) {
				QString destPathPalette = args.destinationPalette(path, num);
				if (!palette.save(destPathPalette)) {
					qWarning() << "Error: Cannot save palette";
					return;
				}
				printf("%s\n", qPrintable(QDir::toNativeSeparators(destPathPalette)));
			} else {
				qWarning() << "Warning: No palette to export";
				return;
			}
		}
	}

	if (error) {
		qWarning() << "Error: Cannot save image";
	}
}

bool toTexture(TextureFile *texture, const QString &path, const Arguments &args, int num = -1)
{
	QString pathMeta = args.inputPathMeta(path),
	        pathPalette = args.inputPathPalette(path);

	if (pathMeta.isEmpty()) {
		qWarning() << "Error: Please set the input path meta";
		return false;
	}

	TextureFile *tex;
	QString destPath;

	if (args.outputFormat().compare("tex", Qt::CaseInsensitive) == 0) {
		tex = new TexFile(*texture);
	} else if (args.outputFormat().compare("tim", Qt::CaseInsensitive) == 0) {
		tex = new TimFile(*texture);
	} else {
		qWarning() << "toTexture: output format not supported";
		return false;
	}

	ExtraData meta;
	if (!meta.open(pathMeta)) {
		qWarning() << "Meta data not found!" << QDir::toNativeSeparators(pathMeta);
		goto toTextureError;
	}
	tex->setExtraData(meta);

	// Not texture to texture
	if (args.outputFormat().compare(args.inputFormat(path)) != 0
	        && tex->depth() < 16) { // Do not use isPaletted for that!
		if (pathPalette.isEmpty()) {
			qWarning() << "Error: Please set the input path palette";
			return false;
		}

		QImage paletteImage;
		if (paletteImage.load(pathPalette)) {
			if (!tex->setPalette(paletteImage)) {
				qWarning() << "Error: Please set the depth in the meta file";
				return false;
			}

			if (args.palette() < 0 || args.palette() >= tex->colorTableCount()) {
				qWarning() << "Error: Please set a valid number of palette";
				return false;
			}

			if (!tex->convertToIndexedFormat(args.palette())) {
				qWarning() << "Error: Colors in the image does not match with the palette, have you changed some colors?";
				return false;
			}
		} else {
			qWarning() << "Error: Cannot open the input palette";
			return false;
		}
	}

	destPath = args.destination(path, num);

	if (!tex->saveToFile(destPath)) {
		goto toTextureError;
	}

	printf("%s\n", qPrintable(QDir::toNativeSeparators(destPath)));

	delete tex;
	return true;
toTextureError:
	delete tex;
	return false;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName("Vincent Tim");
	QCoreApplication::setApplicationVersion("1.1");
#ifdef Q_OS_WIN
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM 850"));
#endif
	
#ifdef TESTS_ENABLED
	qDebug() << "Running tests...";
	
	Collect c("tests/tim/files");
	c.textureData("tim");
#endif

	Arguments args;

	if (args.help() || args.paths().isEmpty()) {
		args.showHelp();
	} else {
		foreach (const QString &path, args.paths()) {
			TextureFile *texture;

			if (args.inputFormat(path) == args.outputFormat()) {
				qWarning() << "Error: input and output formats are not different";
				a.exit(1);
			}

			QFile f(path);
			if (f.open(QIODevice::ReadOnly)) {

				if (!args.analysis()) {
					texture = TextureFile::factory(args.inputFormat(path));

					if (texture->open(f.readAll())) {
						if (TextureFile::supportedTextureFormats().contains(args.outputFormat())) {
							if (!toTexture(texture, path, args)) {
								break;
							}
						} else if (TextureFile::supportedTextureFormats().contains(args.inputFormat(path))) {
							fromTexture(texture, path, args);
						} else {
							qWarning() << "Error: input format or output format must be a supported texture format" << TextureFile::supportedTextureFormats();
							a.exit(1);
						}
					} else {
						qWarning() << "Error: Cannot open Texture file";
						a.exit(1);
					}

					f.close();

					delete texture;
				} else { // Search tim files
					QByteArray data = f.readAll();
					f.close();
					QList<PosSize> positions = TimFile::findTims(data);

					int num = 0;
					foreach (const PosSize &pos, positions) {
						texture = new TimFile();
						if (texture->open(data.mid(pos.first, pos.second))) {
							if (args.outputFormat().compare("tim", Qt::CaseInsensitive) == 0) {
								if (!texture->saveToFile(args.destination(path, num))) {
									break;
								}
							} else {
								fromTexture(texture, path, args, num++);
							}
						} else {
							qWarning() << "Error: Cannot open Texture file";
							a.exit(1);
						}
						delete texture;
					}
				}
			} else {
				qWarning() << "Error: cannot open file" << QDir::toNativeSeparators(path) << f.errorString();
				a.exit(1);
			}
		}
	}

	QTimer::singleShot(0, &a, SLOT(quit()));

	return a.exec();
}
