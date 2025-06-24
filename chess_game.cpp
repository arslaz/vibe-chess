#include "chess_game.h"
#include "engine.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <cstring>
#include <fstream>
#include <algorithm>

const int TILE_SIZE = 100;
const sf::Vector2f BOARD_POSITION(560, 140);
const int MAX_PIECES = 32;
const std::string LOG_FILENAME = "chess_results.txt";
const std::string PLAYER_NAME = "Player";
const std::string BOT_NAME = "Stockfish";

struct GameSounds {
    sf::SoundBuffer moveBuffer;
    sf::SoundBuffer captureBuffer;
    sf::Sound moveSound;
    sf::Sound captureSound;

    bool loadSounds() {
        if (!moveBuffer.loadFromFile("sound/click.wav") ||
            !captureBuffer.loadFromFile("sound/hover.mp3")) {
            std::cerr << "Failed to load sound files\n";
            return false;
        }
        moveSound.setBuffer(moveBuffer);
        captureSound.setBuffer(captureBuffer);
        return true;
    }
};

struct PromotionWindow {
    sf::RectangleShape background;
    sf::Text title;
    sf::RectangleShape pieces[4];
    sf::Sprite pieceSprites[4];
    bool visible = false;
    int selectedPiece = 0;
    sf::Vector2i promotionPos;

    PromotionWindow(sf::Font& font, sf::Texture& pieceTex) {
        background.setSize(sf::Vector2f(400, 200));
        background.setFillColor(sf::Color(50, 50, 50, 220));
        background.setOutlineThickness(3);
        background.setOutlineColor(sf::Color(200, 170, 50));

        title.setFont(font);
        title.setString(L"Выберите фигуру:");
        title.setCharacterSize(30);
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold);

        for (int i = 0; i < 4; ++i) {
            pieces[i].setSize(sf::Vector2f(80, 80));
            pieces[i].setFillColor(sf::Color(70, 70, 70));
            pieces[i].setOutlineThickness(2);
            pieces[i].setOutlineColor(sf::Color(200, 170, 50));

            pieceSprites[i].setTexture(pieceTex);
            int type = (i == 0) ? 3 : (i == 1) ? 1 : (i == 2) ? 2 : 0;
            pieceSprites[i].setTextureRect(sf::IntRect(type * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE));
            pieceSprites[i].setOrigin(TILE_SIZE / 2.f, TILE_SIZE / 2.f);
        }
    }

    void setPosition(int x, int y, bool isWhite) {
        promotionPos = sf::Vector2i(x, y);
        background.setPosition(x - 200, y - 100);
        title.setPosition(x - 190, y - 80);

        for (int i = 0; i < 4; ++i) {
            pieces[i].setPosition(x - 150 + i * 100, y);
            pieceSprites[i].setPosition(x - 110 + i * 100, y + 40);
            pieceSprites[i].setTextureRect(sf::IntRect(
                (i == 0) ? 3 : (i == 1) ? 1 : (i == 2) ? 2 : 0,
                isWhite ? 0 : 1,
                TILE_SIZE, TILE_SIZE
            ));
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!visible) return;
        window.draw(background);
        window.draw(title);
        for (int i = 0; i < 4; ++i) {
            window.draw(pieces[i]);
            window.draw(pieceSprites[i]);
        }
    }

    bool handleEvent(sf::Event& event, sf::RenderWindow& window) {
        if (!visible) return false;
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        for (int i = 0; i < 4; ++i) {
            if (pieces[i].getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    selectedPiece = (i == 0) ? 5 : (i == 1) ? 2 : (i == 2) ? 3 : 4;
                    visible = false;
                    return true;
                }
            }
        }
        return false;
    }
};

struct GameOverScreen {
    sf::RectangleShape background;
    sf::Text message;
    sf::RectangleShape menuButton;
    sf::Text menuButtonText;
    sf::RectangleShape restartButton;
    sf::Text restartButtonText;
    bool visible = false;
    bool isWhiteWinner = false;

