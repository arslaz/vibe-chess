#include <SFML/Graphics.hpp>
#include "menu.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Tactics Royale", sf::Style::Close);
    window.setFramerateLimit(60);
    startGame(window);
    return 0;
}