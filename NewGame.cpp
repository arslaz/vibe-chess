#include "NewGame.h"
#include "Button.h"
#include "chess_game.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>

void openNewGame(sf::RenderWindow& window, float musicVolume, float soundVolume) {
    // Загрузка шрифта
    sf::Font font;
    if (!font.loadFromFile("image/arial.ttf")) return;

    // Загрузка звуков
    sf::SoundBuffer hoverBuffer, clickBuffer;
    if (!hoverBuffer.loadFromFile("sound/hover.mp3") || !clickBuffer.loadFromFile("sound/click.wav")) return;

    sf::Sound hoverSound(hoverBuffer), clickSound(clickBuffer);
    hoverSound.setVolume(soundVolume);
    clickSound.setVolume(soundVolume);

    // Создание трех кнопок "Игра с ботом"
    const int buttonCount = 3;
    Button botGameBtns[buttonCount];
    const float buttonWidth = 300.f;
    const float buttonHeight = 60.f;
    const float startY = 250.f;
    const float spacing = 80.f; // Расстояние между кнопками

    for (int i = 0; i < buttonCount; i++) {
        botGameBtns[i].setup(font, "Game with bot " + std::to_string(i + 1),
            sf::Vector2f(window.getSize().x / 2 - buttonWidth / 2, startY + i * (buttonHeight + spacing)),
            sf::Vector2f(buttonWidth, buttonHeight),
            &hoverSound, &clickSound);
    }

    // Настройка кнопки "Назад"
    sf::Texture backTexture;
    if (!backTexture.loadFromFile("image/back.png")) return;

    sf::Sprite backButton(backTexture);
    const float backButtonSize = 50.f;
    float scale = backButtonSize / std::max(backTexture.getSize().x, backTexture.getSize().y);
    backButton.setScale(scale, scale);
    backButton.setPosition(20, 20);

    const float originalScale = scale;
    const float hoverScale = originalScale * 1.1f;
    bool backWasHovered = false;

    // Основной цикл
    while (window.isOpen()) {
        sf::Event event;
        bool mousePressed = false;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                mousePressed = true;
            }
        }

        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // Обновление кнопок "Игра с ботом"
        for (int i = 0; i < buttonCount; i++) {
            botGameBtns[i].update(mousePos);
            if (botGameBtns[i].isClicked(mousePos, mousePressed)) {
                clickSound.play();

                sf::SoundBuffer moveBuffer;
                static sf::Sound moveSound(moveBuffer);

                ChessGameSettings settings;
                settings.soundVolume = soundVolume;
                settings.musicVolume = musicVolume;
                settings.engineDepth = 10;
                settings.moveSound = &moveSound;
                int levell;
                if (i == 1) {
                    levell = 3;
                }
                else if (i == 2) {
                    levell = 10;
                }
                else {
                    levell = 20;
                }

                int level = i + 1; // Уровень сложности в зависимости от номера кнопки (1, 2, 3)
                runChessGame(window, settings, levell);
                window.setTitle("Vibe Chess");
            }
        }

        // Обновление кнопки "Назад"
        bool backHovered = backButton.getGlobalBounds().contains(mousePos);
        if (backHovered) {
            backButton.setScale(hoverScale, hoverScale);
            if (!backWasHovered) {
                hoverSound.play();
            }
            if (mousePressed) {
                clickSound.play();
                return;
            }
        }
        else {
            backButton.setScale(originalScale, originalScale);
        }
        backWasHovered = backHovered;

        // Отрисовка
        window.clear(sf::Color(30, 30, 30));

        // Рисуем все кнопки
        for (int i = 0; i < buttonCount; i++) {
            botGameBtns[i].draw(window);
        }

        window.draw(backButton);
        window.display();
    }
}