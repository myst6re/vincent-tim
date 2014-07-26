#include "Collect.h"
#include "TexFile.h"

Collect::Collect(const QString &dir) :
    _dir(dir)
{
}

void Collect::texData() const
{
	QStringList texFilenames = _dir.entryList(QStringList("*.tex"), QDir::Files);
	QMap<QString, QMap<quint32, int> > occurrences;

	foreach (const QString &texFilename, texFilenames) {
		TexFile tex;
		if (tex.openFromFile(_dir.filePath(texFilename))) {
			if (tex.header().bitDepth != 8) {
				continue;
			}
			QMap<QString, QVariant> fields = tex.extraData().fields();
			QMapIterator<QString, QVariant> it(fields);
			while (it.hasNext()) {
				it.next();
				bool ok;
				quint32 val = it.value().toUInt(&ok);
				if (!ok) {
					qDebug() << "Collect::texData not an int" << it.key() << it.value();
					continue;
				}
				QMap<quint32, int> counts = occurrences.value(it.key());
				int count = counts.value(val, 0);
				counts.insert(val, count + 1);
				occurrences.insert(it.key(), counts);
			}
		} else {
			qDebug() << texFilename << "cannot open file";
		}
	}

	QMapIterator<QString, QMap<quint32, int> > it(occurrences);
	while (it.hasNext()) {
		it.next();
		const QString &name = it.key();
		const QMap<quint32, int> &values = it.value();

		qDebug() << name;

		QMapIterator<quint32, int> itValues(values);
		while (itValues.hasNext()) {
			itValues.next();
			qDebug() << "\t" << itValues.key() << "-> x" << itValues.value();
		}
	}
}