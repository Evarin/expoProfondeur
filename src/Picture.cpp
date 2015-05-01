#include "../include/Picture.h"

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>

#include "../include/kiss_fftr.h"
#include "../include/Displayer.h"

#define gethue(r, g, b, mx, mn) (mx == mn ? 0 : (mx == r ? (60*(g-b)/(mx-mn) + 360)%360 : (mx==g ? (60*(b-r)/(mx-mn) + 120) : (60*(r-g)/(mx-mn) + 240) ) ) )
#define getsat(mx, mn) (mx == 0 ? 0 : 100 - (100*mn)/mx)
#define getval(mx) mx

#define computeGradient(NO, N, NE,  O, C, E,  SO, S, SE) (NO - NE + 2*(O - E) + SO - SE)

using namespace std;

typedef struct aPixel {
    float dx, dy, d, h, s;
} pixel;

bool fexists(const std::string& filename) {
  ifstream ifile(filename.c_str());
  return ifile;
}


// Chargemement des coordonnées des lignes
void loadDirections(const string &file, vector<direction> &directions){
    ifstream fichier(file.c_str(), ios::in);
    float val;
    unsigned int i = 0;

    while(fichier >> val && i < directions.size()){
        directions[i].angle = val;
        fichier >> directions[i].ampl;
        fichier >> directions[i].hue;
        fichier >> directions[i].sat;
        i++;
    }
    fichier.close();
}

void saveDirections(const string &file, const vector<direction> &directions){
    ofstream fichier(file.c_str(), ios::out);
    for(vector<direction>::const_iterator direction = directions.begin();
          direction != directions.end() ;
          direction ++) {
        fichier << direction->angle << " " << direction->ampl << " " << direction->hue << " " << direction->sat << " ";
    }
    fichier.close();
}

