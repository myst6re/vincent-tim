#ifndef COLLECT_H
#define COLLECT_H

#include <QtCore>

class Collect
{
public:
	Collect(const QString &dir);
	void texData() const;
private:
	QDir _dir;
};

#endif // COLLECT_H
