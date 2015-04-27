#include <SFML/Graphics.hpp>

#include <cmath>
#include <cstdio>
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>

#include "include/Picture.h"
#include "include/Displayer.h"

using namespace std;

int main()
{
    srand(time(NULL));

    string dir = "D:\\Prog\\expoProfondeur\\JPEG\\";

    // Ouverture de la fenêtre
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(desktop, "Profondeur", sf::Style::Default, settings);

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
