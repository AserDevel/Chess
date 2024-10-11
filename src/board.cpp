#include <iostream>
#include <string>
#include <vector>
#include <bits/stdc++.h>

#include <pieces.cpp>

using namespace std;

typedef struct {
    Piece* piece;
    string from, to;
    bool piece_captured;
    bool pawn_swapped;
} Move;

enum State { 
    NEUTRAL, 
    WHITE_CHECK, 
    BLACK_CHECK, 
    WHITE_CHECKMATE, 
    BLACK_CHECKMATE, 
    TIE
};

class Board {
    private:
        int size;
        Entity entity;
        Piece* sel_piece;
        Entity sel_field;
        int current_move;
        vector<Move> moves;
        Entity last_move[2];
        vector<string> valid_fields;
        vector <Piece*> pieces;
        vector<Piece*> captured_pieces;
        vector<Piece*> swapped_pieces;
        vector<Piece*> swap_selection;
        bool pawn_swapping;
        Piece* white_king;
        Piece* black_king;
        Color turn;
        State state;

    public:
        Board(int size, int x, int y) {
            this->size = size - size % 8;
            string fileName = "chess_board.png";
            this->entity = Entity(x, y, this->size, this->size, fileName);
            this->sel_field = Entity(x, y, this->size / 8, this->size / 8, "selected_field.png");
            this->last_move[0] = Entity(-10000, -10000, this->size / 8, this->size / 8, "previous_field.png");
            this->last_move[1] = Entity(-10000, -10000, this->size / 8, this->size / 8, "previous_field.png");
        }

        int getSize() { return size; }
        Piece* getSelectedPiece() { return sel_piece; }
        Entity getSelectedField() { return sel_field; }
        bool is_pawn_swapping() { return pawn_swapping; }
        State getState() { return state; }
        Entity* getLastMove() { return last_move; }
        Entity getEntity() { return entity; }
        vector<Piece*> getPieces() { return pieces; }
        vector<Piece*> getPieceSelection() { return swap_selection; }
        vector<Move> getMoves() { return moves; }
        int getCurrentMove() { return current_move; }
        Color getTurn() { return turn; }
        Piece* animation;
        bool board_updated;

        void reset() {
            pieces.clear();
            captured_pieces.clear();
            swap_selection.clear();
            swapped_pieces.clear();
            moves.clear();
            state = NEUTRAL;
            current_move = 0;
            turn = WHITE;
            last_move[0].x = -10000;
            last_move[1].x = -10000;
            //white side
            for (int i = 0; i < 8; i++) {
                char field[2] = {i+65, 50};
                pieces.push_back(new Pawn(size, entity.x, entity.y, field, WHITE));
            }
            pieces.push_back(new Rook(size, entity.x, entity.y, "A1", WHITE));
            pieces.push_back(new Rook(size, entity.x, entity.y, "H1", WHITE));
            pieces.push_back(new Knight(size, entity.x, entity.y, "B1", WHITE));
            pieces.push_back(new Knight(size, entity.x, entity.y, "G1", WHITE));
            pieces.push_back(new Bishop(size, entity.x, entity.y, "C1", WHITE));
            pieces.push_back(new Bishop(size, entity.x, entity.y, "F1", WHITE));
            pieces.push_back(new Queen(size, entity.x, entity.y, "D1", WHITE));
            white_king = new King(size, entity.x, entity.y, "E1", WHITE);
            pieces.push_back(white_king);

            //black side
            for (int i = 0; i < 8; i++) {
                char field[2] = {i+65, 55};
                pieces.push_back(new Pawn(size, entity.x, entity.y, field, BLACK));
            }
            pieces.push_back(new Rook(size, entity.x, entity.y, "A8", BLACK));
            pieces.push_back(new Rook(size, entity.x, entity.y, "H8", BLACK));
            pieces.push_back(new Knight(size, entity.x, entity.y, "B8", BLACK));
            pieces.push_back(new Knight(size, entity.x, entity.y, "G8", BLACK));
            pieces.push_back(new Bishop(size, entity.x, entity.y, "C8", BLACK));
            pieces.push_back(new Bishop(size, entity.x, entity.y, "F8", BLACK));
            pieces.push_back(new Queen(size, entity.x, entity.y, "D8", BLACK));
            black_king = new King(size, entity.x, entity.y, "E8", BLACK);
            pieces.push_back(black_king);

            for (auto& p : pieces) {
                p->update_target_fields(pieces);
            }
        }

