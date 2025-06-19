#include "chess_game.h"
#include "engine.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>

const int TILE_SIZE = 100;
const sf::Vector2f BOARD_POSITION(560, 140);
const int MAX_PIECES = 32;
const std::string HISTORY_FILE = "chess_history.txt";

// Sound effects structure
struct GameSounds {
    sf::SoundBuffer moveBuffer;
    sf::SoundBuffer captureBuffer;
    sf::Sound moveSound;
    sf::Sound captureSound;

    bool loadSounds() {
        if (!moveBuffer.loadFromFile("sound/move.wav") ||
            !captureBuffer.loadFromFile("sound/capture.wav")) {
            std::cerr << "Failed to load sound effects\n";
            return false;
        }

        moveSound.setBuffer(moveBuffer);
        captureSound.setBuffer(captureBuffer);
        return true;
    }
};

// Function to write game result to history file
void writeGameHistory(const std::string& result, const std::string& moves) {
    std::ofstream file(HISTORY_FILE, std::ios::app);
    if (!file) {
        std::cerr << "Failed to open history file\n";
        return;
    }

    // —читаем количество уже существующих игр
    int gameCount = 0;
    {
        std::ifstream inFile(HISTORY_FILE);
        std::string line;
        while (std::getline(inFile, line)) {
            if (line.find("Game #") != std::string::npos) {
                gameCount++;
            }
        }
    }

    // «аписываем новую игру без даты
    file << "Game #" << (gameCount + 1)
        << " | Result: " << result << "\n"
        << "Moves: " << moves << "\n\n";
}

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
    return std::string(1, 'a' + x) + std::string(1, '8' - y);
}

bool isValidCoordinate(int x, int y) {
    return (x >= 0 && x < 8 && y >= 0 && y < 8);
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
            ps.sprite.setTextureRect({ type * TILE_SIZE, color * TILE_SIZE, TILE_SIZE, TILE_SIZE });
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

bool applyMove(int layout[8][8], const std::string& move, GameSounds& sounds) {
    if (move.length() < 4) return false;

    int fromX = move[0] - 'a';
    int fromY = 7 - (move[1] - '1');
    int toX = move[2] - 'a';
    int toY = 7 - (move[3] - '1');

    if (!isValidCoordinate(fromX, fromY) || !isValidCoordinate(toX, toY))
        return false;

    bool isCapture = (layout[toY][toX] != 0);
    layout[toY][toX] = layout[fromY][fromX];
    layout[fromY][fromX] = 0;

    // Play appropriate sound
    if (isCapture) {
        sounds.captureSound.play();
    }
    else {
        sounds.moveSound.play();
    }

    return true;
}

void makeBotMove(ChessEngine& engine, int layout[8][8], std::string& moveHistory,
    PieceSprite pieces[], int& pieceCount, sf::Texture& pieceTex,
    const ChessGameSettings& settings, GameSounds& sounds) {
    std::cout << "Requesting bot move..." << std::endl;
    engine.SendCommand("go depth " + std::to_string(settings.engineDepth));
    std::string botResponse = engine.GetResponse(10000);
    std::cout << "Raw bot response: " << botResponse << std::endl;

    size_t bestMovePos = botResponse.find("bestmove ");
    if (bestMovePos != std::string::npos) {
        std::string botMove = botResponse.substr(bestMovePos + 9, 4);
        std::cout << "Parsed bot move: " << botMove << std::endl;

        if (applyMove(layout, botMove, sounds)) {
            moveHistory += (moveHistory.empty() ? "" : " ") + botMove;
            updatePieceSprites(pieces, pieceCount, layout, pieceTex);
            std::cout << "Bot move applied successfully" << std::endl;
        }
        else {
            std::cerr << "Failed to apply bot move!" << std::endl;
        }
    }
    else {
        std::cerr << "No valid move found in response!" << std::endl;
    }
}

void showGameResult(sf::RenderWindow& window, const std::string& result, const std::string& moves) {
    writeGameHistory(result, moves);

    sf::Font font;
    if (!font.loadFromFile("image/arial.ttf")) {
        std::cerr << "Failed to load font for result display\n";
        return;
    }

    sf::Text resultText(result, font, 50);
    resultText.setFillColor(sf::Color::White);
    resultText.setPosition(400, 300);

    sf::Text hintText("Press ESC to continue", font, 20);
    hintText.setFillColor(sf::Color(200, 200, 200));
    hintText.setPosition(450, 400);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                return;
            }
        }

        window.clear(sf::Color(30, 30, 30));
        window.draw(resultText);
        window.draw(hintText);
        window.display();
    }
}

