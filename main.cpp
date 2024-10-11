#include <iostream>
#include <vector>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <board.cpp>
#include <constants.h>

using namespace std;

//Globals
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
Board chess_board(SIZE, FONT_SIZE / 2, FONT_SIZE / 2);
bool app_is_running = true;
bool timer;
int last_frame_time;
int frame;
SDL_Rect moves_rect;
int max_scroll, scroll_value;
int start_time, black_time, white_time;

struct Sounds {
    Mix_Music* game_start;
    Mix_Music* game_end;
    Mix_Music* move;
    Mix_Music* move_check;
    Mix_Music* capture;
    Mix_Music* promote;
} sounds;

bool initializeMixer() {
    if (Mix_Init(0) != 0) {
        cout << stderr << "Error initializing SLD_Mixer\n";
        return false;
    }
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    sounds.game_start = Mix_LoadMUS(((string)SRC_PATH + "assets/sounds/game-start.mp3").c_str());
    sounds.game_end = Mix_LoadMUS(((string)SRC_PATH + "assets/sounds/game-end.mp3").c_str());
    sounds.move = Mix_LoadMUS(((string)SRC_PATH + "assets/sounds/move.mp3").c_str());
    sounds.move_check = Mix_LoadMUS(((string)SRC_PATH + "assets/sounds/move-check.mp3").c_str());
    sounds.capture = Mix_LoadMUS(((string)SRC_PATH + "assets/sounds/capture.mp3").c_str());
    sounds.promote = Mix_LoadMUS(((string)SRC_PATH + "assets/sounds/promote.mp3").c_str());

    return true;
}

bool initializeWindow() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        cout <<  stderr << "Error initializing SDL.\n";
        return false;
    }

    int h = chess_board.getSize() + 2 * chess_board.getEntity().y;
    int w = h * 16 / 9;
    window = SDL_CreateWindow(
        "Chess",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        w,
        h,
        0
    );

    if (!window) {
        cout << stderr << "Error creating SDL Window.\n";
        return false;
    }
    SDL_Surface* surface = IMG_Load(((string)SRC_PATH + "icon.png").c_str());
    SDL_SetWindowIcon(window, surface);

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        cout << stderr << "Error creating SDL Renderer\n";
        return false;
    }

    if (!initializeMixer()) {
        return false;
    }    

    if (TTF_Init() != 0) {
        cout << stderr << "Error initializing TTF\n";
        return false;
    }

    font = TTF_OpenFont(((string)SRC_PATH + "assets/fonts/UbuntuMono-B.ttf").c_str(), FONT_SIZE);
    if (!font) {
        cout << "Cannot find font file!\n";
        SDL_Quit();
        exit(1);
    }
    return true;
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type)
        {
        case SDL_QUIT:
            app_is_running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                app_is_running = false;
                break;
            case SDLK_r:
                chess_board.reset();
                Mix_PlayMusic(sounds.game_start, 1);
                timer = false;
                black_time = 0, white_time = 0;
                break;
            case SDLK_t:
                if (timer == false) { 
                    start_time = SDL_GetTicks();
                    timer = true; 
                }
            case SDLK_RIGHT:
                if (chess_board.getCurrentMove() < chess_board.getMoves().size()) {
                    Mix_PlayMusic(sounds.move, 1);
                }
                chess_board.fast_forward();
                break;
            case SDLK_LEFT:
                if (chess_board.getCurrentMove() > 0) {
                    Mix_PlayMusic(sounds.move, 1);
                }
                chess_board.rewind();
                break;
            default:
                break;
            }
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.clicks = 1) { 
                chess_board.check_mouse_hit(event.button.x, event.button.y);
            }
            break;
        case SDL_MOUSEWHEEL: 
            if (event.wheel.mouseX > moves_rect.x && event.wheel.mouseX < moves_rect.x + moves_rect.w && 
            event.wheel.mouseY > moves_rect.y && event.wheel.mouseY < moves_rect.y + moves_rect.h) {
                if ((scroll_value > 0 && event.wheel.y > 0) || 
                (scroll_value < max_scroll && event.wheel.y < 0)) {
                    scroll_value -= 4 * event.wheel.y;
                }
            }
            break;
        default:
            break;
        }
    }
}

void sleep_frame() {
    int frame_target_time = 1000 / FPS;
    int time_to_wait = frame_target_time - (SDL_GetTicks() - last_frame_time);

    if (time_to_wait > 0 && time_to_wait <= frame_target_time) {
        SDL_Delay(time_to_wait);
    }

    last_frame_time = SDL_GetTicks();
}