        // Checks which field the mouse clicked, and updates the selected piece
        void check_mouse_hit(int x, int y) {
            char field[2];
            for (int i = 0; i < 9; i++) {
                if (x >= this->entity.x + (size / 8) * i && x < this->entity.x + (size / 8) * (i + 1)) {
                    field[0] = 65 + i;
                }
                if (y < this->entity.y + (size / 8) * (8 - i) && y >= this->entity.y + (size / 8) * (7 - i)) {
                    field[1] = 49 + i;
                }
            }
            if (pawn_swapping) {
                check_swap_hit(field);
            }
            else if (current_move == moves.size()) {
                field_update(field);
            }
            else { sel_piece = NULL; }
        }

        // Rewind one move
        void rewind() {
            if (!moves.empty() && current_move > 0) {
                Move move = moves.at(current_move - 1);
                update_last_move(move.from, move.to);
                move.piece->update_field(move.from);
                move.piece->update_position();
                if (move.piece_captured) {
                    pieces.push_back(captured_pieces.back());
                    captured_pieces.pop_back();
                }
                if (move.pawn_swapped) {
                    pieces.insert(pieces.begin(), move.piece);
                    auto it = find(pieces.begin(), pieces.end(), swapped_pieces.back());
                    pieces.erase(it);                    
                    swapped_pieces.insert(swapped_pieces.begin(), swapped_pieces.back());
                    swapped_pieces.pop_back();
                }
                current_move--;
                if (current_move > 0) { move = moves.at(current_move - 1); }
                update_last_move(move.from, move.to);
            }
        }

        // Fast forward one move
        void fast_forward() {
            if (!moves.empty() && current_move < moves.size()) {
                Move move = moves.at(current_move);
                update_last_move(move.from, move.to);
                if (move.piece_captured) {
                    for (int i = 0; i < pieces.size(); i++) {
                        if (pieces.at(i)->getField() == move.to) {
                            captured_pieces.push_back(pieces.at(i));
                            pieces.erase(pieces.begin() + i);
                            break;
                        }   
                    }
                }
                move.piece->update_field(move.to);
                move.piece->update_position();
                if (move.pawn_swapped) {
                    pieces.insert(pieces.begin(), swapped_pieces.front());
                    auto it = find(pieces.begin(), pieces.end(), move.piece);
                    pieces.erase(it);
                    swapped_pieces.push_back(swapped_pieces.front());
                    swapped_pieces.erase(swapped_pieces.begin());                 
                }
                update_last_move(move.from, move.to);
                current_move++;
            }
        }
            
    private:
        // Attemps to move the selected piece to the new field.
        void field_update(string new_field) {
            bool field_updated = false;
            if (sel_piece != NULL) { // Update the field of the selected piece if valid
                for (string valid_field : valid_fields) {
                    if (new_field == valid_field) {
                        field_updated = true;
                        string saved_field = sel_piece->getField();
                        sel_piece->update_field(new_field);

                        if (sel_piece->isPawn()) { // Check if there's a pawn swap
                            if ((sel_piece->getColor() == WHITE && sel_piece->getField()[1] == '8') ||
                                (sel_piece->getColor() == BLACK && sel_piece->getField()[1] == '1')) {
                                pawn_swapping = true;
                                generate_swap_selection(sel_piece->getColor());
                            }
                        }
                        bool piece_captured = false;
                        int i = pieces.size() - 1;  
                        for (; i >= 0; i--) { // Check if a piece was captured
                            auto& p = pieces[i];  
                            if (new_field == p->getField() && turn != p->getColor()) {
                                captured_pieces.push_back(p);
                                pieces.erase(pieces.begin() + i);
                                piece_captured = true;
                                break;
                            }
                        }
                        for (auto& p : pieces) {
                            p->update_target_fields(pieces);
                        }
                        Move last_move = { sel_piece, saved_field, new_field, piece_captured, pawn_swapping };
                        update_last_move(last_move.from, last_move.to);
                        moves.push_back(last_move);
                        current_move++;
                        check_board_state();
                        animation = sel_piece;
                        board_updated = true;
                        if (turn == WHITE) { turn = BLACK; }
                        else { turn = WHITE; }
                        break;
                    }
                }
                sel_piece = NULL; // Deselect the piece
            }
            if (!field_updated) { field_select(new_field); }
        }

