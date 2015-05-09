#include "../include/Displayer.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#include "../include/Picture.h"
#include "../include/logging.h"

#define min(a,b) (a>b?b:a)
#define max(a,b) (a<b?b:a)
#define abs(a) (a>0?a:0)
#define TFREE 80
#define TSHOW 250
#define TTRANS 100.0f
#define TTOT 650

using namespace std;

// Calcule une injection de [|0,nb|) dans [|0,mx|)

vector<int> randInj (int mx, int nb) {
    cout << nb << "->" << mx << "\n";
    vector<int> mem(nb);
    if (mx<nb)
        for (int i = 0; i < nb; i++)
            if (i < mx) mem[i] = i;
            else mem[i] = (i-mx) % min(5, mx);
    else {
        for (int i = 0; i < nb; i++)
            mem[i] = rand()%(mx-i);
        for (int i = nb-1; i >= 0; i--) {
            int n = mem[i];
            for (int j = i-1; j >= 0; j--) {
                if (n >= mem[j]) n++;
            }
            mem[i] = n;
        }
    }
    for(int i = 0; i < nb; i++) cout << mem[i] << ",";
    return mem;
}

void HSV2RVB(int h, int s, int v, vector<sf::Uint8> &ret){
    int t = (h/60) % 6;
    int f = h % 60;
    ret[(t/2 + 2)%3] = v * (100 - s) / 100;
    ret[((t + 1)/2)%3] = v;
    ret[(7-t)%3] = v * (6000 - (t&1 ? f : 60 - f) * s) / 6000;
}

Displayer::Displayer(sf::RenderWindow &window, const string &dir) :
    pictures(), screen(window)
{
    frame = TSHOW;
    numImage = 0;
    numTex = 0;

    vertices.resize(screenCX * screenCY * 2);
    for(int i = 0; i < screenCX * screenCY * 2; i++) vertices[i].color = sf::Color::White;;

    int numFiles = 0;
    DIR *repertoire = opendir(dir.c_str());
    if(!repertoire)
        return;
    struct dirent *ent;
    string file;
    while ( (ent = readdir(repertoire)) != NULL)
    {
         char *d = ent->d_name;
         char *p = strrchr(d, '.');
         if(*d != '.' && p && *(p+1) !='d'){
            *p = '\0';
            file.assign(d);
            files.push_back(dir + file);
            numFiles++;
         }
    }
    ostringstream infor;
    infor << files.size() << " fichiers trouvés";
    logInfo(infor.str());
    closedir(repertoire);
    imgOrder = randInj(numFiles, numFiles);

    pictures.push_back(new Picture(files[imgOrder[0]]));

    photoSprite.setTexture(pictures[numTex]->getTexture());
    photoSprite.setTextureRect(sf::IntRect(0, 0, Displayer::screenX, Displayer::screenY));
    photoSprite.setScale(1, 1);
}

void Displayer::showDirections (const float amplpct, const float anglepct, const float colpct) {
    const vector<direction> &odir = pictures[numTex]->directions;
    const vector<direction> &ddir = pictures[!numTex]->directions;
    int n = 0;
    sf::Vector2f temp, du;
    const float amplnpct = (1.0f - amplpct);
    const float anglenpct = (1.0f - anglepct);
    const float colnpct = (1.0f - colpct);
    vector<sf::Uint8> colors(3);
    for(int i = 0; i < Displayer::screenCX; i++) {
        for(int j = 0; j < Displayer::screenCY; j++) {
            int tp = (i+j*screenCX);

            float ampl = amplnpct * odir[tp].ampl + amplpct * ddir[tp].ampl;
            if (ampl < 1.0f) continue;
            float angle = anglenpct * odir[tp].angle + anglepct * ddir[tp].angle;

            int hue = colnpct * odir[tp].hue + colpct * ddir[tp].hue;
            int sat = (colnpct * odir[tp].sat + colpct * ddir[tp].sat) * 1.0f;
            HSV2RVB(hue, (sqrt(sat)*10.0f), 200, colors);
            sf::Color color(colors[0], colors[1], colors[2]);

            temp = sf::Vector2f((i + 0.5f) * CARREAU, (j + 0.5f) * CARREAU);
            du = sf::Vector2f(- CARREAU * sinf(angle), CARREAU * cosf(angle)) * powf(ampl, 0.5) / 15.0f / 1.414f;
            vertices[n << 1].position = temp + du;
            vertices[(n << 1)|1].position = temp - du;
            vertices[n << 1].color = vertices[(n << 1)|1].color = color;
            n++;
        }
    }
    screen.draw(&vertices[0], n*2, sf::Lines);
}

