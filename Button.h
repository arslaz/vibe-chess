#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class Button {
private:
    sf::RectangleShape shape;
    sf::Text text;
    sf::RectangleShape hoverOverlay;
    bool isHovered = false;
    sf::Sound* hoverSound = nullptr;
    sf::Sound* clickSound = nullptr;

public:
    Button();
    void setup(const sf::Font& font, const std::string& label,
        sf::Vector2f position, sf::Vector2f size,
        sf::Sound* hoverSnd, sf::Sound* clickSnd);
    void update(const sf::Vector2f& mousePos);
    bool isClicked(const sf::Vector2f& mousePos, bool mousePressed);
    void draw(sf::RenderWindow& window);
    void setPosition(sf::Vector2f position);
}; 