    GameOverScreen(sf::Font& font) {
        background.setSize(sf::Vector2f(600, 300));
        background.setFillColor(sf::Color(30, 30, 30, 220));
        background.setOutlineThickness(3);
        background.setOutlineColor(sf::Color(200, 170, 50));
        background.setPosition((1920 - 600) / 2, (1080 - 300) / 2);

        message.setFont(font);
        message.setCharacterSize(50);
        message.setFillColor(sf::Color::White);
        message.setStyle(sf::Text::Bold);

        menuButton.setSize(sf::Vector2f(250, 60));
        menuButton.setFillColor(sf::Color(100, 100, 70));
        menuButton.setOutlineThickness(2);
        menuButton.setOutlineColor(sf::Color(100, 100, 100));

        menuButtonText.setFont(font);
        menuButtonText.setString(L"Главное меню");
        menuButtonText.setCharacterSize(30);
        menuButtonText.setFillColor(sf::Color::White);

        restartButton.setSize(sf::Vector2f(250, 60));
        restartButton.setFillColor(sf::Color(100, 100, 100));
        restartButton.setOutlineThickness(2);
        restartButton.setOutlineColor(sf::Color(100, 100, 100));

        restartButtonText.setFont(font);
        restartButtonText.setString(L"Заново");
        restartButtonText.setCharacterSize(30);
        restartButtonText.setFillColor(sf::Color::White);

        updatePositions();
    }

    void setWinner(bool whiteWon) {
        isWhiteWinner = whiteWon;
        message.setString(whiteWon ? L"Белые выиграли!" : L"Черные выиграли!");
        updatePositions();
    }

    void updatePositions() {
        sf::FloatRect textRect = message.getLocalBounds();
        message.setOrigin(textRect.left + textRect.width / 2.0f,
            textRect.top + textRect.height / 2.0f);
        message.setPosition(background.getPosition().x + background.getSize().x / 2,
            background.getPosition().y + 80);

        float buttonY = background.getPosition().y + 180;
        float centerX = background.getPosition().x + background.getSize().x / 2;

        menuButton.setPosition(centerX - 270, buttonY);
        menuButtonText.setPosition(centerX - 240, buttonY + 10);

        restartButton.setPosition(centerX + 20, buttonY);
        restartButtonText.setPosition(centerX + 70, buttonY + 10);
    }

    void draw(sf::RenderWindow& window) {
        if (!visible) return;
        window.draw(background);
        window.draw(message);
        window.draw(menuButton);
        window.draw(menuButtonText);
        window.draw(restartButton);
        window.draw(restartButtonText);
    }

    bool isMenuButtonClicked(sf::Vector2i mousePos) {
        return visible && menuButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }

    bool isRestartButtonClicked(sf::Vector2i mousePos) {
        return visible && restartButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }
};

int getTextureIndex(int piece) {
    switch (abs(piece)) {
    case 1: return 5; // pawn
    case 2: return 1; // knight
    case 3: return 2; // bishop
    case 4: return 0; // rook
    case 5: return 3; // queen
    case 6: return 4; // king
    default: return -1;
    }
}

std::string toChessNotation(int x, int y) {
    char file = 'a' + x;
    char rank = '8' - y;
    return std::string(1, file) + std::string(1, rank);
}

bool isValidCoordinate(int x, int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

struct PieceSprite {
    int x, y, piece;
    sf::Sprite sprite;
    bool alive = true;
};

void updatePieceSprites(PieceSprite pieces[], int& pieceCount, int boardLayout[8][8], sf::Texture& tex) {
    pieceCount = 0;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            int piece = boardLayout[y][x];
            if (!piece) continue;

            PieceSprite ps;
            ps.x = x;
            ps.y = y;
            ps.piece = piece;
            ps.sprite.setTexture(tex);

            int type = getTextureIndex(piece);
            int color = piece > 0 ? 1 : 0;
            ps.sprite.setTextureRect(sf::IntRect(type * TILE_SIZE, color * TILE_SIZE, TILE_SIZE, TILE_SIZE));
            ps.sprite.setOrigin(TILE_SIZE / 2.f, TILE_SIZE / 2.f);
            ps.sprite.setPosition(
                BOARD_POSITION.x + x * TILE_SIZE + TILE_SIZE / 2.f,
                BOARD_POSITION.y + y * TILE_SIZE + TILE_SIZE / 2.f
            );

            ps.alive = true;
            pieces[pieceCount++] = ps;
        }
    }
}