        // Selects a piece on the given field if valid, otherwise deselect
        void field_select(string new_field) {
            for (auto& p : pieces) {
                if (new_field == p->getField() && turn == p->getColor()) {
                    sel_piece = p;
                    valid_fields.clear();
                    for (auto& f : p->getTargetFields()) {
                        if (validate_field(p, f)) { valid_fields.push_back(f); }
                    }
                    sel_field.x = p->getEntity().x;
                    sel_field.y = p->getEntity().y;
                    break;
                } 
                else {
                    sel_piece = NULL;
                }
            }
        }

        // generates the GUI for the swap selection
        void generate_swap_selection(Color color) {
            swap_selection.clear();

            swap_selection.push_back(new Queen(size, entity.x, entity.y, "I6", color));
            swap_selection.push_back(new Rook(size, entity.x, entity.y, "I5", color));
            swap_selection.push_back(new Knight(size, entity.x, entity.y, "I4", color));
            swap_selection.push_back(new Bishop(size, entity.x, entity.y, "I3", color));
        }

        void check_swap_hit(string field) {
            for (auto& new_piece : swap_selection) {
                if (field == new_piece->getField()) { 
                    pawn_swapping = false;
                    Piece* pawn = moves.back().piece;
                    new_piece->update_field(pawn->getField());
                    new_piece->update_position();
                    auto it = find(pieces.begin(), pieces.end(), pawn);
                    pieces.erase(it);
                    swapped_pieces.push_back(new_piece);
                    pieces.insert(pieces.begin(), new_piece);

                    for (auto& p : pieces) {
                        p->update_target_fields(pieces);
                    }
                    check_board_state();
                    board_updated = true;
                }
            }
        }

        bool is_check(Piece* king) {
            bool check = false;
            for (const auto& p : pieces) {
                if (p->getColor() != king->getColor()) {
                    for (const auto& f : p->getTargetFields()) {
                        if (f == king->getField()) {
                            check = true;
                        }
                    }
                }
            }
            return check;
        }

        // Checks if the given color has a valid move
        bool valid_moves(Color color) {
            for (auto& p : pieces) {
                if (color == p->getColor()) {
                    for (auto& new_field : p->getTargetFields()) {
                        if (validate_field(p, new_field)) { 
                            return true;                        
                        }
                    }
                }
            }
            return false;
        }

        // Moves a piece, and checks if it's valid, then undo's the change
        bool validate_field(Piece* piece, string new_field) {
            bool valid = true;
            string saved_field = piece->getField();
            piece->update_field(new_field);
            bool piece_was_captured = false;
            int i = pieces.size() - 1;  
            for (; i >= 0; i--) { // Check if a piece was captured
                auto& p = pieces[i];  
                if (new_field == p->getField() && piece->getColor() != p->getColor()) {
                    piece_was_captured = true;
                    captured_pieces.push_back(p);
                    pieces.erase(pieces.begin() + i);
                    break;
                }
            } 
            for (auto& p : pieces) {
                p->update_target_fields(pieces);
            }
            if ((piece->getColor() == WHITE && is_check(white_king)) || 
            (piece->getColor() == BLACK && is_check(black_king))) {
                valid = false;
            }
            piece->update_field(saved_field); //undo the change
            if (piece_was_captured) { // Restore the potentially captured piece
                pieces.insert(pieces.begin() + i, captured_pieces.back());
                captured_pieces.pop_back();
            }
            for (auto& p : pieces) {
                p->update_target_fields(pieces);        
            }
            return valid;
        }

        // check the board current state
        void check_board_state() {
            bool check = is_check(black_king);
            bool valid_move = valid_moves(BLACK);
            if (check && valid_move) {
                state = BLACK_CHECK;
                return;
            }
            else if (check && !valid_move) {
                state = BLACK_CHECKMATE;
                return;
            }
            else if (!check && !valid_move) {
                state = TIE;
                return;
            } 
        
            check = is_check(white_king);
            valid_move = valid_moves(WHITE);
            if (check && valid_move) {
                state = WHITE_CHECK;
                return;
            }
            else if (check && !valid_move) {
                state = WHITE_CHECKMATE;
                return;
            }
            else if (!check && !valid_move) {
                state = TIE;
                return;
            } 
            state = NEUTRAL;
            return;
        }

        void update_last_move(string from, string to) {
            last_move[0].x = entity.x + last_move[0].w * (from[0] - 65);
            last_move[0].y = entity.y - last_move[0].h * (from[1] - 56);
            last_move[1].x = entity.x + last_move[1].w * (to[0] - 65);
            last_move[1].y = entity.y - last_move[1].h * (to[1] - 56);
        }
};
