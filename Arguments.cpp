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
#include "Arguments.h"
#include <QCoreApplication>
#include <QDir>
#include "TextureFile.h"

Arguments::Arguments() :
	_palette(-1)
{
	_parser.addHelpOption();
	_parser.addVersionOption();
	
	TIM_ADD_ARGUMENT(TIM_OPTION_NAMES("if", "input-format"),
	                 "Input format (*tim*, tex, png, jpg, bmp).",
	                 "input-format", "");
	TIM_ADD_ARGUMENT(TIM_OPTION_NAMES("of", "output-format"),
	                 "Output format (tim, tex, *png*, jpg, bmp).",
	                 "output-format", "png");
	TIM_ADD_ARGUMENT(TIM_OPTION_NAMES("p", "palette"),
	                 "If the input format is a texture: Select the palette to extract.",
	                 "palette", "-1");
	TIM_ADD_ARGUMENT("input-path-palette",
	                 "If the output format is a texture: path to the palette file.",
	                 "input-path-palette", "");
	TIM_ADD_ARGUMENT("input-path-meta",
	                 "If the output format is a texture: path to the meta file.",
	                 "input-path-meta", "");
	TIM_ADD_FLAG(TIM_OPTION_NAMES("ep", "export-palette"),
	                 "Save palette colors into a 'output format' file.");
	TIM_ADD_FLAG(TIM_OPTION_NAMES("em", "export-meta"),
	                 "Save meta data in a text file.");
	TIM_ADD_FLAG(TIM_OPTION_NAMES("e", "export-all"),
	                 "Alias for --ep --em.");
	TIM_ADD_FLAG(TIM_OPTION_NAMES("a", "analysis"),
	             "Analysis mode, search TIM files into the input file.");

	_parser.addPositionalArgument("files", QCoreApplication::translate("Arguments", "Input files."), "[files...]");
	_parser.addPositionalArgument("directory", QCoreApplication::translate("Arguments", "Output directory."), "[directory]");

	parse();
}

QStringList Arguments::paths() const
{
	return _paths;
}

QString Arguments::inputFormat(const QString &path) const
{
	QString inputFormat = _parser.value("input-format");
	if(inputFormat.isEmpty()) {
		int index = path.lastIndexOf('.');
		if(index > -1) {
			return path.mid(index + 1);
		}
	}
	return inputFormat;
}

QString Arguments::outputFormat() const
{
	return _parser.value("output-format");
}

QString Arguments::destinationPath(const QString &source, const QString &format, int num, int palette) const
{
	QString destPath,
	        sourceFilename = source.mid(source.lastIndexOf('/') + 1);

	if (!_directory.isEmpty()) {
		destPath = QString("%1/%2").arg(_directory, sourceFilename);
	} else {
		destPath = sourceFilename;
	}

	if (num >= 0) {
		destPath.append(QString(".%1").arg(num));
	}

	if (palette >= 0) {
		destPath.append(QString(".%1").arg(palette));
	}

	return destPath.append(".").append(format);
}

QString Arguments::destination(const QString &source, int num, int palette) const
{
	return destinationPath(source, outputFormat(), num, palette);
}

QString Arguments::destinationMeta(const QString &source, int num) const
{
	return destinationPath(source, "meta", num);
}

QString Arguments::destinationPalette(const QString &source, int num) const
{
	return destinationPath(source, "palette." + outputFormat(), num);
}

QString Arguments::searchRelatedFile(const QString &inputPathImage, const QString &extension) const
{
	int indexInputExtension;
	QString inputPathTruncated = inputPathImage;

	while ((indexInputExtension = inputPathTruncated.lastIndexOf('.')) >= 0) {
		inputPathTruncated.truncate(indexInputExtension);
		if (QFile::exists(inputPathTruncated + "." + extension)) {
			return inputPathTruncated + "." + extension;
		}
	}

	return QString();
}

QString Arguments::inputPathPalette(const QString &inputPathImage) const
{
	QString palette = _parser.value("input-path-palette");
	if (palette.isEmpty()) {
		QString inputExtension = inputPathImage.mid(inputPathImage.lastIndexOf("."));
		return searchRelatedFile(inputPathImage, "palette" + inputExtension);
	}
	return palette;
}

QString Arguments::inputPathMeta(const QString &inputPathImage) const
{
	QString meta = _parser.value("input-path-meta");
	if (meta.isEmpty()) {
		return searchRelatedFile(inputPathImage, "meta");
	}
	return meta;
}

bool Arguments::exportPalettes() const
{
	return _parser.isSet("export-palette") || exportAll();
}

bool Arguments::exportMeta() const
{
	return _parser.isSet("export-meta") || exportAll();
}

bool Arguments::exportAll() const
{
	return _parser.isSet("export-all");
}

bool Arguments::help() const
{
	return _parser.isSet("help");
}

int Arguments::palette() const
{
	return _palette;
}

bool Arguments::analysis() const
{
	return _parser.isSet("analysis");
}

void Arguments::parse()
{
	bool ok;

	_parser.process(*qApp);

	wilcardParse();

	_palette = _parser.value("palette").toInt(&ok);
	if (!ok) {
		_palette = -1;
	}
}

QStringList Arguments::searchFiles(const QString &path)
{
	int index = path.lastIndexOf('/');
	QString dirname, filename;

	if (index > 0) {
		dirname = path.left(index);
		filename = path.mid(index + 1);
	} else {
		filename = path;
	}

	QDir dir(dirname);
	QStringList entryList = dir.entryList(QStringList(filename), QDir::Files);
	int i=0;
	foreach (const QString &entry, entryList) {
		entryList.replace(i++, dir.filePath(entry));
	}
	return entryList;
}

void Arguments::wilcardParse()
{
	QStringList paths;

	foreach (const QString &path, _parser.positionalArguments()) {
		if (path.contains('*') || path.contains('?')) {
			paths << searchFiles(QDir::fromNativeSeparators(path));
		} else {
			paths << QDir::fromNativeSeparators(path);
		}
	}

	if (!paths.isEmpty()) {
		// Output directory
		if (QDir(paths.last()).exists()) {
			_directory = paths.takeLast();
		}

		_paths = paths;
	}
}