int findPieceIndex(PieceSprite pieces[], int pieceCount, int x, int y) {
    for (int i = 0; i < pieceCount; ++i) {
        if (pieces[i].x == x && pieces[i].y == y && pieces[i].alive)
            return i;
    }
    return -1;
}

bool applyMove(int layout[8][8], const std::string& move, PieceSprite pieces[], int& pieceCount,
    sf::Texture& pieceTex, PromotionWindow& promoWindow, GameSounds& sounds) {
    if (move.length() < 4) return false;

    int fromX = move[0] - 'a';
    int fromY = 7 - (move[1] - '1');
    int toX = move[2] - 'a';
    int toY = 7 - (move[3] - '1');

    if (!isValidCoordinate(fromX, fromY) || !isValidCoordinate(toX, toY))
        return false;

    int piece = layout[fromY][fromX];
    int capturedPiece = layout[toY][toX];
    bool isCapture = (capturedPiece != 0);

    if (abs(piece) == 6 && abs(fromX - toX) == 2) {
        bool isKingside = (toX > fromX);
        int rookFromX = isKingside ? 7 : 0;
        int rookToX = isKingside ? 5 : 3;

        layout[fromY][rookToX] = layout[fromY][rookFromX];
        layout[fromY][rookFromX] = 0;
        updatePieceSprites(pieces, pieceCount, layout, pieceTex);
    }
    else if (abs(piece) == 1 && (toY == 0 || toY == 7)) {
        promoWindow.setPosition(BOARD_POSITION.x + toX * TILE_SIZE + TILE_SIZE / 2,
            BOARD_POSITION.y + toY * TILE_SIZE + TILE_SIZE / 2,
            piece > 0);
        promoWindow.visible = true;
        promoWindow.promotionPos = sf::Vector2i(toX, toY);
    }
    else {
        if (isCapture) sounds.captureSound.play();
        else sounds.moveSound.play();
    }

    layout[toY][toX] = piece;
    layout[fromY][fromX] = 0;

    if (!promoWindow.visible) {
        updatePieceSprites(pieces, pieceCount, layout, pieceTex);
    }

    return true;
}

bool checkForMate(ChessEngine& engine, const std::string& moveHistory, bool whiteToMove) {
    engine.SendCommand("position startpos moves " + moveHistory);
    engine.SendCommand("go depth 1");
    std::string response = engine.GetResponse(5000);
    return response.find("mate 0") != std::string::npos || response.find("stalemate") != std::string::npos;
}

void logGameResult(bool isWhiteWinner) {
    std::ofstream logFile(LOG_FILENAME, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Error: Could not open log file!" << std::endl;
        return;
    }

    static int gameNumber = 0;
    std::ifstream inFile(LOG_FILENAME);
    std::string line;
    while (std::getline(inFile, line)) gameNumber++;

    logFile << gameNumber + 1 << ". " << (isWhiteWinner ? "White wins" : "Black wins") << "\n";
}

void makeBotMove(ChessEngine& engine, int layout[8][8], std::string& moveHistory,
    PieceSprite pieces[], int& pieceCount, sf::Texture& pieceTex,
    const ChessGameSettings& settings, bool& gameOver, PromotionWindow& promoWindow,
    GameSounds& sounds, GameOverScreen& gameOverScreen, int difficulty) {

    engine.SendCommand("go depth " + std::to_string(settings.engineDepth));
    std::string botResponse = engine.GetResponse(10000);

    size_t bestMovePos = botResponse.find("bestmove ");
    if (bestMovePos != std::string::npos) {
        std::string botMove = botResponse.substr(bestMovePos + 9, 4);

        if (applyMove(layout, botMove, pieces, pieceCount, pieceTex, promoWindow, sounds)) {
            moveHistory += (moveHistory.empty() ? "" : " ") + botMove;

            int toY = 7 - (botMove[3] - '1');
            int toX = botMove[2] - 'a';
            int piece = layout[toY][toX];
            bool isPawnPromotion = (abs(piece) == 1) && (toY == 0 || toY == 7);

            if (isPawnPromotion) {
                layout[toY][toX] = (piece > 0) ? 5 : -5;
            }

            updatePieceSprites(pieces, pieceCount, layout, pieceTex);

            if (checkForMate(engine, moveHistory, true)) {
                gameOver = true;
                gameOverScreen.visible = true;
                gameOverScreen.setWinner(false);
                logGameResult(false);
            }
        }
    }
}

