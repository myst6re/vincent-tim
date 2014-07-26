#ifndef COLLECT_H
#define COLLECT_H

#include <QtCore>

class Collect
{
public:
	Collect(const QString &dir);
	void textureData(const QString &format) const;
private:
	QDir _dir;
};

#endif // COLLECT_H
