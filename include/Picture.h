#ifndef PICTURE_H
#define PICTURE_H

#include <string>
#include "Displayer.h"
#include "kiss_fft.h"
#include "kiss_fftr.h"

#define CARREAU 12
#define ANGLERES 128

typedef struct dirStruct {
    float angle;
    float ampl;
    float sat;
    float hue;
} direction;

class Picture
{
    public:
        Picture(const std::string &fichier, bool compute = false);
        virtual ~Picture();

        const std::vector<direction> &directions;
        const sf::Texture &getTexture();
        void prepareTransitionTo(const Picture &destination);
        bool available;

        static std::string toLaunch;

    protected:
    private:
        sf::RenderTexture ophoto;
        std::vector<direction> directs;
};

#endif // PICTURE_H