void update() {
    sleep_frame();
    if (chess_board.board_updated) {
        Move move = chess_board.getMoves().at(chess_board.getMoves().size() - 1);
        State state = chess_board.getState();
        if (state == NEUTRAL) {
            if (move.pawn_swapped == true && chess_board.is_pawn_swapping() == false) {
                Mix_PlayMusic(sounds.promote, 1);
            }
            else if (move.piece_captured) {
                Mix_PlayMusic(sounds.capture, 1);
            }
            else {
                Mix_PlayMusic(sounds.move, 1);
            }
        }
        else if (state == WHITE_CHECK || state == BLACK_CHECK) {
            Mix_PlayMusic(sounds.move_check, 1);
        }
        else if (state == WHITE_CHECKMATE || state == BLACK_CHECKMATE || state == TIE) {
            Mix_PlayMusic(sounds.game_end, 1);
        }
        chess_board.board_updated = false;
    }
}

void render_text(char* text, SDL_Color color, int x, int y, bool centered) {
    SDL_Surface *tmp = TTF_RenderText_Blended(font, text, color);
    SDL_Texture *label = SDL_CreateTextureFromSurface(renderer, tmp);
    SDL_Rect textRect = { x - centered * (tmp->w / 2), y, tmp->w, tmp->h };

    SDL_RenderCopy(renderer, label, NULL, &textRect);
    SDL_FreeSurface(tmp);
    SDL_DestroyTexture(label);
}

// Render the "past moves" display
void render_moves_display() {
    Entity board = chess_board.getEntity();
    int x, w;
    x = board.x + board.w + FONT_SIZE / 2;
    SDL_GetWindowSize(window, &w, NULL);
    w = (w - x) - FONT_SIZE / 2;
    moves_rect = { x, board.y + board.h / 4, w, board.h / 2 };
    SDL_Rect moves_barrier_rect = { x, 0, w, moves_rect.h / 2 + FONT_SIZE / 2 };
    SDL_SetRenderDrawColor(renderer, 60, 50, 40, 255);
    SDL_RenderFillRect(renderer, &moves_rect);
    SDL_Color white = { 255, 255, 255, 255 };
    vector<Move> moves = chess_board.getMoves();

    if (!moves.empty()) {
        for (int i = 0; i < moves.size(); i++) {
            auto& m = moves.at(i);
            int x = moves_rect.x;
            int y = moves_rect.y + FONT_SIZE * (i / 4) - scroll_value;
            if (i % 4 == 1) { x += moves_rect.w / 4; }
            else if (i % 4 == 2) { x += moves_rect.w / 2; }
            else if (i % 4 == 3) { x += 3 * moves_rect.w / 4; }
            
            if (i == chess_board.getCurrentMove() - 1) {
                SDL_Rect current_rect = { x, y, moves_rect.w / 4, FONT_SIZE };
                SDL_SetRenderDrawColor(renderer, 79, 200, 100, 255);
                SDL_RenderFillRect(renderer, &current_rect);
            }

            Entity ent = m.piece->getEntity();
            ent.x = x;
            ent.y = y;
            ent.w = FONT_SIZE;
            ent.h = FONT_SIZE;
            ent.render(renderer);

            string text ="  ->" + m.to;
            render_text(&text[0], white, x, y, false);

            if (i == moves.size() - 1 && ent.y + scroll_value + ent.h > moves_rect.y + moves_rect.h) {
                max_scroll = (ent.y + scroll_value + ent.h) - (moves_rect.y + moves_rect.h);
            }
        }
    }
    SDL_SetRenderDrawColor(renderer, 70, 60, 50, 255);
    SDL_RenderFillRect(renderer, &moves_barrier_rect);
    moves_barrier_rect.y = moves_rect.y + moves_rect.h;
    SDL_RenderFillRect(renderer, &moves_barrier_rect);
}

void render_timer() {
    int white_time_left = TIME - white_time; 
    int black_time_left = TIME - black_time;
    int minutes, seconds;

    int x, y, w, h;
    SDL_Color black = { 0, 0, 0, 255 };
    x = moves_rect.x + moves_rect.w / 3;
    y = moves_rect.y - moves_rect.h / 3;
    w = moves_rect.w / 3;
    h = moves_rect.h / 7;
    SDL_Rect timer_rect = { x, y, w, h };

    // Black timer
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &timer_rect);
    minutes = black_time_left / 60000;
    black_time_left = black_time_left % 60000;
    seconds = black_time_left / 1000;
    string black_time_string = to_string(minutes) + ":" + to_string(seconds);
    render_text(&black_time_string[0], black, x + timer_rect.w / 2, y, true);

    // White timer
    y = moves_rect.y + moves_rect.h - timer_rect.h + moves_rect.h / 3;
    timer_rect.y = y;
    SDL_RenderFillRect(renderer, &timer_rect);
    minutes = white_time_left / 60000;
    white_time_left = white_time_left % 60000;
    seconds = white_time_left / 1000;
    string white_time_string = to_string(minutes) + ":" + to_string(seconds);
    render_text(&white_time_string[0], black, x + timer_rect.w / 2, y, true);

    if (timer) {
        Color turn = chess_board.getTurn();
        if (turn == WHITE) {
            white_time = SDL_GetTicks() - start_time - black_time;
        }
        else if (turn == BLACK) {
            black_time = SDL_GetTicks() - start_time - white_time;
        }
    }

}

