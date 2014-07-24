#include "TextureImageFile.h"
#include <QBuffer>

TextureImageFile::TextureImageFile(const char *format) :
    _format(format)
{
}

bool TextureImageFile::open(const QByteArray &data)
{
	bool ret = _image.loadFromData(data, _format);
	_colorTables.clear();
	_colorTables.append(_image.colorTable());
	return ret;
}

bool TextureImageFile::save(QByteArray &data) const
{
	QBuffer buff;

	bool ret = _image.save(&buff, _format);

	data = buff.data();

	return ret;
}
