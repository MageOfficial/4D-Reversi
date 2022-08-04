#include <SFML\Graphics.hpp>
#include <SFML\Window.hpp>

#include "AI.cpp"

struct Reference {
    int posX;
    int posY;
};

void viewFullBoard(sf::RenderWindow& window, FullBoard fullboard, Reference TLCorner, int size, int thick) {

    sf::Text text;
    sf::Font font;
    font.loadFromFile("C:\\Windows\\Fonts\\ARIAL.ttf");
    text.setFont(font); 
    String scoreStr=std::to_string(fullboard.score[0])+" : "+std::to_string(fullboard.score[1]);
    text.setString(scoreStr);
    text.setCharacterSize(50); 
    text.setPosition(400,10);
    window.draw(text);

    sf::Texture whitePieceText, blackPieceText;
    whitePieceText.loadFromFile("textures/whitePiece.png");
    blackPieceText.loadFromFile("textures/blackPiece.png");

    for (int l = 0; l < fullboard.w_size; l++) {
        for (int k = fullboard.z_size - 1; k >= 0; k--) {
            for (int j = fullboard.board_size - 1; j >= 0; j--) {
                for (int i = 0; i < fullboard.board_size; i++) {
                    sf::RectangleShape rectangle;
                    sf::Color color(120, 120, 120);
                    rectangle.setSize(sf::Vector2f(size, size));
                    rectangle.setOrigin(size / 2, size / 2);
                    rectangle.setFillColor(color);
                    rectangle.setOutlineColor(sf::Color::Black);
                    rectangle.setOutlineThickness(thick);
                    rectangle.setPosition(TLCorner.posX + (size + thick) * i + ((size + thick) * fullboard.board_size + 4) * l, TLCorner.posY + (size + thick) * j + ((size + thick) * fullboard.board_size + 4) * k);
                    window.draw(rectangle);

                    Board curBoard = fullboard.w_axis[l].z_axis[fullboard.z_size - 1 - k];
                    U64 sq = 1ull << (((fullboard.board_size - 1 - j) * 8) + i);
                    if ((curBoard.board[0] & sq) != 0) {
                        //std::cout<<"Black: "<<l<<" "<< i<<" "<<k<<" "<<j<<std::endl;
                        sf::CircleShape circle(size * .475, 120ULL);
                        circle.setOrigin(circle.getRadius(), circle.getRadius());
                        circle.setTexture(&blackPieceText);
                        //circle.setFillColor(sf::Color::Black);
                        circle.setPosition(TLCorner.posX + (size + thick) * i + ((size + thick) * fullboard.board_size + 4) * l, TLCorner.posY + (size + thick) * j + ((size + thick) * fullboard.board_size + 4) * k);
                        window.draw(circle);
                    }
                    else if ((curBoard.board[1] & sq) != 0) {
                        //std::cout<<"White: "<<l<<" "<< i<<" "<<k<<" "<<j<<std::endl;
                        sf::CircleShape circle(size * .475, 120ULL);
                        circle.setOrigin(circle.getRadius(), circle.getRadius());
                        circle.setTexture(&whitePieceText);
                        //circle.setFillColor(sf::Color::White);
                        circle.setPosition(TLCorner.posX + (size + thick) * i + ((size + thick) * fullboard.board_size + 4) * l, TLCorner.posY + (size + thick) * j + ((size + thick) * fullboard.board_size + 4) * k);
                        window.draw(circle);
                    }
                }
            }
        }
    }
}

void viewMoves(sf::RenderWindow& window, FullBoard fullboard, Reference TLCorner, int size, int thick, std::vector<Move> moveList, int turn) {
    sf::CircleShape circle(size / 3, 120ULL);
    circle.setOrigin(circle.getRadius(), circle.getRadius());
    if (turn == 0) {
        sf::Color color(80, 80, 80);
        circle.setFillColor(color);
    }
    else if (turn == 1) {
        sf::Color color(180, 180, 180);
        circle.setFillColor(color);
    }
    for (Move& move : moveList) {
        int idx = SquareOf(move.move);

        circle.setPosition(TLCorner.posX + (size + thick) * (idx % 8) + ((size + thick) * fullboard.board_size + 4) * move.w, TLCorner.posY + (size + thick) * (fullboard.board_size - 1 - (idx / 8)) + ((size + thick) * fullboard.board_size + 4) * (fullboard.z_size - 1 - move.z));
        window.draw(circle);
    }
}


