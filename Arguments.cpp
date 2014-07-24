#include "Arguments.h"
#include <QCoreApplication>
#include <QDir>
#include "TextureFile.h"

Arguments::Arguments() :
	_inputFormat(), _outputFormat("png"),
    _exportPalettes(false), _exportMeta(false),
	_help(false), _analysis(false), _palette(-1)
{
	parse();
}

const QStringList &Arguments::paths() const
{
	return _paths;
}

QString Arguments::inputFormat(const QString &path) const
{
	if(_inputFormat.isEmpty()) {
		int index = path.lastIndexOf('.');
		if(index > -1) {
			return path.mid(index + 1);
		}
	}
	return _inputFormat;
}

const QString &Arguments::outputFormat() const
{
	return _outputFormat;
}

const QString &Arguments::destination() const
{
	return _destination;
}

const QString &Arguments::inputPathPalette() const
{
	return _inputPathPalette;
}

const QString &Arguments::inputPathMeta() const
{
	return _inputPathMeta;
}

bool Arguments::exportPalettes() const
{
	return _exportPalettes;
}

bool Arguments::exportMeta() const
{
	return _exportMeta;
}

bool Arguments::help() const
{
	return _help;
}

int Arguments::palette() const
{
	return _palette;
}

bool Arguments::analysis() const
{
	return _analysis;
}

void Arguments::parse()
{
	QStringList args = qApp->arguments();
	args.removeFirst();// Application path

	_help = false;
	_paths.clear();

	while (!args.isEmpty()) {
		const QString &arg = args.takeFirst();

		if ((arg == "-if" || arg == "--input-format") && !args.isEmpty()) {
			_inputFormat = args.takeFirst();
		} else if ((arg == "-of" || arg == "--output-format") && !args.isEmpty()) {
			_outputFormat = args.takeFirst();
		} else if (arg == "-h" || arg == "--help") {
			_help = true;
		} else if ((arg == "-p" || arg == "--palette") && !args.isEmpty()) {
			bool ok;
			_palette = args.takeFirst().toInt(&ok);
			if(!ok) {
				_palette = -1;
			}
		} else if (arg == "--input-path-palette" && !args.isEmpty()) {
			_inputPathPalette = args.takeFirst();
		} else if (arg == "--input-path-meta" && !args.isEmpty()) {
			_inputPathMeta = args.takeFirst();
		} else if ((arg == "-d" || arg == "--destination") && !args.isEmpty()) {
			_destination = args.takeFirst();
		} else if (arg == "-ep" || arg == "--export-palette") {
			_exportPalettes = true;
		} else if (arg == "-em" || arg == "--export-meta") {
			_exportMeta = true;
		} else if (arg == "-ea" || arg == "--export-all") {
			_exportPalettes = true;
			_exportMeta = true;
		} else if (arg == "-a" || arg == "--analysis") {
			_analysis = true;
		} else {
			_paths << QDir::fromNativeSeparators(arg);
		}
	}

	wilcardParse();
}

QMap<QString, QString> Arguments::commands() const
{
	QMap<QString, QString> options;

	options["-if --input-format"] = "Input format (*tim*, tex, png, jpg, bmp).";
	options["-of --output-format"] = "Output format (tim, tex, *png*, jpg, bmp).";
	options["-h --help"] = "Show this help and quit.";
	// TODO: and how select several palettes?
	options["-p --palette"] = "If the input format is a texture: Select the palette to extract.";
	options["--input-path-palette"] = "If the output format is a texture: path to the palette file.";
	options["--input-path-meta"] = "If the output format is a texture: path to the meta file.";
	options["-d --destination"] = "Destination directory.";
	options["-ep --export-palette"] = "Save palette colors into a 'output format' file.";
	options["-em --export-meta"] = "Save meta data in a text file.";
	options["-ea --export-all"] = "Alias for -ep -em.";
	options["-a --analysis"] = "Analysis mode, search TIM files into the input file.";
	// options["-q --quiet"] = "Suppress all outputs"; // TODO?

	return options;
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

	return QDir(dirname).entryList(QStringList() << filename, QDir::Files);
}

void Arguments::wilcardParse()
{
	QStringList paths;

	foreach (const QString &path, _paths) {
		if (path.contains('*') || path.contains('?')) {
			paths << searchFiles(path);
		} else {
			paths << path;
		}
	}

	_paths = paths;
}
