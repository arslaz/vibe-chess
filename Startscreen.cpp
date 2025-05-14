#include "Startscreen.h"

void drawSplashScreen(RenderWindow& window, Texture& splashTexture, Font& font) {
    Sprite splash(splashTexture);
    splash.setScale(
        1920.0f / splash.getTexture()->getSize().x,
        1080.0f / splash.getTexture()->getSize().y
    );

    Text prompt;
    prompt.setFont(font);
    prompt.setString(L"Нажмите, чтобы начать");
    prompt.setCharacterSize(30);
    prompt.setFillColor(Color::White);
    prompt.setPosition(850, 800);

    window.clear();
    window.draw(splash);
    window.draw(prompt);
    window.display();
}