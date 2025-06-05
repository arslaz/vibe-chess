#include "chess_game.h"
#include "engine.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>

const int TILE_SIZE = 100;
const sf::Vector2f BOARD_POSITION(560, 140);

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

void updatePieceSprites(std::vector<PieceSprite>& pieces, int boardLayout[8][8], sf::Texture& tex) {
    pieces.clear();
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
            pieces.push_back(ps);
        }
    }
}

int findPieceIndex(std::vector<PieceSprite>& pieces, int x, int y) {
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (pieces[i].x == x && pieces[i].y == y && pieces[i].alive)
            return (int)i;
    }
    return -1;
}

bool applyMove(int layout[8][8], const std::string& move) {
    if (move.length() < 4) return false;

    int fromX = move[0] - 'a';
    int fromY = 7 - (move[1] - '1'); // Convert to 0-based index
    int toX = move[2] - 'a';
    int toY = 7 - (move[3] - '1');

    if (!isValidCoordinate(fromX, fromY) || !isValidCoordinate(toX, toY))
        return false;

    layout[toY][toX] = layout[fromY][fromX];
    layout[fromY][fromX] = 0;
    return true;
}

void makeBotMove(ChessEngine& engine, int layout[8][8], std::string& moveHistory,
    std::vector<PieceSprite>& pieces, sf::Texture& pieceTex,
    const ChessGameSettings& settings) {
    std::cout << "Requesting bot move..." << std::endl;
    engine.SendCommand("go depth " + std::to_string(settings.engineDepth));
    std::string botResponse = engine.GetResponse(10000);
    std::cout << "Raw bot response: " << botResponse << std::endl;

    size_t bestMovePos = botResponse.find("bestmove ");
    if (bestMovePos != std::string::npos) {
        std::string botMove = botResponse.substr(bestMovePos + 9, 4);
        std::cout << "Parsed bot move: " << botMove << std::endl;

        if (applyMove(layout, botMove)) {
            moveHistory += (moveHistory.empty() ? "" : " ") + botMove;
            updatePieceSprites(pieces, layout, pieceTex);
            if (settings.moveSound) settings.moveSound->play();
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

void runChessGame(sf::RenderWindow& window, const ChessGameSettings& settings) {
    ChessEngine engine;
    if (!engine.ConnectToEngine(L"stockfish.exe")) {
        std::cerr << "Failed to start Stockfish!\n";
        return;
    }

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
    backButton.setScale(0.25f, 0.25f);
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
    std::vector<PieceSprite> pieces;
    updatePieceSprites(pieces, layout, pieceTex);

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
                backButton.setScale(hoverBack ? 0.3f : 0.25f, hoverBack ? 0.3f : 0.25f);
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
                        dragPieceIndex = findPieceIndex(pieces, boardX, boardY);

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
                            moveHistory = newHistory;
                            validMove = true;
                            isWhiteTurn = !isWhiteTurn;

                            updatePieceSprites(pieces, layout, pieceTex);
                            if (settings.moveSound) settings.moveSound->play();

                            // Bot move
                            if (!isWhiteTurn) {
                                makeBotMove(engine, layout, moveHistory, pieces, pieceTex, settings);
                                isWhiteTurn = true;
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
        }

        // Rendering
        window.clear(sf::Color(50, 50, 50));
        window.draw(board);

        for (const auto& piece : pieces) {
            if (piece.alive) {
                window.draw(piece.sprite);
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