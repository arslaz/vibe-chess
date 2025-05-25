#include "NewGame.h"
#include "Button.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>

void openNewGame(sf::RenderWindow& window, float musicVolume, float soundVolume) {
    // �������� ������
    sf::Font font;
    if (!font.loadFromFile("image/arial.ttf")) return;

    // �������� ������
    sf::SoundBuffer hoverBuffer, clickBuffer;
    if (!hoverBuffer.loadFromFile("sound/hover.mp3") || !clickBuffer.loadFromFile("sound/click.wav")) return;

    sf::Sound hoverSound(hoverBuffer), clickSound(clickBuffer);
    hoverSound.setVolume(soundVolume);
    clickSound.setVolume(soundVolume);

    // �������� ������ "���� � �����"
    Button botGameBtn;
    botGameBtn.setup(font, "Game with bot",
        sf::Vector2f(window.getSize().x / 2 - 150, 300),
        sf::Vector2f(300, 60),
        &hoverSound, &clickSound);

    // ��������� ������ "�����"
    sf::Texture backTexture;
    if (!backTexture.loadFromFile("image/back.png")) return;

    sf::Sprite backButton(backTexture);
    const float backButtonSize = 50.f; // ������������� ������ 50x50
    float scale = backButtonSize / std::max(backTexture.getSize().x, backTexture.getSize().y);
    backButton.setScale(scale, scale);
    backButton.setPosition(20, 20);

    const float originalScale = scale;
    const float hoverScale = originalScale * 1.1f;
    bool backWasHovered = false; // ���� ��� ������������ ���������

    // �������� ����
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


        botGameBtn.update(mousePos);
        if (botGameBtn.isClicked(mousePos, mousePressed)) {
            // ����� ����� ������ ������� ���� � �����
        }


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

        // ���������
        window.clear(sf::Color(30, 30, 30)); // ����� ���
        window.draw(backButton);
        botGameBtn.draw(window);
        window.display();
    }
}