void Displayer::showDirectionsDest () {
    const vector<direction> &ddir = pictures[!numTex]->directions;
    int n = 0;
    sf::Vector2f temp, du;
    vector<sf::Uint8> colors(3);
    for(int i = 0; i < Displayer::screenCX; i++) {
        for(int j = 0; j < Displayer::screenCY; j++) {
            int tp = (i+j*screenCX);

            float ampl = ddir[tp].ampl;
            if (ampl < 1.0f) continue;
            float angle = ddir[tp].angle;

            int hue = ddir[tp].hue;
            int sat = ddir[tp].sat * 1.0f;
            HSV2RVB(hue, (sqrt(sat)*10.0f), 200, colors);
            sf::Color color(colors[0], colors[1], colors[2]);

            temp = sf::Vector2f((i + 0.5f) * CARREAU, (j + 0.5f) * CARREAU);
            du = sf::Vector2f(- CARREAU * sinf(angle), CARREAU * cosf(angle)) * powf(ampl, 0.5) / 15.0f / 1.414f;
            vertices[n << 1].position = temp + du;
            vertices[(n << 1)|1].position = temp - du;
            vertices[n << 1].color = vertices[(n << 1)|1].color = color;
            n++;
        }
    }
    screen.draw(&vertices[0], n*2, sf::Lines);
}

void Displayer::update() {
    float progress = max(0.0f, min(1.0f, (float)(frame - TFREE)/(float)(TTRANS)));
    float aprog = .5 - pow(cos(min(1.0f, progress * 1.5f) * M_PI), 1.0f) * .5;
    float bprog = .5 - pow(cos(max(0.0f, progress * 1.5f - 0.5f)*M_PI), 1.0f) * .5;
    int alpha0 = min(1.0f, (frame - TSHOW)/150.0f)*255;
    if (pictures.size() == 2 && alpha0 <= 255) {
        if (frame < TFREE || progress >= 1.0f) showDirectionsDest();
        else showDirections(aprog, bprog, progress);
    }

    // Affichage de la photo
    if (frame >= TSHOW) {
        int alpha = min(1.0f, (frame - TSHOW)/150.0f)*255;
        // cout << alpha << "\n";
        photoSprite.setColor(sf::Color(255, 255, 255, alpha));
        screen.draw(photoSprite);
    }

    if (frame <= TFREE) {
        int alpha = min(1, (TFREE - frame)/80.)*255;
        // cout << alpha << "\n";
        photoSprite.setColor(sf::Color(255, 255, 255, alpha));
        screen.draw(photoSprite);
    }

    // Timeline
    frame++;
    if (frame == TFREE) {
        photoSprite.setTexture(pictures[numTex]->getTexture());
        photoSprite.setTextureRect(sf::IntRect(0, 0, Displayer::screenX, Displayer::screenY));
        photoSprite.setScale(1, 1);
        numTex = !numTex;
    }

    if (frame == TTOT) {
        numImage++;
        numImage %= files.size();
        if (numImage == 0)
            imgOrder = randInj(files.size(), files.size());
        if (pictures.size() < 2) {
            pictures.push_back(new Picture(files[imgOrder[numImage]]));
            numTex = !numTex;
        } else {
            delete pictures[numTex];
            pictures[numTex] = new Picture(files[imgOrder[numImage]]);
        }
        while (!pictures[numTex]->available) {
            logError("Problem with " +files[imgOrder[numImage]]);
            numImage++;
            numImage %= files.size();
            delete pictures[numTex];
            pictures[numTex] = new Picture(files[imgOrder[numImage]]);
        }
        pictures[!numTex]->prepareTransitionTo(*(pictures[numTex]));
        frame = 0;
    }
}

Displayer::~Displayer()
{
    //dtor
}


int Displayer::screenX = 800;
int Displayer::screenY = 600;
int Displayer::screenCX = 80;
int Displayer::screenCY = 60;
