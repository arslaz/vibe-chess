#include "Button.h"

Button::Button() {
    shape.setFillColor(sf::Color(70, 150, 250));
    hoverOverlay.setFillColor(sf::Color(255, 255, 255, 50));
}

void Button::setup(const sf::Font& font, const std::wstring& label,
    sf::Vector2f position, sf::Vector2f size,
    sf::Sound* hoverSnd, sf::Sound* clickSnd) {
    shape.setSize(size);
    shape.setPosition(position);

    text.setFont(font);
    text.setString(label);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition(position.x + 20, position.y + 10);

    hoverOverlay.setSize(size);
    hoverOverlay.setPosition(position);

    hoverSound = hoverSnd;
    clickSound = clickSnd;
}

void Button::update(const sf::Vector2f& mousePos) {
    bool nowHovered = shape.getGlobalBounds().contains(mousePos);

    if (nowHovered && !isHovered && hoverSound) {
        hoverSound->play();
    }

    isHovered = nowHovered;
}

bool Button::isClicked(const sf::Vector2f& mousePos, bool mousePressed) {
    if (isHovered && mousePressed && clickSound) {
        clickSound->play();
        return true;
    }
    return false;
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    if (isHovered) {
        window.draw(hoverOverlay);
    }
    window.draw(text);
}

void Button::setPosition(sf::Vector2f position) {
    sf::Vector2f offset = position - shape.getPosition();
    shape.move(offset);
    hoverOverlay.move(offset);
    text.move(offset);
}