// chess_game.h
#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

struct ChessGameSettings {
    sf::Sound* moveSound = nullptr;
    sf::Music* backgroundMusic = nullptr;
    float soundVolume = 50.f;
    float musicVolume = 50.f;
    int engineDepth = 10; // уровень сложности
};

void runChessGame(sf::RenderWindow& window, const ChessGameSettings& settings);