void computeDirections (const sf::Image &img, vector<direction> &dirmap){
    const sf::Uint8 *imgdata = img.getPixelsPtr();
    sf::Vector2u sz = img.getSize();
    int wdt = sz.x;
    int hgt = sz.y;
    sf::Uint8 *newdata = (sf::Uint8 *) malloc(sizeof(sf::Uint8)*wdt*hgt);
    pixel *tempdata = (pixel *) malloc(sizeof(pixel)*wdt*hgt);
    int i,j;

    // Initialisation
    for(i=0;i<wdt;i++){
        for(j=0;j<hgt;j++){
            int tpos = (i+j*wdt)*4;
            int tp = i+j*wdt;
            int r = imgdata[tpos];
            int g = imgdata[tpos+1];
            int b = imgdata[tpos+2];
            int mx = max(r, max(g, b));
            int mn = min(r, min(g, b));
            newdata[tp] = (sf::Uint8)((r + g + b)/3);
            tempdata[tp].h = gethue(r, g, b, mx, mn);
            tempdata[tp].s = getsat(mx, mn);
        }
    }
    int szl = wdt;

    // Calcul des gradients //


    // Centre
    for(i=1;i<wdt-1;i++){
        for(j=1;j<hgt-1;j++){
            int tpos = (i+j*wdt);
            int tp = (i+j*wdt);
            int dx = computeGradient(newdata[tpos-szl-1],newdata[tpos-szl],newdata[tpos-szl+1],
                                     newdata[tpos-1],    newdata[tpos],    newdata[tpos+1],
                                     newdata[tpos+szl-1],newdata[tpos+szl],newdata[tpos+szl+1]);
            int dy = computeGradient(newdata[tpos-szl-1],newdata[tpos-1],newdata[tpos+szl-1],
                                     newdata[tpos-szl],    newdata[tpos],newdata[tpos+szl],
                                     newdata[tpos-szl+1],newdata[tpos+1],newdata[tpos+szl+1]);
            float d = sqrt(dx*dx + dy*dy);
            tempdata[tp].dx = dx/d;
            tempdata[tp].dy = dy/d;
            tempdata[tp].d = d;
        }
    }
    // Colonne de bord N
    for(j=1;j<hgt-1;j++){
        i=0;
        int tpos = (i+j*wdt);
        int tp = (i+j*wdt);
        int dx = computeGradient(newdata[tpos-szl],newdata[tpos-szl],newdata[tpos-szl+1],
                                 newdata[tpos],    newdata[tpos],    newdata[tpos+1],
                                 newdata[tpos+szl],newdata[tpos+szl],newdata[tpos+szl+1]);
        int dy = computeGradient(newdata[tpos-szl],newdata[tpos],newdata[tpos+szl],
                                 newdata[tpos-szl],    newdata[tpos],newdata[tpos+szl],
                                 newdata[tpos-szl+1],newdata[tpos+1],newdata[tpos+szl+1]);
        float d = sqrt(dx*dx + dy*dy);
        tempdata[tp].dx = dx/d;
        tempdata[tp].dy = dy/d;
        tempdata[tp].d = d;
        i = wdt-1;
        tpos = (i+j*wdt);
        tp = (i+j*wdt);
        dx = computeGradient(newdata[tpos-szl-1],newdata[tpos-szl],newdata[tpos-szl],
                             newdata[tpos-1],    newdata[tpos],    newdata[tpos],
                             newdata[tpos+szl-1],newdata[tpos+szl],newdata[tpos+szl]);
        dy = computeGradient(newdata[tpos-szl-1],newdata[tpos-1],newdata[tpos+szl-1],
                             newdata[tpos-szl],    newdata[tpos],newdata[tpos+szl],
                             newdata[tpos-szl],newdata[tpos],newdata[tpos+szl]);
        d = sqrt(dx*dx + dy*dy);
        tempdata[tp].dx = dx/d;
        tempdata[tp].dy = dy/d;
        tempdata[tp].d = d;
    }

    // Lignes de bord
    for(i=2;i<wdt-1;i++){
        j=0;
        int tpos = (i+j*wdt);
        int tp = (i+j*wdt);
        int dx = computeGradient(newdata[tpos-1],newdata[tpos],newdata[tpos+1],
                                 newdata[tpos-1],    newdata[tpos],    newdata[tpos+1],
                                 newdata[tpos+szl-1],newdata[tpos+szl],newdata[tpos+szl+1]);
        int dy = computeGradient(newdata[tpos-1],newdata[tpos-1],newdata[tpos+szl-1],
                                 newdata[tpos],    newdata[tpos],newdata[tpos+szl],
                                 newdata[tpos+1],newdata[tpos+1],newdata[tpos+szl+1]);
        float d = sqrt(dx*dx + dy*dy);
        tempdata[tp].dx = dx/d;
        tempdata[tp].dy = dy/d;
        tempdata[tp].d = d;
        j = hgt-1;
        tpos = (i+j*wdt);
        tp = (i+j*wdt);
        dx = computeGradient(newdata[tpos-szl-1],newdata[tpos-szl],newdata[tpos-szl+1],
                             newdata[tpos-1],    newdata[tpos],    newdata[tpos+1],
                             newdata[tpos-1],newdata[tpos],newdata[tpos+1]);
        dy = computeGradient(newdata[tpos-szl-1],newdata[tpos-1],newdata[tpos-1],
                             newdata[tpos-szl],    newdata[tpos],newdata[tpos],
                             newdata[tpos-szl+1],newdata[tpos+1],newdata[tpos+1]);
        d = sqrt(dx*dx + dy*dy);
        tempdata[tp].dx = dx/d;
        tempdata[tp].dy = dy/d;
        tempdata[tp].d = d;
    }

    free(newdata);

    int swdt = sz.x/CARREAU;
    int shgt = sz.y/CARREAU;

    dirmap.resize(swdt * shgt);

    // Histogramme des directions
    kiss_fft_scalar *directions = (kiss_fft_scalar *) malloc(ANGLERES * sizeof(kiss_fft_scalar));
    kiss_fft_scalar *hues = (kiss_fft_scalar *) malloc(ANGLERES * sizeof(kiss_fft_scalar));

    // Préparation des convolutions

    vector<float> gaussienne(ANGLERES/2+1);
    float sigmas = 0.10f;
    float gausscst = 1/sqrt(2*M_PI*sigmas);
    for(int k=0; k<=ANGLERES/2; k++) {
        gaussienne[k] = gausscst * expf(-(k*k)/sigmas);
    }

    kiss_fftr_cfg cfgf = kiss_fftr_alloc(ANGLERES, 0, NULL, NULL);
    kiss_fftr_cfg cfgi = kiss_fftr_alloc(ANGLERES, 1, NULL, NULL);
    kiss_fft_cpx *fout = (kiss_fft_cpx *) malloc((ANGLERES/2+1) * sizeof(kiss_fft_cpx));

    int t = 0;

    for(int k = 0; k < ANGLERES; k++) directions[k] = hues[k] = 0.0f;

    for(i = 0; i < swdt; i++){
        for(j = 0; j < shgt; j++){
            int tp = (i + j * swdt);
            int tp2 = (i + j * wdt) * CARREAU;
            float sat = 0.0f;
            float sattot = 0.0f;

            for(int di = 0; di < CARREAU; di++) {
                for(int dj = 0; dj < CARREAU; dj++) {
                    int ttp = tp2 + di + dj*wdt;
                    pixel *px = tempdata + ttp;

                    int angle = ((int)round((atan2(px->dy, px->dx) / M_PI) * ANGLERES ) + ANGLERES * 2) % ANGLERES;
                    directions[angle] += px->d;

                    int hue = (int)round(px->h / 360.0f * ANGLERES);
                    hues[hue] += 1.0f;

                    sat += px->s;
                    sattot += 1.0f;
                }
            }

            // Directions
            kiss_fftr(cfgf, directions, fout);

            for(int k=0; k<=ANGLERES/2; k++) {
                fout[k].r *= gaussienne[k];
                fout[k].i *= gaussienne[k];
            }

            kiss_fftri(cfgi, fout, directions);

            int mdir = 0;
            float mamp = 0.0f;
            for(int k = 0; k<ANGLERES; k++){
                if(directions[k] > mamp){
                    mdir = k;
                    mamp = directions[k];
                }
                directions[k] = 0.0f;
            }
            dirmap[tp].angle = (float) mdir * M_PI / ANGLERES;
            dirmap[tp].ampl = mamp / 128.0f;

            // Hue
            kiss_fftr(cfgf, hues, fout);

            for(int k=0; k<=ANGLERES/2; k++) {
                fout[k].r *= gaussienne[k];
                fout[k].i *= gaussienne[k];
            }

            kiss_fftri(cfgi, fout, hues);

            int mhue = 0;
            mamp = 0.0f;
            for(int k = 0; k < ANGLERES; k++){
                if(hues[k] > mamp){
                    mhue = k;
                    mamp = hues[k];
                }
                hues[k] = 0.0f;
            }
            dirmap[tp].hue = (float) mhue * 360.0f / ANGLERES;
            dirmap[tp].sat = sat / sattot;
        }
    }
    free(cfgf);
    free(cfgi);

    free(fout);
    free(directions);

    free(tempdata);
}