int main() {
std::cout<<"here"<<std::endl;
    system("chcp 65001 > nul");
    /*
    Board* board0 = new Board(0, 0);
    Board* board1 = new Board(0x810000000ULL, 0x1008000000ULL);
    Board* board2 = new Board(0x1008000000ULL, 0x810000000ULL);
    BoardStack* stack1 = new BoardStack();
    BoardStack* stack2 = new BoardStack();
    BoardStack* stack3 = new BoardStack();
    BoardStack* stack4 = new BoardStack();
    stack1->z_axis.push_back(*board0);
    stack1->z_axis.push_back(*board0);
    stack1->z_axis.push_back(*board0);
    stack1->z_axis.push_back(*board0);
    stack2->z_axis.push_back(*board0);
    stack2->z_axis.push_back(*board1);
    stack2->z_axis.push_back(*board2);
    stack2->z_axis.push_back(*board0);
    stack3->z_axis.push_back(*board0);
    stack3->z_axis.push_back(*board2);
    stack3->z_axis.push_back(*board1);
    stack3->z_axis.push_back(*board0);
    stack4->z_axis.push_back(*board0);
    stack4->z_axis.push_back(*board0);
    stack4->z_axis.push_back(*board0);
    stack4->z_axis.push_back(*board0);

    std::vector<BoardStack> boardStack = { *stack1,*stack2,*stack3,*stack4 };
    FullBoard* fullboard = new FullBoard(8, 4, 4, boardStack);
    */

    Board* board0 = new Board(0, 0);
    Board* board1 = new Board(0x20400ULL, 0x40200ULL);
    Board* board2 = new Board(0x40200ULL, 0x20400ULL);

    BoardStack* stack1 = new BoardStack();
    BoardStack* stack2 = new BoardStack();
    BoardStack* stack3 = new BoardStack();
    BoardStack* stack4 = new BoardStack();
    stack1->z_axis.push_back(*board0);
    stack1->z_axis.push_back(*board0);
    stack1->z_axis.push_back(*board0);
    stack1->z_axis.push_back(*board0);
    stack2->z_axis.push_back(*board0);
    stack2->z_axis.push_back(*board1);
    stack2->z_axis.push_back(*board2);
    stack2->z_axis.push_back(*board0);
    stack3->z_axis.push_back(*board0);
    stack3->z_axis.push_back(*board2);
    stack3->z_axis.push_back(*board1);
    stack3->z_axis.push_back(*board0);
    stack4->z_axis.push_back(*board0);
    stack4->z_axis.push_back(*board0);
    stack4->z_axis.push_back(*board0);
    stack4->z_axis.push_back(*board0);

    std::vector<BoardStack> boardStack = { *stack1,*stack2,*stack3,*stack4 };
    FullBoard* fullboard = new FullBoard(4, 4, 4, boardStack);

    printFullBoard(*fullboard);

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "4D Othello");
    window.setFramerateLimit(60);

    float x, y;
    int size = 50;
    int thick = 3;
    int turn = 0;
    Reference TLCorner = { 50, 100 };
    std::vector<Move> moveList = movegen(*fullboard, turn);
    while (window.isOpen()) {

        window.clear();

        viewFullBoard(window, *fullboard, TLCorner, size, thick);
        if (moveList.empty()) {
            turn = 1 - turn;
            moveList = movegen(*fullboard, turn);
            if (moveList.empty()) {
                std::cout << "gameOver" << std::endl;
            }
        }
        viewMoves(window, *fullboard, TLCorner, size, thick, moveList, turn);

        window.display();

        sf::Vector2i pos = sf::Mouse::getPosition(window);
        x = pos.x;
        y = pos.y;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.key.code == sf::Mouse::Left) {
                    int l = (x - (TLCorner.posX - size / 2)) / ((size + thick) * fullboard->board_size + 4);
                    int i = ((x - (TLCorner.posX - size / 2)) / ((size + thick) * fullboard->board_size + 4) - l) * fullboard->board_size;
                    int k = (y - (TLCorner.posY - size / 2)) / ((size + thick) * fullboard->board_size + 4);
                    int j = fullboard->board_size - ((y - (TLCorner.posY - size / 2)) / ((size + thick) * fullboard->board_size + 4) - k) * fullboard->board_size;
                    k = fullboard->z_size - 1 - k;

                    U64 sq = 1ULL << (j * 8 + i);
                    for (Move& move : moveList) {
                        //std::cout<<"Move: 0x"<<std::hex<<move.move<<" z:"<<move.z<<" w:"<<move.w<<std::endl;
                        if (move.move == sq && move.z == k && move.w == l) {
                            makeMove(fullboard, move, turn);
                            window.clear();
                            viewFullBoard(window, *fullboard, TLCorner, size, thick);
                            window.display();
                            //greedyMoveMake(fullboard, 1 -turn);
                            depthMakeMove(fullboard, 4, 1-turn);
                            //turn=1-turn;
                            moveList = movegen(*fullboard, turn);
                            break;
                        }
                    }
                }
            }
        }

    }
    return 0;
}

