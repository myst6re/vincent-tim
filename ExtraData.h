#ifndef EXTRADATA_H
#define EXTRADATA_H

#include <QMap>
#include <QVariant>

class ExtraData
{
public:
	explicit ExtraData(const QMap<QString, QVariant> &fields = QMap<QString, QVariant>());
	bool open(const QString &filename);
	bool open(QIODevice *device);
	bool save(const QString &filename) const;
	bool save(QIODevice *device) const;
	inline const QMap<QString, QVariant> &fields() const {
		return _fields;
	}
private:
	QMap<QString, QVariant> _fields;
};

#endif // EXTRADATA_H
