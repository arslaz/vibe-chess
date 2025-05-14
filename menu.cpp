#include "menu.h"

void MenuButton::set(const Font& font, const String& text, Vector2f position, Vector2f size, Color color, Texture* iconTexture, bool staticColor) {
    shape.setSize(size);
    shape.setFillColor(color);
    shape.setPosition(position);
    shape.setOutlineThickness(0);

    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(20);
    label.setFillColor(Color::White);
    label.setPosition(position.x + 60, position.y + 10);

    isStaticColor = staticColor;

    if (iconTexture) {
        icon.setTexture(*iconTexture);
        icon.setPosition(position.x + 15, position.y + 5);
        icon.setScale(0.045f, 0.045f); // Увеличенный масштаб иконки
    }
}

void MenuButton::draw(RenderWindow& window) {
    window.draw(shape);
    if (icon.getTexture()) window.draw(icon);
    window.draw(label);
}

bool MenuButton::isMouseOver(Vector2f mousePos) {
    return shape.getGlobalBounds().contains(mousePos);
}

void drawMainMenu(RenderWindow& window, Font& font, Texture& logoTex, Texture& boardTex,
    MenuButton& btn1, MenuButton& btn2, MenuButton& settings, MenuButton& history,
    MenuButton& registerBtn, MenuButton& loginBtn) {
    // для меню выделение
    RectangleShape sidebar(Vector2f(300, 1080));
    sidebar.setFillColor(Color(20, 20, 20));
    // общий фон
    RectangleShape separator(Vector2f(2, 1080));
    separator.setPosition(300, 0);
    separator.setFillColor(Color(50, 50, 50));
    // лого
    Sprite logo(logoTex);
    logo.setPosition(10, 5);
    logo.setScale(0.2f, 0.2f);

    Sprite board(boardTex);
    board.setPosition(480, 100);
    board.setScale(0.9f, 0.9f);

    Vector2f mousePos = (Vector2f)Mouse::getPosition(window);
    MenuButton* buttons[] = { &btn1, &btn2, &settings, &history, &registerBtn, &loginBtn };

    for (auto btn : buttons) {
        if (!btn->isStaticColor && btn->isMouseOver(mousePos)) {
            btn->shape.setFillColor(Color(btn->shape.getFillColor()) + Color(40, 40, 40));
        }
        else if (!btn->isStaticColor) {
            btn->shape.setFillColor(Color(20, 20, 20));
        }
    }

    window.clear(Color(30, 30, 30));
    window.draw(sidebar);
    window.draw(separator);
    window.draw(logo);
    for (auto btn : buttons) btn->draw(window);
    window.draw(board);
    window.display();
}