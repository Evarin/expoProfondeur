#ifndef PICTURE_H
#define PICTURE_H

#include <string>
#include "Displayer.h"

#define CARREAU 12
#define ANGLERES 128

typedef struct dirStruct {
    float angle;
    float ampl;
} direction;

class Picture
{
    public:
        Picture(const std::string &fichier);
        virtual ~Picture();

        const std::vector<direction> &directions;
        const sf::Texture &getTexture();
    protected:
    private:
        sf::RenderTexture ophoto;
        std::vector<direction> directs;
};

#endif // PICTURE_H