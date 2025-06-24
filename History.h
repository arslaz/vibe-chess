#ifndef HISTORY_H
#define HISTORY_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class HistoryScreen {
public:
    HistoryScreen(float soundVolume);
    void loadResults();
    void draw(sf::RenderWindow& window);
    bool handleEvents(sf::RenderWindow& window);

private:
    static const int MAX_RESULTS = 100;
    sf::Font font;
    sf::RectangleShape background;
    sf::Text title;
    sf::Text results[MAX_RESULTS];
    int resultsCount;
    sf::Texture backTexture;
    sf::Sprite backSprite;
    bool backHovered;

  
    sf::SoundBuffer hoverBuffer;
    sf::SoundBuffer clickBuffer;
    sf::Sound hoverSound;
    sf::Sound clickSound;
};

void openHistory(sf::RenderWindow& window, float soundVolume);

#endif 