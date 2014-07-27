#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <QCommandLineParser>

#define TIM_ADD_ARGUMENT(names, description, valueName, defaultValue) \
	_parser.addOption(QCommandLineOption(names, description, valueName, defaultValue));

#define TIM_ADD_FLAG(names, description) \
	_parser.addOption(QCommandLineOption(names, description));

#define TIM_OPTION_NAMES(shortName, fullName) \
	(QStringList() << shortName << fullName)

class Arguments
{
public:
	Arguments();
	inline void showHelp(int exitCode = 0) {
		_parser.showHelp(exitCode);
	}

	QStringList paths() const;
	QString inputFormat(const QString &path = QString()) const;
	QString outputFormat() const;
	QString destination(const QString &source, int num = -1, int palette = -1) const;
	QString destinationMeta(const QString &source, int num = -1) const;
	QString destinationPalette(const QString &source, int num = -1) const;
	QString inputPathPalette(const QString &inputPathImage) const;
	QString inputPathMeta(const QString &inputPathImage) const;
	bool exportPalettes() const;
	bool exportMeta() const;
	bool help() const;
	int palette() const;
	bool analysis() const;
private:
	bool exportAll() const;
	void parse();
	void wilcardParse();
	static QStringList searchFiles(const QString &path);
	QString destinationPath(const QString &source, const QString &format, int num = -1, int palette = -1) const;
	QString searchRelatedFile(const QString &inputPathImage, const QString &extension) const;
	QStringList _paths;
	QString _directory;
	int _palette;
	QCommandLineParser _parser;
};

#endif // ARGUMENTS_H
