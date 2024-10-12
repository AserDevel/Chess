#include <iostream>
#include <string>
#include <vector>

#include <entity.cpp>

using namespace std;

enum Color { BLACK, WHITE };

// Base class for all chess pieces
class Piece {
    protected:
        bool pawn = false;
        int board_size, board_posX, board_posY;
        Entity entity;
        Color color;
        string field;
        vector<string> target_fields;

        // check the field based on the field position relative to current position
        void update_relative_field(int x, int y, vector<Piece*> pieces) {
            string new_field = {char(field[0] + x), char(field[1] + y)};
            if (new_field[0] < 65 || new_field[0] > 72 || new_field[1] < 49 || new_field[1] > 56) {
                return;
            }
            for (const auto& p : pieces) {
                if (new_field == p->getField() && color == p->getColor()) {
                    return;
                }
            }
            target_fields.push_back(new_field);
        }

        // Updates the field position by going in one direction until hits another piece.
        // param x and y should always be 1 or -1.
        void update_linear_fields(int x, int y, vector<Piece*> pieces) {
        string new_field;
        bool flag = true;
        int i = 1;
        while (flag) {
            new_field = {char(field[0] + x*i), char(field[1] + y*i)};
            if (new_field[0] < 65 || new_field[0] > 72 || new_field[1] < 49 || new_field[1] > 56) {
                break;
            }
            for (const auto& p : pieces) {
                if (new_field == p->getField()) {
                    flag = false;
                    if (color != p->getColor()) {
                        target_fields.push_back(new_field);
                    }
                    break;
                }
            }
            if (flag) { 
                target_fields.push_back(new_field); 
                i++;
            }
        }
    }

    public:
        // public getters and setters
        bool isPawn() { return pawn; }
        string getField() { return field; }
        Color getColor() { return color; }
        Entity getEntity() { return entity; }
        vector<string> getTargetFields() { return target_fields; }

        // Takes a container of pieces, and checks which fields this piece can target based on it's movement rules
        virtual void  update_target_fields(vector<Piece*> pieces) { 
            // Base class polymorphic function
        }

        // Virtual destructor
        virtual ~Piece() {}

        // updates the x,y coordinate. Used for the animation function
        void set_position(int x, int y) {
            this->entity.x = x;
            this->entity.y = y;
        }

        // Update position based on this piece current field
        void update_position() {
            entity.x = board_posX + entity.w * (this->field[0] - 65);
            entity.y = board_posY + board_size - entity.h * (this->field[1] - 48);
        }

        // Update this field based on the given field.
        void update_field(string field_str) {
            bool validX;
            bool validY;
            for (int i = 0; i < 8; i++) {
                if (field_str[0] == i+65 || field_str[0] == 73) { validX = true; }
                if (field_str[1] == i+49) { validY = true; }
            }
            if (!validX || !validY || field_str.length() != 2) {
                cout << "Invalid field: " << &field_str[0] << endl;
            }
            else {
                field = field_str;
            }
        }
};

// Class to represent the pawn piece
class Pawn : public Piece {
    public:
        Pawn(int board_size, int board_posX, int board_posY, string field_str, Color color) {
            this->board_size = board_size;
            this->board_posX = board_posX;
            this->board_posY = board_posY;
            this->color = color;
            this->pawn = true;
            string fileName;
            if (color == WHITE) { fileName = "white_pawn.png"; }
            else if (color == BLACK) { fileName = "black_pawn.png"; }
            this->entity = Entity(0, 0, board_size/8, board_size/8, fileName);
            update_field(field_str);
            update_position();
        }

        // This piece has the most complicated movement 
        // It's also the only piece that has different movement based on its color
        void update_target_fields(vector<Piece*> pieces) override {
            target_fields.clear();

            // Check vertical fields
            if (color == WHITE) {
                if (field[1] == '8') { return; } //return if end is reached 
                target_fields.push_back({field[0], char(field[1] + 1)});
                if (field[1] == '2') {
                    target_fields.push_back({field[0], char(field[1] + 2)});
                }
            }
            else if (color == BLACK) {
                if (field[1] == '1') { return; }
                target_fields.push_back({field[0], char(field[1] - 1)});
                if (field[1] == '7') {
                    target_fields.push_back({field[0], char(field[1] - 2)});
                }
            }
            for (const auto& p : pieces) {
                if (target_fields.at(0) == p->getField()) {
                    target_fields.clear();
                    break;
                }
                else if (target_fields.size() == 2 && target_fields.at(1) == p->getField()) {
                    target_fields.pop_back();
                }
            }
            // Check diagonal fields
            if (color == WHITE) {
                for (const auto& p : pieces) {
                    if (p->getColor() == BLACK && p->getField()[1] == field[1] + 1 &&
                    (p->getField()[0] == field[0] + 1 || p->getField()[0] == field[0] - 1)) {
                        target_fields.push_back(p->getField());
                    }
                }
            }
            else if (color == BLACK) {
                for (const auto& p : pieces) {
                    if (p->getColor() == WHITE && p->getField()[1] == field[1] - 1 &&
                    (p->getField()[0] == field[0] + 1 || p->getField()[0] == field[0] - 1)) {
                        target_fields.push_back(p->getField());
                    }
                }
            }
        }
};

