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

QString Arguments::destination() const
{
	return _directory;
}

QString Arguments::inputPathPalette() const
{
	return _parser.value("input-path-palette");
}

QString Arguments::inputPathMeta() const
{
	return _parser.value("input-path-meta");
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