Picture::Picture(const string &fichier) :
    directions(directs) {
    sf::Texture tphoto;
    if(! tphoto.loadFromFile(fichier + ".jpg"))
        return;
    sf::Vector2u tps = tphoto.getSize();
    tphoto.setSmooth(true);

    ophoto.create(Displayer::screenX, Displayer::screenY);

    float pscale = min((float) Displayer::screenX / tps.x, (float) Displayer::screenY / tps.y);
    float ptx = (Displayer::screenX - tps.x*pscale)/2;
    float pty = (Displayer::screenY - tps.y*pscale)/2;

    sf::Sprite photoSprite;
    photoSprite.setTexture(tphoto);
    photoSprite.setTextureRect(sf::IntRect(0, 0, tps.x, tps.y));
    photoSprite.setScale(pscale, -pscale);
    photoSprite.setPosition(ptx, Displayer::screenY - pty);

    ophoto.draw(photoSprite);

    directs.resize(Displayer::screenCX * Displayer::screenCY);

    char *truc = (char *)malloc(100 * sizeof(char));
    sprintf(truc, "%s_%dx%d.data", fichier.c_str(), Displayer::screenCX, Displayer::screenCY);
    string datafile(truc);
    if (! fexists(datafile)) {
        sf::Image img = getTexture().copyToImage();
        //img.flipVertically();
        computeDirections(img, directs);
        saveDirections(datafile, directs);
    } else {
        loadDirections(datafile, directs);
    }
    free(truc);
}

const sf::Texture &Picture::getTexture () {
    return ophoto.getTexture();
}

Picture::~Picture()
{
    //dtor
}
