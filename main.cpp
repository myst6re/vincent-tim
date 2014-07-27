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
	if (!texture->image().save(destPath)) {
		return false;
	}

	printf("%s\n", qPrintable(destPath));
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
					printf("%s\n", qPrintable(destPathMeta));
				}
			}
		}

		if (args.exportPalettes() && texture->depth() < 16) {
			QImage palette = texture->palette();
			if (!palette.isNull()) {
				QString destPathPalette = args.destinationPalette(path, num);
				if (!palette.save(destPathPalette)) {
					qWarning() << "Error: Cannot save palette";
					return;
				}
				printf("%s\n", qPrintable(destPathPalette));
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
	QString pathMeta = args.inputPathMeta(path);

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
		qWarning() << "Meta data not found!" << pathMeta;
		goto toTextureError;
	}
	tex->setExtraData(meta);

	destPath = args.destination(path, num);

	if (!tex->saveToFile(destPath)) {
		goto toTextureError;
	}

	printf("%s\n", qPrintable(destPath));

	delete tex;
	return true;
toTextureError:
	delete tex;
	return false;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName("tim");
	QCoreApplication::setApplicationVersion("1.0");
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
						}
					} else {
						qWarning() << "Error: Cannot open Texture file";
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
						}
						delete texture;
					}
				}
			} else {
				qWarning() << "Error: cannot open file" << path << f.errorString();
			}
		}
	}

	QTimer::singleShot(0, &a, SLOT(quit()));

	return a.exec();
}