// Class to represent the rook piece
class Rook : public Piece {
    public:
        Rook(int board_size, int board_posX, int board_posY, string field_str, Color color) {
            this->board_size = board_size;
            this->board_posX = board_posX;
            this->board_posY = board_posY;
            this->color = color;
            string fileName;
            if (color == WHITE) { fileName = "white_rook.png"; }
            else if (color == BLACK) { fileName = "black_rook.png"; }
            this->entity = Entity(0, 0, board_size/8, board_size/8, fileName);
            update_field(field_str);
            update_position();
    }

    void update_target_fields(vector<Piece*> pieces) override {
        target_fields.clear();

        update_linear_fields(0, 1, pieces);
        update_linear_fields(0, -1, pieces);
        update_linear_fields(1, 0, pieces);
        update_linear_fields(-1, 0, pieces);
    }
};

// Class to represent the knight piece
class Knight : public Piece {
    public:
        Knight(int board_size, int board_posX, int board_posY, string field_str, Color color) {
            this->board_size = board_size;
            this->board_posX = board_posX;
            this->board_posY = board_posY;
            this->color = color;
            string fileName;
            if (color == WHITE) { fileName = "white_knight.png"; }
            else if (color == BLACK) { fileName = "black_knight.png"; }
            this->entity = Entity(0, 0, board_size/8, board_size/8, fileName);
            update_field(field_str);
            update_position();
    }

    void update_target_fields(vector<Piece*> pieces) override {
        target_fields.clear();

        update_relative_field(1, 2, pieces);
        update_relative_field(-1, 2, pieces);
        update_relative_field(1, -2, pieces);
        update_relative_field(-1, -2, pieces);
        update_relative_field(2, 1, pieces);
        update_relative_field(2, -1, pieces);
        update_relative_field(-2, 1, pieces);
        update_relative_field(-2, -1, pieces);
    }
};

// Class to represent the bishop piece
class Bishop : public Piece {
    public:
        Bishop(int board_size, int board_posX, int board_posY, string field_str, Color color) {
            this->board_size = board_size;
            this->board_posX = board_posX;
            this->board_posY = board_posY;
            this->color = color;
            string fileName;
            if (color == WHITE) { fileName = "white_bishop.png"; }
            else if (color == BLACK) { fileName = "black_bishop.png"; }
            this->entity = Entity(0, 0, board_size/8, board_size/8, fileName);
            update_field(field_str);
            update_position();
    }

    void update_target_fields(vector<Piece*> pieces) override {
        target_fields.clear();

        update_linear_fields(1, 1, pieces);
        update_linear_fields(1, -1, pieces);
        update_linear_fields(-1, 1, pieces);
        update_linear_fields(-1, -1, pieces);
    }
};

// Class to represent the queen piece
class Queen : public Piece {
    public:
        Queen(int board_size, int board_posX, int board_posY, string field_str, Color color) {
            this->board_size = board_size;
            this->board_posX = board_posX;
            this->board_posY = board_posY;
            this->color = color;
            string fileName;
            if (color == WHITE) { fileName = "white_queen.png"; }
            else if (color == BLACK) { fileName = "black_queen.png"; }
            this->entity = Entity(0, 0, board_size/8, board_size/8, fileName);
            update_field(field_str);
            update_position();
    }

    void update_target_fields(vector<Piece*> pieces) override {
        target_fields.clear();

        update_linear_fields(0, 1, pieces);
        update_linear_fields(0, -1, pieces);
        update_linear_fields(1, 0, pieces);
        update_linear_fields(-1, 0, pieces);
        update_linear_fields(1, 1, pieces);
        update_linear_fields(1, -1, pieces);
        update_linear_fields(-1, 1, pieces);
        update_linear_fields(-1, -1, pieces);
    }
};

// Class to represent the king piece
class King : public Piece {
    public:
        King(int board_size, int board_posX, int board_posY, string field_str, Color color) {
            this->board_size = board_size;
            this->board_posX = board_posX;
            this->board_posY = board_posY;
            this->color = color;
            string fileName;
            if (color == WHITE) { fileName = "white_king.png"; }
            else if (color == BLACK) { fileName = "black_king.png"; }
            this->entity = Entity(0, 0, board_size/8, board_size/8, fileName);
            update_field(field_str);
            update_position();
    }

    void update_target_fields(vector<Piece*> pieces) override {
        target_fields.clear();

        update_relative_field(1, 0, pieces);
        update_relative_field(0, 1, pieces);
        update_relative_field(-1, 0, pieces);
        update_relative_field(0, -1, pieces);
        update_relative_field(1, 1, pieces);
        update_relative_field(1, -1, pieces);
        update_relative_field(-1, 1, pieces);
        update_relative_field(-1, -1, pieces);
    }
};