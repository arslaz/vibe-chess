#include "Menu.h"
#include "NewGame.h"
#include "settings.h"
#include "Button.h"
#include <SFML/Audio.hpp>
#include <fstream>

void startGame(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.loadFromFile("image/arial.ttf")) {
        return;
    }

    sf::Music bgMusic;
    if (bgMusic.openFromFile("sound/music.mp3")) {
        float volume = 100.f;
        std::ifstream vfile("volume.txt");
        if (vfile.is_open()) {
            vfile >> volume;
            vfile.close();
        }
        bgMusic.setVolume(volume);
        bgMusic.setLoop(true);
        bgMusic.play();
    }


    sf::SoundBuffer hoverBuffer, clickBuffer;
    if (!hoverBuffer.loadFromFile("sound/hover.mp3") || !clickBuffer.loadFromFile("sound/click.wav")) {
        return;
    }

    sf::Sound hoverSound(hoverBuffer), clickSound(clickBuffer);
    float soundVolume = 100.f;
    std::ifstream svfile("volumesound.txt");
    if (svfile.is_open()) {
        svfile >> soundVolume;
        svfile.close();
    }
    hoverSound.setVolume(soundVolume);
    clickSound.setVolume(soundVolume);

    sf::Texture logoTexture;
    if (!logoTexture.loadFromFile("image/logo2.png")) {
        return;
    }
    sf::Sprite logoSprite(logoTexture);
    logoSprite.setPosition(700, 70);
    logoSprite.setScale(0.5f, 0.5f);


    Button buttons[4];
    const std::string labels[4] = { "New Game", "History", "Settings", "Exit" };

    for (int i = 0; i < 4; ++i) {
        buttons[i].setup(font, labels[i],
            sf::Vector2f(810, 350 + i * 80),
            sf::Vector2f(300, 60),
            &hoverSound, &clickSound);
    }

    while (window.isOpen()) {
        sf::Event event;
        bool mousePressed = false;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                mousePressed = true;
            }
        }

        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));


        for (int i = 0; i < 4; ++i) {
            buttons[i].update(mousePos);

            if (buttons[i].isClicked(mousePos, mousePressed)) {
                if (labels[i] == "New Game") {
                    clickSound.play();
                    window.clear(sf::Color(30, 30, 30)); 
                    openNewGame(window, bgMusic.getVolume(), soundVolume);

                    hoverSound.setVolume(soundVolume);
                    clickSound.setVolume(soundVolume);
                    continue; 
                }
                else if (labels[i] == "Settings") {
                    openSettings(window, bgMusic, hoverSound, clickSound);

                    hoverSound.setVolume(soundVolume);
                    clickSound.setVolume(soundVolume);
                }
                else if (labels[i] == "Exit") {
                    window.close();
                }
            }
        }


        window.clear(sf::Color(30, 30, 30));
        window.draw(logoSprite);
        for (int i = 0; i < 4; ++i) {
            buttons[i].draw(window);
        }
        window.display();
    }
}