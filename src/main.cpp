#include <SFML\Graphics.hpp>
#include <SFML\Window.hpp>

#include "AI.hpp"

struct Reference {
    int posX;
    int posY;
};

void viewGame(sf::RenderWindow& window, Game& game, Reference TLCorner, int size, int thick) {

    sf::Text text;
    sf::Font font;
    font.loadFromFile("C:\\Windows\\Fonts\\ARIAL.ttf");
    text.setFont(font); 
    string scoreStr = std::to_string(game.score[0]) + " : " + std::to_string(game.score[1]);
    text.setString(scoreStr);
    text.setCharacterSize(50); 
    text.setPosition(sf::Vector2f(400, 10));
    window.draw(text);

    sf::Texture whitePieceText, blackPieceText;
    whitePieceText.loadFromFile("textures/whitePiece.png");
    blackPieceText.loadFromFile("textures/blackPiece.png");
    for (int l = 0; l < game.w_size; l++) {
        for (int k = game.z_size - 1; k >= 0; k--) {
            for (int j = game.board_size - 1; j >= 0; j--) {
                for (int i = 0; i < game.board_size; i++) {
                    sf::RectangleShape rectangle;
                    sf::Color color(00, 160, 80);
                    rectangle.setSize(sf::Vector2f(size, size));
                    rectangle.setOrigin(sf::Vector2f(size / 2, size / 2));
                    rectangle.setFillColor(color);
                    rectangle.setOutlineColor(sf::Color::Black);
                    rectangle.setOutlineThickness(thick);
                    rectangle.setPosition(sf::Vector2f(TLCorner.posX + (size + thick) * i + ((size + thick) * game.board_size + 4) * l, TLCorner.posY + (size + thick) * j + ((size + thick) * game.board_size + 4) * k));
                    window.draw(rectangle);

                    Board curBoard = game.board_grid[l][game.z_size - 1 - k];
                    U64 sq = 1ull << (((game.board_size - 1 - j) * 8) + i);
                    if ((curBoard.board[0] & sq) != 0) {
                        sf::CircleShape circle(size * .475, 120ULL);
                        circle.setOrigin(sf::Vector2f(circle.getRadius(), circle.getRadius()));
                        circle.setTexture(&blackPieceText);
                        circle.setPosition(sf::Vector2f(TLCorner.posX + (size + thick) * i + ((size + thick) * game.board_size + 4) * l, TLCorner.posY + (size + thick) * j + ((size + thick) * game.board_size + 4) * k));
                        window.draw(circle);
                    }
                    else if ((curBoard.board[1] & sq) != 0) {
                        sf::CircleShape circle(size * .475, 120ULL);
                        circle.setOrigin(sf::Vector2f(circle.getRadius(), circle.getRadius()));
                        circle.setTexture(&whitePieceText);
                        circle.setPosition(sf::Vector2f(TLCorner.posX + (size + thick) * i + ((size + thick) * game.board_size + 4) * l, TLCorner.posY + (size + thick) * j + ((size + thick) * game.board_size + 4) * k));
                        window.draw(circle);
                    }
                }
            }
        }
    }
}

void viewMoves(sf::RenderWindow& window, Game& game, Reference TLCorner, int size, int thick, std::vector<Move> moveList) {
    sf::CircleShape circle(size / 3, 120ULL);
    circle.setOrigin(sf::Vector2f(circle.getRadius(), circle.getRadius()));
    if (!game.color) {
        sf::Color color(80, 80, 80);
        circle.setFillColor(color);
    }
    else if (game.color) {
        sf::Color color(180, 180, 180);
        circle.setFillColor(color);
    }
    for (Move& move : moveList) {
        int idx = SquareOf(move.move);

        circle.setPosition(sf::Vector2f(TLCorner.posX + (size + thick) * (idx % 8) + ((size + thick) * game.board_size + 4) * move.w, TLCorner.posY + (size + thick) * (game.board_size - 1 - (idx / 8)) + ((size + thick) * game.board_size + 4) * (game.z_size - 1 - move.z)));
        window.draw(circle);
    }
}

int main() {
    std::cout << "Start!" << std::endl;
    system("chcp 65001 > nul");
    Board* board0 = new Board(0, 0);
    Board* board1 = new Board(0x20400ULL, 0x40200ULL);
    Board* board2 = new Board(0x40200ULL, 0x20400ULL);

    std::vector<std::vector<Board>> board_grid(4, std::vector<Board>(4, *board0));
    board_grid[1][1] = *board1;
    board_grid[1][2] = *board2;
    board_grid[2][1] = *board2;
    board_grid[2][2] = *board1;

    Game game(4, 4, 4, board_grid);

    std::cout << game;

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "4D Othello");
    window.setFramerateLimit(60);

    float x, y;
    int size = 50;
    int thick = 3;
    Reference TLCorner = { 50, 100 };
    std::vector<Move> moveList = game.movegen();
    while (window.isOpen()) {

        window.clear();

        viewGame(window, game, TLCorner, size, thick);
        if (game.gameOver()) {
            std::cout << "gameOver" << std::endl;
        }
        viewMoves(window, game, TLCorner, size, thick, moveList);

        window.display();

        sf::Vector2i pos = sf::Mouse::getPosition(window);
        x = pos.x;
        y = pos.y;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); }

            if (event.type == sf::Event::MouseButtonPressed && game.color == 0) {
                if (event.key.code == sf::Mouse::Left) {
                    int l = (x - (TLCorner.posX - size / 2)) / ((size + thick) * game.board_size + 4);
                    int i = ((x - (TLCorner.posX - size / 2)) / ((size + thick) * game.board_size + 4) - l) * game.board_size;
                    int k = (y - (TLCorner.posY - size / 2)) / ((size + thick) * game.board_size + 4);
                    int j = game.board_size - ((y - (TLCorner.posY - size / 2)) / ((size + thick) * game.board_size + 4) - k) * game.board_size;
                    k = game.z_size - 1 - k;

                    U64 sq = 1ULL << (j * 8 + i);
                    for (Move& move : moveList) {
                        if (move.move == sq && move.z == k && move.w == l) {
                            game.makeMove(move);
                            window.clear();
                            viewGame(window, game, TLCorner, size, thick);
                            window.display();
                            Move aiMove = depthSearch(game, 4);
                            game.makeMove(aiMove);
                            moveList = game.movegen();
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

