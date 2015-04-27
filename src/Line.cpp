#include "../include/Line.h"
#define BUFLENGTH 50
#include <cmath>
#include <iostream>
#include <time.h>
#define min(a,b) (a>b?b:a)
#define TLEN 150
#define distc(a,b,c,d) ((a-c)*(a-c)+(b-d)*(b-d))

// Positionne la tête de ligne
void Line::setPosition(int x, int y, float t){
    cx=x;
    cy=y;
    vth=t;
}

// Ajoute un point à la ligne
// La ligne est considérée comme un tableau cyclique
// La longueur maximale de la ligne est BUFLENGTH
void Line::addPoint(float x,float y,bool white){
    ptLen=min(ptLen+1,BUFLENGTH);
    sf::Vector2f tmp(x,y);
    sf::Vector2f perp(y-py,px-x);
    float n=1/sqrt(perp.x*perp.x+perp.y*perp.y)*thickness;
    perp.x*=n; perp.y*=n;
    showPoints[ptEnd*2]=sf::Vertex(tmp-perp,white?sf::Color::White : sf::Color(150,150,150));
    showPoints[ptEnd*2+1]=sf::Vertex(tmp+perp,white?sf::Color::White : sf::Color(150,150,150));
    if(ptEnd==0){
        showPoints[BUFLENGTH*2]=showPoints[0];
        showPoints[BUFLENGTH*2+1]=showPoints[1];
    }

    ptEnd++;
    ptEnd%=BUFLENGTH;
    if(ptLen==BUFLENGTH)
        ptStart=(ptEnd)%BUFLENGTH;
    px=x; py=y;
}

// Enlève le plus vieux point de la ligne
void Line::eraseLastPoint(){
    if(ptLen>0){
        ptStart++;
        ptStart%=BUFLENGTH;
        ptLen--;
    }
}

// Libère la ligne de son chemin
void Line::free(){
    ath=0;
    isfree=true;
    linked=0;
}

void Line::goback(){
    isfree=false;
    pathPos=pathLen-2;
    linked=-1;
}

// Met à jour la ligne (et la fait avancer)
void Line::update(sf::RenderWindow &canvas){
    if(!linked){
        cx+=vr*cos(vth);
        cy+=vr*sin(vth);
        vth+=ath;
        ath+=((rand()%300)/300.-.5-ath)*.1;
    }
    if(isfree){
        addPoint(cx,cy,false);
    }else if(linked){
        if(pathPos<pathLen-1){
            float px=lpath[curPath][pathPos*2],py=lpath[curPath][pathPos*2+1];
            addPoint(px,py,true);

            cx=px;cy=py;
            const char dn=16;
            float tpx=lpath[curPath][pathPos*2-dn],tpy=lpath[curPath][pathPos*2-dn+1];
            vth=atan2(py-tpy,px-tpx);
            if(linked==-1) vth+=3.14159;

            pathPos+=linked;
            if(pathPos<TLEN) free();
        }else{
            eraseLastPoint();
        }
    }else{
        float px,py,c=((float)transFrm)/TLEN;
        c=.5-cos(c*3.14159)*.5;
        px=cx*(1-c)+lpath[curPath][transFrm*2]*c;
        py=cy*(1-c)+lpath[curPath][transFrm*2+1]*c;
        addPoint(px,py,false);
        transFrm++;
        if(transFrm>=TLEN)
            linked=1;
    }

    // Affichage
    if(ptLen>1){
        canvas.draw(&showPoints[ptStart*2],min(ptLen*2-1,BUFLENGTH*2+2-ptStart*2),sf::TrianglesStrip);
        if(ptStart>=ptEnd&&ptEnd>0) canvas.draw(&showPoints[0],ptEnd*2-1,sf::TrianglesStrip);
    }
}


void Line::attributePath(std::vector<int> &path, float dx, float dy, float scale){
    float tpx,tpy,px,py;
    int i=2,n=TLEN;
    const int TTL=2*TLEN;
    int startP,sens;
    thickness=(rand()%3+1)/2.;
    if(distc(cx,cy,path[0],path[1])
       < distc(cx,cy,path[path.size()-2],path[path.size()-1])){
        startP=0;sens=1;
    }else{
        startP=path.size()-2;
        sens=-1;
    }
    curPath=!curPath;

    lpath[curPath].resize(path.size()+TLEN*2);

    tpx=px=lpath[curPath][n*2]=dx+path[startP]*scale;
    tpy=py=lpath[curPath][n*2+1]=dy+path[startP+1]*scale;

    n++;
    while(i<path.size()){
        while((px-tpx)*(px-tpx)+(py-tpy)*(py-tpy)<20 && i<path.size()){
            i+=2;
            px=dx+scale*path[startP+i*sens];
            py=dy+scale*path[startP+i*sens+1];
        }
        lpath[curPath][n*2]=tpx=px;
        lpath[curPath][n*2+1]=tpy=py;
        n++;
    }

    const char dn=20;
    float th1=atan2f(lpath[curPath][TTL+1]-lpath[curPath][TTL+dn+1],lpath[curPath][TTL]-lpath[curPath][TTL+dn]);
    float th2=atan2f(lpath[curPath][TTL+dn+1]-lpath[curPath][TTL+dn*2+1],lpath[curPath][TTL+dn]-lpath[curPath][TTL+dn*2]);
    float dth=(th1-th2)/dn;

    px=lpath[curPath][TTL];
    py=lpath[curPath][TTL+1];

    float th=th1;
    for(int p=TLEN-1;p>=0;p--){
        px+=4*cos(th);
        py+=4*sin(th);
        th+=dth;
        lpath[curPath][p*2]=px;
        lpath[curPath][p*2+1]=py;
    }

    newPathLen=n;
    curPath=!curPath;
}

void Line::startTransition(){
    isfree=false;
    curPath=!curPath;
    pathLen=newPathLen;
    linked=0;
    transFrm=0;
    pathPos=TLEN;
}

Line::Line()
{
    thickness=.5;
    isfree=true;
    linked=0;
    ptLen=0;
    ptStart=ptEnd=0;
    vr=6;
    vth=rand()%3;
    cx=cy=300;
    ath=0;
    showPoints.resize(BUFLENGTH*2+4);
}

Line::~Line()
{
    //dtor
}
