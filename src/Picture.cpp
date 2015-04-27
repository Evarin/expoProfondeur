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

#define computeGradient(NO,N,NE, O,C,E, SO,S,SE) (NO - NE + 2*(O - E) + SO - SE)

using namespace std;

typedef struct aPixel {
    float dx,dy,d;
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
        i++;
    }
    fichier.close();
}

void saveDirections(const string &file, const vector<direction> &directions){
    ofstream fichier(file.c_str(), ios::out);
    for(vector<direction>::const_iterator direction = directions.begin();
          direction != directions.end() ;
          direction ++) {
        fichier << direction->angle << " " << direction->ampl << " ";
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
            newdata[tp] = (sf::Uint8)((imgdata[tpos]+imgdata[tpos+1]+imgdata[tpos+2])/3);
//            tempdata[tp].x=i;
//            tempdata[tp].y=j;
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

    vector<float> gaussienne(ANGLERES/2+1);
    float sigmas = 0.10f;
    float gausscst = 1/sqrt(2*M_PI*sigmas);
    for(int k=0; k<=ANGLERES/2; k++) {
        gaussienne[k] = gausscst * expf(-(k*k)/sigmas);
    }

    kiss_fftr_cfg cfgf = kiss_fftr_alloc(ANGLERES,0,NULL,NULL);
    kiss_fftr_cfg cfgi = kiss_fftr_alloc(ANGLERES,1,NULL,NULL);
    kiss_fft_scalar *directions = (kiss_fft_scalar *) malloc(ANGLERES * sizeof(kiss_fft_scalar));
    kiss_fft_cpx *fout = (kiss_fft_cpx *) malloc((ANGLERES/2+1) * sizeof(kiss_fft_cpx));

    for(i = 0; i < ANGLERES; i++) directions[i] = 0.0f;

    int t = 0;

    for(i=0;i<swdt;i++){
        for(j=0;j<shgt;j++){
            int tp = (i+j*swdt);
            int tp2 = (i + j * wdt) * CARREAU;

            for(int di = 0; di<CARREAU; di++) {
                for(int dj = 0; dj<CARREAU; dj++) {
                    int ttp = tp2 + di + dj*wdt;
                    pixel *px = tempdata + ttp;
                    int angle = ((int)round((atan2(px->dy, px->dx)/M_PI)*ANGLERES)+ANGLERES*2)%ANGLERES;
                    directions[angle] += px->d;
                }
            }

            kiss_fftr(cfgf, directions, fout);

            for(int k=0; k<=ANGLERES/2; k++) {
                if(t==0) std::cout << ((int)(directions[k]*100.0f)/100.0f) << " ";
                fout[k].r *= gaussienne[k];
                fout[k].i *= gaussienne[k];
            }

            kiss_fftri(cfgi, fout, directions);

            if(t==0) std::cout << "\n";
            for(int k=0; k<ANGLERES; k++)
                if(t==0) std::cout << ((int)(directions[k]/1.28f)/100.0f) << " ";
            t++;

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