void runChessGame(sf::RenderWindow& window, const ChessGameSettings& settings, int level) {
    // Initialize sounds
    GameSounds sounds;
    if (!sounds.loadSounds()) {
        std::cerr << "Some sound effects will be missing\n";
    }

    ChessEngine engine;
    if (!engine.ConnectToEngine(L"stockfish.exe")) {
        std::cerr << "Failed to start Stockfish!\n";
        return;
    }
    engine.setDifficulty(level);

    // Initialize textures
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

    // Initial board setup
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
    std::string moveHistory;
    PieceSprite pieces[MAX_PIECES];
    int pieceCount = 0;
    updatePieceSprites(pieces, pieceCount, layout, pieceTex);

    // Dragging variables
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

            // Handle back button
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            bool nowHover = backButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            if (nowHover != hoverBack) {
                hoverBack = nowHover;
                backButton.setScale(hoverBack ? 0.15f : 0.10f, hoverBack ? 0.15f : 0.10f);
                if (hoverBack && settings.moveSound) settings.moveSound->play();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
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

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && dragging) {
                int toX = static_cast<int>((event.mouseButton.x - BOARD_POSITION.x) / TILE_SIZE);
                int toY = static_cast<int>((event.mouseButton.y - BOARD_POSITION.y) / TILE_SIZE);

                bool validMove = false;
                if (isValidCoordinate(toX, toY)) {
                    std::string move = toChessNotation(dragFromX, dragFromY) + toChessNotation(toX, toY);
                    std::cout << "Attempting move: " << move << std::endl;

                    if ((isWhiteTurn && layout[toY][toX] <= 0) || (!isWhiteTurn && layout[toY][toX] >= 0)) {
                        int capturedPiece = layout[toY][toX];
                        int movingPiece = layout[dragFromY][dragFromX];

                        // Make temporary move
                        layout[toY][toX] = movingPiece;
                        layout[dragFromY][dragFromX] = 0;

                        // Validate with Stockfish
                        std::string newHistory = moveHistory.empty() ? move : moveHistory + " " + move;
                        engine.SendCommand("position startpos moves " + newHistory);
                        engine.SendCommand("isready");
                        std::string response = engine.GetResponse(5000);

                        if (response.find("readyok") != std::string::npos) {
                            // Play sound for the move
                            if (capturedPiece != 0) {
                                sounds.captureSound.play();
                            }
                            else {
                                sounds.moveSound.play();
                            }

                            moveHistory = newHistory;
                            validMove = true;
                            isWhiteTurn = !isWhiteTurn;

                            updatePieceSprites(pieces, pieceCount, layout, pieceTex);

                            // Check for game end conditions
                            engine.SendCommand("position startpos moves " + moveHistory);
                            engine.SendCommand("go depth 1");
                            std::string gameStateResponse = engine.GetResponse(1000);

                            if (gameStateResponse.find("mate") != std::string::npos) {
                                std::string winner = isWhiteTurn ? "Black" : "White";
                                showGameResult(window, winner + " wins by checkmate!", moveHistory);
                                engine.SafeClose();
                                return;
                            }
                            else if (gameStateResponse.find("stalemate") != std::string::npos) {
                                showGameResult(window, "Draw by stalemate!", moveHistory);
                                engine.SafeClose();
                                return;
                            }

                            // Bot move
                            if (!isWhiteTurn) {
                                makeBotMove(engine, layout, moveHistory, pieces, pieceCount, pieceTex, settings, sounds);
                                isWhiteTurn = true;

                                // Check again for game end after bot move
                                engine.SendCommand("position startpos moves " + moveHistory);
                                engine.SendCommand("go depth 1");
                                gameStateResponse = engine.GetResponse(1000);

                                if (gameStateResponse.find("mate") != std::string::npos) {
                                    std::string winner = isWhiteTurn ? "Black" : "White";
                                    showGameResult(window, winner + " wins by checkmate!", moveHistory);
                                    engine.SafeClose();
                                    return;
                                }
                                else if (gameStateResponse.find("stalemate") != std::string::npos) {
                                    showGameResult(window, "Draw by stalemate!", moveHistory);
                                    engine.SafeClose();
                                    return;
                                }
                            }
                        }
                        else {
                            // Revert if invalid
                            layout[dragFromY][dragFromX] = movingPiece;
                            layout[toY][toX] = capturedPiece;
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

            // Show history when H key is pressed
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::H) {
                showGameResult(window, "Game History", moveHistory);
            }
        }

        // Rendering
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
        window.display();
    }

    engine.SafeClose();
}