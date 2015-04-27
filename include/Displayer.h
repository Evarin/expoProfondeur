#ifndef DISPLAYER_H
#define DISPLAYER_H

#include <vector>
#include <SFML/Graphics.hpp>

#include "Picture.h"

class Picture;

using namespace std;

class Displayer
{
    public:
        Displayer(sf::RenderWindow &window, const string &dir);
        virtual ~Displayer();
        void update();

        static int screenX, screenY, screenCX, screenCY;

    protected:
    private:
        sf::RenderWindow &screen;
        sf::Sprite photoSprite;
        int numImage;
        int numTex;
        unsigned long int frame;
        vector<string> files;
        vector<int> imgOrder;
        vector<Picture *> pictures;
        sf::VertexArray vertices;
        void showDirections (float amplpct, float anglepct);
};

#endif // DISPLAYER_H
