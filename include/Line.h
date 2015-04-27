#ifndef LINE_H
#define LINE_H
#include <vector>
#include <SFML/Graphics.hpp>

class Line
{
    public:
        Line();
        virtual ~Line();
        void update(sf::RenderWindow& canvas);
        void attributePath(std::vector<int> &path, float dx, float dy, float scale);
        void startTransition();
        void setPosition(int x, int y, float t);
        void free();
        void goback();
    protected:
    private:
        void addPoint(float x,float y,bool white);
        void eraseLastPoint();
        std::vector<sf::Vertex> showPoints;
        char curPath=0;
        int ptStart,ptEnd,ptLen,pathPos,pathLen,transFrm;
        int newPathLen;
        bool isfree;
        char linked;
        float vth,vr,ath;
        float px,py;
        float thickness;
        float cx,cy;
        std::vector<int> lpath[2];
};

#endif // LINE_H
