#include "History.h"
#include <fstream>
#include <iostream>

const std::string LOG_FILENAME = "chess_results.txt";

HistoryScreen::HistoryScreen(float soundVolume) : resultsCount(0) {

    if (!font.loadFromFile("image/arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return;
    }


    if (!backTexture.loadFromFile("image/back.png")) {
        std::cerr << "Failed to load back button texture!" << std::endl;
        return;
    }


    backSprite.setTexture(backTexture);
    backSprite.setPosition(30.f, 30.f);
    const float backScale = 60.f / std::max(backTexture.getSize().x, backTexture.getSize().y);
    backSprite.setScale(backScale, backScale);


    background.setSize(sf::Vector2f(1920, 1080));
    background.setFillColor(sf::Color(30, 30, 30));


    title.setFont(font);
    title.setString(L"Результаты партий");
    title.setCharacterSize(50);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    title.setPosition((1920 - title.getGlobalBounds().width) / 2, 50);

    if (!hoverBuffer.loadFromFile("sound/hover.mp3")) {
        std::cerr << "Failed to load hover sound!" << std::endl;
    }
    if (!clickBuffer.loadFromFile("sound/click.wav")) {
        std::cerr << "Failed to load click sound!" << std::endl;
    }
    hoverSound.setBuffer(hoverBuffer);
    clickSound.setBuffer(clickBuffer);
    hoverSound.setVolume(soundVolume);
    clickSound.setVolume(soundVolume);

    backHovered = false;
}

void HistoryScreen::loadResults() {
    resultsCount = 0;
    std::ifstream file(LOG_FILENAME);

    if (!file.is_open()) {
        std::cerr << "Failed to open results file!" << std::endl;

        results[resultsCount].setFont(font);
        results[resultsCount].setString("Файл с историей не найден");
        results[resultsCount].setCharacterSize(30);
        results[resultsCount].setFillColor(sf::Color::White);
        results[resultsCount].setPosition(
            (1920 - results[resultsCount].getGlobalBounds().width) / 2,
            150
        );
        resultsCount = 1;
        return;
    }

    std::string line;
    float yPosition = 150.f;

    while (std::getline(file, line) && resultsCount < MAX_RESULTS) {
        if (!line.empty()) {
            results[resultsCount].setFont(font);
            results[resultsCount].setString(line);
            results[resultsCount].setCharacterSize(30);

            // Определяем цвет в зависимости от результата
            if (line.find("White wins") != std::string::npos ||
                line.find("Победа белых") != std::string::npos) {
                results[resultsCount].setFillColor(sf::Color(100, 255, 100)); 
            }
            else if (line.find("Black wins") != std::string::npos ||
                line.find("Победа черных") != std::string::npos) {
                results[resultsCount].setFillColor(sf::Color(255, 100, 100)); 
            }

            results[resultsCount].setPosition(
                (1920 - results[resultsCount].getGlobalBounds().width) / 2,
                yPosition
            );

            yPosition += 40.f;
            resultsCount++;
        }
    }

    file.close();

    if (resultsCount == 0) {
        results[resultsCount].setFont(font);
        results[resultsCount].setString("История партий пуста");
        results[resultsCount].setCharacterSize(30);
        results[resultsCount].setFillColor(sf::Color::White);
        results[resultsCount].setPosition(
            (1920 - results[resultsCount].getGlobalBounds().width) / 2,
            150
        );
        resultsCount = 1;
    }
}

bool HistoryScreen::handleEvents(sf::RenderWindow& window) {
    sf::Event event;
    bool mousePressed = false;

    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
            return false;
        }
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left) {
            mousePressed = true;
        }
    }

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    // Обработка кнопки "Назад"
    bool currentHover = backSprite.getGlobalBounds().contains(mousePos);
    if (currentHover && !backHovered) {
        hoverSound.play();
    }
    backHovered = currentHover;

    if (backHovered) {
        const float backScale = 60.f / std::max(backTexture.getSize().x, backTexture.getSize().y);
        backSprite.setScale(backScale * 1.1f, backScale * 1.1f);
        if (mousePressed) {
            clickSound.play();
            return false;
        }
    }
    else {
        const float backScale = 60.f / std::max(backTexture.getSize().x, backTexture.getSize().y);
        backSprite.setScale(backScale, backScale);
    }

    return true;
}

void HistoryScreen::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(30, 30, 30));

    // Рисуем фон
    window.draw(background);

    // Рисуем заголовок
    window.draw(title);

    // Рисуем кнопку "Назад"
    window.draw(backSprite);

    // Рисуем все результаты
    for (int i = 0; i < resultsCount; ++i) {
        window.draw(results[i]);
    }

    window.display();
}

void openHistory(sf::RenderWindow& window, float soundVolume) {
    HistoryScreen historyScreen(soundVolume);
    historyScreen.loadResults();

    bool showHistory = true;
    while (showHistory && window.isOpen()) {
        showHistory = historyScreen.handleEvents(window);
        historyScreen.draw(window);
    }
}