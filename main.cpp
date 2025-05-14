#include <SFML/Graphics.hpp>
#include "Startscreen.h"
#include "menu.h"

using namespace sf;

int main() {
    RenderWindow window(VideoMode(1920, 1080), "Tactics Royale");
    window.setFramerateLimit(60);

    Texture splashTex, logoTex, boardTex, settingsIcon, historyIcon;
    Font font;

    if (!splashTex.loadFromFile("image/nachzastavka.png") ||
        !logoTex.loadFromFile("image/logo2.png") ||
        !boardTex.loadFromFile("image/setings.png") ||
        !settingsIcon.loadFromFile("image/settings.png") ||
        !historyIcon.loadFromFile("image/setings.png") ||
        !font.loadFromFile("image/font1.ttf")) {
        return -1;
    }

    drawSplashScreen(window, splashTex, font);

    bool inSplash = true;
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();
            if (inSplash && (event.type == Event::MouseButtonPressed ||
                (event.type == Event::KeyPressed && event.key.code == Keyboard::Space))) {
                inSplash = false;
            }
        }

        if (inSplash) continue;

        MenuButton btn1, btn2, settings, history, registerBtn, loginBtn;

        btn1.set(font, L"Игра с ботом", { 1300, 300 }, { 350, 70 }, Color(80, 200, 100), nullptr, true);
        btn2.set(font, L"Игра с другом", { 1300, 390 }, { 350, 70 }, Color(70, 70, 70));

        settings.set(font, L"Настройки", { 20, 120 }, { 250, 50 }, Color(70, 70, 70), &settingsIcon);
        history.set(font, L"История", { 20, 180 }, { 250, 50 }, Color(70, 70, 70), &historyIcon);

        registerBtn.set(font, L"Регистрация", { 20, 700 }, { 250, 50 }, Color(80, 200, 100), nullptr, true);
        loginBtn.set(font, L"Вход", { 20, 760 }, { 250, 50 }, Color(70, 70, 70));

        drawMainMenu(window, font, logoTex, boardTex, btn1, btn2, settings, history, registerBtn, loginBtn);
    }

    return 0;
}
