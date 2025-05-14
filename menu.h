#pragma once
#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
using namespace sf;

struct MenuButton {
    RectangleShape shape;
    Text label;
    Sprite icon;
    bool isStaticColor = false;

    void set(const Font& font, const String& text, Vector2f position, Vector2f size, Color color, Texture* iconTexture = nullptr, bool staticColor = false);
    void draw(RenderWindow& window);
    bool isMouseOver(Vector2f mousePos);
};

void drawMainMenu(RenderWindow& window, Font& font, Texture& logoTex, Texture& boardTex,
    MenuButton& btn1, MenuButton& btn2, MenuButton& settings, MenuButton& history,
    MenuButton& registerBtn, MenuButton& loginBtn);

#endif
