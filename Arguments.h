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