void render_pawn_swapper() {
    Entity ent = chess_board.getEntity();
    SDL_Rect rect = { ent.x + ent.w, ent.y + 2 * (ent.h / 8), ent.w / 8, 4 * (ent.h / 8) };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);
    for (const auto& p : chess_board.getPieceSelection()) {
        p->getEntity().render(renderer);
    }
}

void render_states() {
    TTF_SetFontSize(font, FONT_SIZE / 2);
    Entity ent = chess_board.getEntity();
    SDL_Rect state_rect { ent.x, 0, ent.w, FONT_SIZE / 2 };
    SDL_Color white = { 255, 255, 255, 255 };
    switch (chess_board.getState()) {
        case NEUTRAL:
            break;
        case WHITE_CHECK:
            state_rect.y += ent.h + FONT_SIZE / 2;
            SDL_SetRenderDrawColor(renderer, 200, 70, 70, 255);
            SDL_RenderFillRect(renderer, &state_rect);
            render_text("Check", white, state_rect.x + state_rect.w / 2, state_rect.y, true);
            break;
        case WHITE_CHECKMATE:
            SDL_SetRenderDrawColor(renderer, 79, 200, 100, 255);
            SDL_RenderFillRect(renderer, &state_rect);
            render_text("Winner!", white, state_rect.x + state_rect.w / 2, state_rect.y, true);
            state_rect.y += ent.h + FONT_SIZE / 2;
            SDL_SetRenderDrawColor(renderer, 200, 70, 70, 255);
            SDL_RenderFillRect(renderer, &state_rect);
            render_text("Checkmate", white, state_rect.x + state_rect.w / 2, state_rect.y, true);
            break;
        case BLACK_CHECK:
            SDL_SetRenderDrawColor(renderer, 200, 70, 70, 255);
            SDL_RenderFillRect(renderer, &state_rect);
            render_text("Check", white, state_rect.x + state_rect.w / 2, state_rect.y, true);
            break;
        case BLACK_CHECKMATE:
            SDL_SetRenderDrawColor(renderer, 200, 70, 70, 255);
            SDL_RenderFillRect(renderer, &state_rect);
            render_text("Checkmate", white, state_rect.x + state_rect.w / 2, state_rect.y, true);
            state_rect.y += ent.h + FONT_SIZE / 2;
            SDL_SetRenderDrawColor(renderer, 79, 200, 100, 255);
            SDL_RenderFillRect(renderer, &state_rect);
            render_text("Winner!", white, state_rect.x + state_rect.w / 2, state_rect.y, true);
            break;
        case TIE:
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            SDL_RenderFillRect(renderer, &state_rect);
            render_text("Tie", white, state_rect.x + state_rect.w / 2, state_rect.y, true);
            state_rect.y += ent.h + FONT_SIZE / 2;
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            SDL_RenderFillRect(renderer, &state_rect);
            render_text("Tie", white, state_rect.x + state_rect.w / 2, state_rect.y, true);
            break;
        default:
            break;
    }
    TTF_SetFontSize(font, FONT_SIZE);
}

void render_move_animation() {
    if (chess_board.animation != NULL) {
        Entity* last_move = chess_board.getLastMove();
        if (frame <= 30) {
            int x = last_move[0].x + (last_move[1].x - last_move[0].x) * (frame / 30.0);
            int y = last_move[0].y + (last_move[1].y - last_move[0].y) * (frame / 30.0);
            chess_board.animation->set_position(x, y);
            chess_board.animation->getEntity().render(renderer);
            frame++;
        }
        else {
            chess_board.animation = NULL;
            frame = 0;
        }
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 70, 60, 50, 255);
    SDL_RenderClear(renderer);
    
    chess_board.getEntity().render(renderer);

    render_moves_display();

    render_states();

    render_timer();

    chess_board.getLastMove()[0].render(renderer);
    chess_board.getLastMove()[1].render(renderer);

    if (chess_board.getSelectedPiece() != NULL) {
        chess_board.getSelectedField().render(renderer);
    }

    for (const auto& p : chess_board.getPieces()) {
        p->getEntity().render(renderer);
    }

    render_move_animation();
    
    if (chess_board.is_pawn_swapping()) {
        render_pawn_swapper();
    }
    
    SDL_RenderPresent(renderer);
}

void cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    Mix_CloseAudio();
    SDL_Quit();
}

int main() {
    
    initializeWindow();
    chess_board.reset();
    Mix_PlayMusic(sounds.game_start, 1);
    
    while (app_is_running) {
        process_input();
        update();
        render();
    }

    cleanup();

    return 1;
}
