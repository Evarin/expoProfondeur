#include <SFML/Graphics.hpp>
#define KISS_FFT_USE_ALLOCA

#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>

#include "include/Picture.h"
#include "include/Displayer.h"

using namespace std;

int main(int nargs, char **args)
{
    srand(time(NULL));
    Picture::toLaunch = string(args[0]);

    string dir = "D:\\Prog\\expoProfondeur\\JPEG\\";

    std::cout << "\nStarting new process\n";
    for(int i=0; i< nargs; i++) {
        std::cout << args[i] << " ";
    }

    // Ouverture de la fenêtre
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    Displayer::screenX = desktop.getDesktopMode().width;
    Displayer::screenY = desktop.getDesktopMode().height;
    Displayer::screenCX = Displayer::screenX / CARREAU;
    Displayer::screenCY = Displayer::screenX / CARREAU;

    if (nargs > 1) {
        Picture computer(string(args[1]), true);
        return 0;
    }

    sf::RenderWindow window(desktop, "Profondeur", sf::Style::Fullscreen, settings);
    window.setMouseCursorVisible(false);

    sf::Time towait;
    sf::Clock clock;

    Displayer controller(window, dir);

    while (window.isOpen()){
        clock.restart();
        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed || event.type == sf::Event::MouseButtonReleased)
                window.close();
        }

        window.clear();

        controller.update();

        window.display();
        sf::sleep(sf::milliseconds(20) - clock.getElapsedTime());
    }

    return 0;
}