void runChessGame(sf::RenderWindow& window, const ChessGameSettings& settings, int level) {
    ChessEngine engine;
    if (!engine.ConnectToEngine(L"stockfish.exe")) {
        std::cerr << "Failed to start Stockfish!\n";
        return;
    }
    engine.setDifficulty(level);

    GameSounds sounds;
    if (!sounds.loadSounds()) {
        std::cerr << "Some sounds will not be available\n";
    }

    sf::Font font;
    if (!font.loadFromFile("image/arial.ttf")) {
        std::cerr << "Failed to load font!\n";
        return;
    }

    sf::Texture boardTex, pieceTex, backTex;
    if (!boardTex.loadFromFile("PNGs/ChessBoard.png") ||
        !pieceTex.loadFromFile("PNGs/ChessPieces.png") ||
        !backTex.loadFromFile("image/back.png")) {
        std::cerr << "Failed to load textures\n";
        return;
    }

    sf::Sprite board(boardTex);
    board.setPosition(BOARD_POSITION);

    sf::Sprite backButton(backTex);
    backButton.setScale(0.10f, 0.10f);
    backButton.setPosition(20, 20);

    GameOverScreen gameOverScreen(font);
    PromotionWindow promotionWindow(font, pieceTex);

    int layout[8][8] = {
        {-4, -2, -3, -5, -6, -3, -2, -4},
        {-1, -1, -1, -1, -1, -1, -1, -1},
        { 0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0},
        { 1,  1,  1,  1,  1,  1,  1,  1},
        { 4,  2,  3,  5,  6,  3,  2,  4}
    };

    bool isWhiteTurn = true;
    bool gameOver = false;
    std::string moveHistory;
    PieceSprite pieces[MAX_PIECES];
    int pieceCount = 0;
    updatePieceSprites(pieces, pieceCount, layout, pieceTex);

    int dragFromX = -1, dragFromY = -1;
    int dragPieceIndex = -1;
    bool dragging = false;
    sf::Sprite draggedSprite;
    bool hoverBack = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                engine.SafeClose();
                window.close();
            }

            if (promotionWindow.visible && promotionWindow.handleEvent(event, window)) {
                int promoX = promotionWindow.promotionPos.x;
                int promoY = promotionWindow.promotionPos.y;
                int color = (layout[promoY][promoX] > 0) ? 1 : -1;
                layout[promoY][promoX] = promotionWindow.selectedPiece * color;
                updatePieceSprites(pieces, pieceCount, layout, pieceTex);

                isWhiteTurn = !isWhiteTurn;

                if (!isWhiteTurn) {
                    makeBotMove(engine, layout, moveHistory, pieces, pieceCount,
                        pieceTex, settings, gameOver, promotionWindow, sounds,
                        gameOverScreen, level);
                    isWhiteTurn = true;
                }
                continue;
            }

            if (gameOverScreen.visible && event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                if (gameOverScreen.isMenuButtonClicked(mousePos)) {
                    if (settings.backgroundMusic) settings.backgroundMusic->play();
                    engine.SafeClose();
                    return;
                }
                else if (gameOverScreen.isRestartButtonClicked(mousePos)) {
                    gameOver = false;
                    gameOverScreen.visible = false;

                    int newLayout[8][8] = {
                        {-4, -2, -3, -5, -6, -3, -2, -4},
                        {-1, -1, -1, -1, -1, -1, -1, -1},
                        { 0,  0,  0,  0,  0,  0,  0,  0},
                        { 0,  0,  0,  0,  0,  0,  0,  0},
                        { 0,  0,  0,  0,  0,  0,  0,  0},
                        { 0,  0,  0,  0,  0,  0,  0,  0},
                        { 1,  1,  1,  1,  1,  1,  1,  1},
                        { 4,  2,  3,  5,  6,  3,  2,  4}
                    };
                    memcpy(layout, newLayout, sizeof(layout));

                    isWhiteTurn = true;
                    moveHistory.clear();
                    updatePieceSprites(pieces, pieceCount, layout, pieceTex);
                }
            }

            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            bool nowHover = backButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            if (nowHover != hoverBack) {
                hoverBack = nowHover;
                backButton.setScale(hoverBack ? 0.15f : 0.10f, hoverBack ? 0.15f : 0.10f);
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && !gameOver && !promotionWindow.visible) {
                if (hoverBack) {
                    if (settings.backgroundMusic) settings.backgroundMusic->play();
                    engine.SafeClose();
                    return;
                }

                int boardX = static_cast<int>((mousePos.x - BOARD_POSITION.x) / TILE_SIZE);
                int boardY = static_cast<int>((mousePos.y - BOARD_POSITION.y) / TILE_SIZE);

                if (isValidCoordinate(boardX, boardY)) {
                    int piece = layout[boardY][boardX];
                    if ((isWhiteTurn && piece > 0) || (!isWhiteTurn && piece < 0)) {
                        dragFromX = boardX;
                        dragFromY = boardY;
                        dragPieceIndex = findPieceIndex(pieces, pieceCount, boardX, boardY);

                        if (dragPieceIndex != -1) {
                            dragging = true;
                            draggedSprite = pieces[dragPieceIndex].sprite;
                            pieces[dragPieceIndex].alive = false;
                            draggedSprite.setPosition(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseMoved && dragging) {
                draggedSprite.setPosition(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && dragging && !gameOver) {
                int toX = static_cast<int>((event.mouseButton.x - BOARD_POSITION.x) / TILE_SIZE);
                int toY = static_cast<int>((event.mouseButton.y - BOARD_POSITION.y) / TILE_SIZE);

                bool validMove = false;
                if (isValidCoordinate(toX, toY)) {
                    std::string move = toChessNotation(dragFromX, dragFromY) + toChessNotation(toX, toY);

                    if ((isWhiteTurn && layout[toY][toX] <= 0) || (!isWhiteTurn && layout[toY][toX] >= 0)) {
                        int tempLayout[8][8];
                        memcpy(tempLayout, layout, sizeof(tempLayout));

                        if (applyMove(tempLayout, move, pieces, pieceCount, pieceTex, promotionWindow, sounds)) {
                            std::string newHistory = moveHistory.empty() ? move : moveHistory + " " + move;
                            engine.SendCommand("position startpos moves " + newHistory);
                            engine.SendCommand("isready");
                            std::string response = engine.GetResponse(5000);

                            if (response.find("readyok") != std::string::npos) {
                                memcpy(layout, tempLayout, sizeof(layout));
                                moveHistory = newHistory;
                                validMove = true;

                                if (!promotionWindow.visible) {
                                    isWhiteTurn = !isWhiteTurn;
                                    updatePieceSprites(pieces, pieceCount, layout, pieceTex);

                                    if (checkForMate(engine, moveHistory, !isWhiteTurn)) {
                                        gameOver = true;
                                        gameOverScreen.visible = true;
                                        gameOverScreen.setWinner(!isWhiteTurn);
                                        logGameResult(!isWhiteTurn);
                                    }

                                    if (!isWhiteTurn && !gameOver) {
                                        makeBotMove(engine, layout, moveHistory, pieces, pieceCount,
                                            pieceTex, settings, gameOver, promotionWindow, sounds,
                                            gameOverScreen, level);
                                        isWhiteTurn = true;
                                    }
                                }
                            }
                        }
                    }
                }

                if (!validMove && dragPieceIndex != -1) {
                    pieces[dragPieceIndex].alive = true;
                }

                dragging = false;
                dragFromX = dragFromY = -1;
                dragPieceIndex = -1;
            }
        }

        window.clear(sf::Color(50, 50, 50));
        window.draw(board);

        for (int i = 0; i < pieceCount; ++i) {
            if (pieces[i].alive) {
                window.draw(pieces[i].sprite);
            }
        }

        if (dragging) {
            window.draw(draggedSprite);
        }

        window.draw(backButton);
        gameOverScreen.draw(window);
        promotionWindow.draw(window);
        window.display();
    }

    engine.SafeClose();
}