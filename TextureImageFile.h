#ifndef TEXTUREIMAGEFILE_H
#define TEXTUREIMAGEFILE_H

#include "TextureFile.h"

class TextureImageFile : public TextureFile
{
public:
	TextureImageFile(const char *format);
	TextureImageFile(const TextureFile &textureFile);
	bool open(const QByteArray &data);
	bool save(QByteArray &data) const;
	inline quint8 depth() const {
		return image().depth();
	}
private:
	const char *_format;
};

#endif // TEXTUREIMAGEFILE_H
