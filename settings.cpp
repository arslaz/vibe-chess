#include "settings.h"
#include <fstream>
#include <SFML/Graphics.hpp>

void openSettings(sf::RenderWindow& window, sf::Music& music, sf::Sound& hoverSound, sf::Sound& clickSound) {

    sf::Font font;
    if (!font.loadFromFile("image/arial.ttf")) return;


    const float buttonSize = 80.f;
    const float buttonSpacing = 120.f;
    const float centerX = window.getSize().x / 2 - buttonSpacing;
    const float textOffset = 250.f; 
    const float musicY = 300.f;
    const float soundY = 450.f;


    sf::Text musicText(L"Ã”«€ ¿", font, 50);
    musicText.setPosition(centerX - textOffset, musicY - 10.f); 
    musicText.setFillColor(sf::Color::White);

    sf::Text soundText(L"«¬” »", font, 50);
    soundText.setPosition(centerX - textOffset, soundY - 10.f);
    soundText.setFillColor(sf::Color::White);


    sf::RectangleShape musicButtons[3];
    sf::RectangleShape soundButtons[3];


    for (int i = 0; i < 3; ++i) {
        musicButtons[i].setSize({ buttonSize, buttonSize });
        musicButtons[i].setPosition(centerX + i * buttonSpacing, musicY);
        musicButtons[i].setFillColor(i == 0 ? sf::Color::Green : sf::Color(100, 100, 100));
        musicButtons[i].setOutlineThickness(2);
        musicButtons[i].setOutlineColor(sf::Color::White);

        soundButtons[i].setSize({ buttonSize, buttonSize });
        soundButtons[i].setPosition(centerX + i * buttonSpacing, soundY);
        soundButtons[i].setFillColor(i == 0 ? sf::Color::Green : sf::Color(100, 100, 100));
        soundButtons[i].setOutlineThickness(2);
        soundButtons[i].setOutlineColor(sf::Color::White);
    }


    sf::Texture backTexture;
    if (!backTexture.loadFromFile("image/back.png")) return;
    sf::Sprite backButton(backTexture);
    backButton.setPosition(30.f, 30.f);
    const float backScale = 60.f / std::max(backTexture.getSize().x, backTexture.getSize().y);
    backButton.setScale(backScale, backScale);


    bool musicHoverStates[3] = { false };
    bool soundHoverStates[3] = { false };
    bool backHovered = false;


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

   
        bool currentBackHover = backButton.getGlobalBounds().contains(mousePos);
        if (currentBackHover && !backHovered) hoverSound.play();
        backHovered = currentBackHover;

        if (backHovered) {
            backButton.setScale(backScale * 1.1f, backScale * 1.1f);
            if (mousePressed) {
                clickSound.play();
                return;
            }
        }
        else {
            backButton.setScale(backScale, backScale);
        }

  
        for (int i = 0; i < 3; ++i) {
   
            bool musicHover = musicButtons[i].getGlobalBounds().contains(mousePos);
            if (musicHover && !musicHoverStates[i]) hoverSound.play();
            musicHoverStates[i] = musicHover;

            if (musicHover) {
                musicButtons[i].setOutlineThickness(3);
                if (mousePressed) {
                    clickSound.play();
                    float volume = (i + 1) * 30.f;
                    music.setVolume(volume);
                    std::ofstream("volume.txt") << volume;
                    for (int j = 0; j < 3; ++j) {
                        musicButtons[j].setFillColor(j == i ? sf::Color::Green : sf::Color(100, 100, 100));
                    }
                }
            }
            else {
                musicButtons[i].setOutlineThickness(2);
            }

 
            bool soundHover = soundButtons[i].getGlobalBounds().contains(mousePos);
            if (soundHover && !soundHoverStates[i]) hoverSound.play();
            soundHoverStates[i] = soundHover;

            if (soundHover) {
                soundButtons[i].setOutlineThickness(3);
                if (mousePressed) {
                    clickSound.play();
                    float volume = (i + 1) * 30.f;
                    hoverSound.setVolume(volume);
                    clickSound.setVolume(volume);
                    std::ofstream("volumesound.txt") << volume;
                    for (int j = 0; j < 3; ++j) {
                        soundButtons[j].setFillColor(j == i ? sf::Color::Green : sf::Color(100, 100, 100));
                    }
                }
            }
            else {
                soundButtons[i].setOutlineThickness(2);
            }
        }


        window.clear(sf::Color(30, 30, 30));


        window.draw(musicText);
        window.draw(soundText);


        window.draw(backButton);
        for (int i = 0; i < 3; ++i) {
            window.draw(musicButtons[i]);
            window.draw(soundButtons[i]);

    
            sf::Text num(std::to_string(i + 1), font, 36);
            num.setPosition(centerX + i * buttonSpacing + buttonSize / 2 - 12, musicY + 10);
            window.draw(num);

            num.setPosition(centerX + i * buttonSpacing + buttonSize / 2 - 12, soundY + 10);
            window.draw(num);
        }

        window.display();
    }
}