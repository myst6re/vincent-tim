#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <QString>
#include <QStringList>
#include <QMap>

class Arguments
{
public:
	Arguments();
	const QStringList &paths() const;
	QString inputFormat(const QString &path = QString()) const;
	const QString &outputFormat() const;
	const QString &destination() const;
	const QString &inputPathPalette() const;
	const QString &inputPathMeta() const;
	bool exportPalettes() const;
	bool exportMeta() const;
	bool help() const;
	int palette() const;
	bool analysis() const;
	QMap<QString, QString> commands() const;
private:
	void parse();
	void wilcardParse();
	static QStringList searchFiles(const QString &path);
	QStringList _paths;
	QString _inputFormat, _outputFormat, _destination;
	QString _inputPathPalette, _inputPathMeta;
	bool _exportPalettes, _exportMeta, _help, _analysis;
	int _palette;
};

#endif // ARGUMENTS_H
