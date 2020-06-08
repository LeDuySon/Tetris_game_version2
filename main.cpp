#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <ctime>
#include <string>
#include <vector>
#include <unistd.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
//include header
#include "SDL_utils.h"
#include "handreg.h"
using namespace std;

SDL_Color fontColor = {255, 255, 255, 255};


int CalPoint = 0;
int sum_lines = 0;
// ON OFF MUSIC
bool MUSIC = false;
// Set logic game
bool GAME_OVER = false;
//bool INIT = true;
bool isRestart = false;
bool isPause = false;
bool isPlay = false;
bool isMain = true;
bool isSubmenu = false;
bool END_GAME = false;
bool mergeShadow = false;

int fps = 0;
int maps[M][N] = {0};
SDL_Rect block[4];
SDL_Rect nextblock[4];
int shapes[7][4] =
{
    1,3,5,7, // I
    2,4,5,7, // Z
    3,5,4,6, // S
    3,5,4,7, // T
    2,3,5,7, // L
    3,5,7,6, // J
    2,3,4,5, // O
};

struct Points{
    int x, y;
    };

class Queue{
private:
    int queue_size;
    int first = 0;
    int last  = 0;
    int* buffer;
public:
    Queue(int n){
        queue_size = n;
        buffer = new int[n];
    }
    void enqueue(int a){
        if(last < queue_size) buffer[last++] = a;
        else if(manageQ()) buffer[last++] = a;
        else{
            cout << "queue is full" << endl;

        }
    }
    int dequeue(){
        if(first < last) return buffer[first++];
        else{
            cout << "Queue is empty" << endl;
            return -1;
        }
    }
    int sizeQ(){
        return last;
    }

    int top(){
        return buffer[last-1];
    }
    int bottom(){
        return buffer[0];
    }
    bool manageQ(){
        if(first == 0){
            return false;
        }else{
            for(int i =0; i < last - first; i++){
                buffer[i] = buffer[i+ first];
            }
            last = last - first;
            first = 0;
            return true;
        }
        }
    };
Queue shape(2), colors(2);

//Store Point
Queue pointSTR(30);
bool valid(){

    for(int i = 0; i < 4; i++){

        if(block[i].x  > (SCREEN_WIDTH - b_w) || block[i].y+b_h > SCREEN_HEIGHT || block[i].x < 0 || block[i].y < 0){
            return false;
        }else if(maps[(block[i].y)/b_h][(block[i].x)/b_w]){

            return false;
            }
    }
    return true;

}

void initblock(SDL_Renderer* renderer,SDL_Texture *image, SDL_Rect &crop, const int& iw, const int& ih, const int& n, const int& color){
    for(int i =0; i < 4; i++){
        block[i].x = (shapes[n][i]/2)*b_w + SCREEN_WIDTH/2;
        block[i].y = (shapes[n][i]%2)*b_h;
        block[i].w = b_w;
        block[i].h = b_h;
        crop.x = (color-1)*18;
        crop.y = 0;
        crop.w = iw/8+2;
        crop.h = ih;
        SDL_RenderCopy(renderer, image, &crop, &block[i]);
    }
    if(!valid()){
//        SDL_RenderClear(renderer);
        GAME_OVER = true;
}



}

//next block
void next_block(SDL_Renderer* renderer,SDL_Texture *image, SDL_Rect &crop, const int& iw, const int& ih,const int& n, const int& color){
    for(int i =0; i < 4; i++){
        nextblock[i].x = (shapes[n][i]/2)*(b_w+10) + 620;
        nextblock[i].y = (shapes[n][i]%2)*(b_h+10) + 560;
        nextblock[i].w = b_w+10;
        nextblock[i].h = b_h+10;
        crop.x = (color-1)*18;
        crop.y = 0;
        // + them 2 cho do~ bi lech block :D
        crop.w = iw/8+2;
        crop.h = ih;
        SDL_RenderCopy(renderer, image, &crop, &nextblock[i]);
    }


}

void createBlock(SDL_Renderer* renderer,SDL_Texture *image, SDL_Rect &crop_c,SDL_Rect &crop_n, const int& iw, const int& ih){
    int n=rand()%7;
    int color = rand()% 7 + 1;
    shape.enqueue(n);
    colors.enqueue(color);
    initblock(renderer, image, crop_c, iw, ih, shape.dequeue(), colors.dequeue());
    next_block(renderer, image, crop_n, iw, ih, shape.top(), colors.top());
}
void checkgame_over(){
        for(int j = 0; j < N; j++){
            if(maps[0][j]) GAME_OVER = true;
        }
}

void show_gameover(SDL_Renderer* renderer, SDL_Texture *gameover, const int& gov_w, const int& gov_h){
    renderTexture(gameover, renderer, 40, 48, gov_w, gov_h);


}


void draw_shadow(SDL_Renderer* renderer, SDL_Rect &crop, SDL_Texture *image){
    int shadow[4];
    for(int b = 0; b < 4; b++){
        shadow[b] = block[b].y;
    }

    while(valid()){
        for(int i = 0; i < 4; i++){
            block[i].y += b_h;
        }
    }

    if(mergeShadow){
        for(int i = 0; i < 4; i++){
            block[i].y -= b_h;
            SDL_RenderCopy(renderer, image, &crop, &block[i]);
        }
        mergeShadow = false;
    }else{
        for(int i = 0; i < 4; i++){
            block[i].y -= b_h;
            SDL_SetRenderDrawColor(renderer, 255, 0, 255, 0); // purple
            SDL_RenderDrawRect(renderer, &block[i]);
            block[i].y = shadow[i];
    }

    }


}

//game menu
void game_menu(SDL_Texture *menu, SDL_Renderer* renderer,SDL_Texture *direct, SDL_Texture *music_off, SDL_Rect cropDR[], SDL_Rect blockDR[], const int& margin_top, const int& margin_l, const int& b_w, const int& b_h){
    // distance between button
    float dist = 24.5;
    int x, y;
    SDL_Event e;
    while(true){
        while( SDL_PollEvent(&e) ){
                SDL_RenderClear(renderer);
                // render on or off music  !!
                if(MUSIC){
                    renderTexture(menu, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                }else{
                renderTexture(music_off, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                }
                switch( e.type ){
                    case SDL_MOUSEMOTION:
                        x = e.motion.x;
                        y = e.motion.y;
                        if(y >= margin_top && y <= (margin_top+b_h) && x >= margin_l && x <= margin_l + b_w){
                            for(int i = 0; i < 2; i++){
                                cropDR[i].y = margin_top ;
                                SDL_RenderCopy(renderer, direct, &blockDR[i], &cropDR[i]);
                            }
                        }else if(y >= (margin_top + b_h + dist) && y <= (margin_top+ 2 * b_h + dist) && x >= margin_l && x <= margin_l + b_w){
                            for(int i = 0; i < 2; i++){
                                cropDR[i].y = margin_top + b_h + dist;
                                SDL_RenderCopy(renderer, direct, &blockDR[i], &cropDR[i]);

                            }
                        }else if(y >= (margin_top + 2 * b_h + 2 * dist) && y <= (margin_top+ 3*b_h + 2*dist) && x >= margin_l && x <= margin_l + b_w){
                            for(int i = 0; i < 2; i++){
                                cropDR[i].y = margin_top + 2*b_h + 2*dist;
                                SDL_RenderCopy(renderer, direct, &blockDR[i], &cropDR[i]);
                            }
                        }
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        x = e.motion.x;
                        y = e.motion.y;
                        isSubmenu = false;
                        if(y >= margin_top && y <= (margin_top+b_h) && x >= margin_l && x <= margin_l + b_w){
                            SDL_RenderClear(renderer);
                            isPlay = true;
                            isRestart = true;
                            return;
                        }else if(y >= (margin_top + b_h + dist) && y <= (margin_top+ 2 * b_h + dist) && x >= margin_l && x <= margin_l + b_w){
                            SDL_RenderClear(renderer);
                            return;
                        }else if(y >= (margin_top + 2 * b_h + 2 * dist) && y <= (margin_top+ 3*b_h + 2*dist) && x >= margin_l && x <= margin_l + b_w){
                            return;
                        }else if(x >= 400 && x <= 430 && y >= 580 && y <= 625){
                            SDL_RenderClear(renderer);
                            renderTexture(menu, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                            MUSIC = true;
                        }else if(x >= 438 && x <= 475 && y >= 580 && y <= 625){
                            SDL_RenderClear(renderer);
                            renderTexture(music_off, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                            MUSIC = false;

                        }
                        break;

                }
                SDL_RenderPresent(renderer);

        }
    }
}
//compute point
void sub_menu(SDL_Texture *submenu, SDL_Renderer* renderer,SDL_Texture *direct, SDL_Rect cropDR[], SDL_Rect blockDR[], const int& margin_top, const int& margin_l, const int& b_w, const int& b_h){
    float dist = 24.5;
    int x, y;
    SDL_Event e;
    while(true){
        while( SDL_PollEvent(&e) ){
                SDL_RenderClear(renderer);
                renderTexture(submenu, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                // render on or off music  !!
                switch( e.type ){
                    case SDL_MOUSEMOTION:
                        x = e.motion.x;
                        y = e.motion.y;
                        if(y >= margin_top && y <= (margin_top+b_h) && x >= margin_l && x <= margin_l + b_w){
                            for(int i = 0; i < 2; i++){
                                cropDR[i].y = margin_top ;
                                SDL_RenderCopy(renderer, direct, &blockDR[i], &cropDR[i]);
                            }
                        }else if(y >= (margin_top + b_h + dist) && y <= (margin_top+ 2 * b_h + dist) && x >= margin_l && x <= margin_l + b_w){
                            for(int i = 0; i < 2; i++){
                                cropDR[i].y = margin_top + b_h + dist;
                                SDL_RenderCopy(renderer, direct, &blockDR[i], &cropDR[i]);

                            }
                        }else if(y >= (margin_top + 2 * b_h + 2 * dist) && y <= (margin_top+ 3*b_h + 2*dist) && x >= margin_l && x <= margin_l + b_w){
                            for(int i = 0; i < 2; i++){
                                cropDR[i].y = margin_top + 2*b_h + 2*dist;
                                SDL_RenderCopy(renderer, direct, &blockDR[i], &cropDR[i]);
                            }
                        }
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        x = e.motion.x;
                        y = e.motion.y;
                        if(y >= margin_top && y <= (margin_top+b_h) && x >= margin_l && x <= margin_l + b_w){
                            SDL_RenderClear(renderer);
                            isPlay = true;
                            return;
                        }else if(y >= (margin_top + b_h + dist) && y <= (margin_top+ 2 * b_h + dist) && x >= margin_l && x <= margin_l + b_w){
                            SDL_RenderClear(renderer);
                            isRestart = true;
                            isPlay = true;
                            return;
                        }else if(y >= (margin_top + 2 * b_h + 2 * dist) && y <= (margin_top+ 3*b_h + 2*dist) && x >= margin_l && x <= margin_l + b_w){
                            isMain = true;
                            return;
                        }
                        break;

                }
                SDL_RenderPresent(renderer);

        }
    }
}
int compute_points(int level, int line_count)
{
    switch (line_count)
    {
    case 1:
        return 40 * (level + 1);
    case 2:
        return 100 * (level + 1);
    case 3:
        return 300 * (level + 1);
    case 4:
        return 1200 * (level + 1);
    }
    return 0;
}
void ttf_handle(SDL_Renderer* renderer, SDL_Texture* font_Texture, SDL_Surface* ttf_Surface, int i, SDL_Rect Text_c[]){
    font_Texture = SDL_CreateTextureFromSurface(renderer, ttf_Surface);
    SDL_RenderCopy(renderer, font_Texture, NULL, &Text_c[i]);
    return;
}
void render_score(SDL_Renderer* renderer, SDL_Texture* font_Texture, int &level , const int& line_count,SDL_Rect Text_c[], TTF_Font *FONT){
    if(CalPoint <= 500) level = 1;
    else if(CalPoint <= 1500 && CalPoint > 500) level = 2;
    else if(CalPoint > 1500) level = 3;
    string score = to_string(CalPoint)+ "        ";
    string level_str = to_string(level)+ "        ";
    string lines = to_string(sum_lines)+ "        ";
    // ERROR!!!! THIS LINE FUCK !! Undermaintained!!!!!!!!!!!
    SDL_Surface* score_Surface = TTF_RenderText_Solid(FONT, score.c_str(), fontColor);
    SDL_Surface* lines_Surface = TTF_RenderText_Solid(FONT, lines.c_str(), fontColor);
    SDL_Surface* level_Surface = TTF_RenderText_Solid(FONT, level_str.c_str(), fontColor);


    cout << TTF_GetError()  << endl;

    ttf_handle(renderer, font_Texture, score_Surface, 2, Text_c);
    ttf_handle(renderer, font_Texture, lines_Surface, 0, Text_c);
    ttf_handle(renderer, font_Texture, level_Surface, 1, Text_c);


    return;



}

void set_delay(int &delay,const int& level){
    if(level == 2) delay = 300;
    else if(level == 3) delay = 100;
    else delay = 450;
}

void show_pause(SDL_Renderer* renderer, SDL_Texture *pausee, const int& gov_w, const int& gov_h){
    renderTexture(pausee, renderer, 40, 100, gov_w, gov_h);
}

void show_explosion();
//void changeLogic(bool& choice1, bool& choice2, bool& choice3){
//}
int main(int argc, char *argv[]){
    SDL_Window* window;
    SDL_Renderer* renderer;
    initSDL(window, renderer);
    // load image
    SDL_Texture *image = loadTexture("images/tiles.png", renderer);
    SDL_Texture *back_ground = loadTexture("images/new_113.png", renderer);
    SDL_Texture *menu = loadTexture("images/menu.png", renderer);
    SDL_Texture *gameover = loadTexture("images/gameover_new.png", renderer);
    SDL_Texture *music_off = loadTexture("images/musicoff.png", renderer);
    SDL_Texture *pause = loadTexture("images/pause.png", renderer);
    SDL_Texture *subMenu = loadTexture("images/submenu.png", renderer);
    SDL_Texture *font_Texture;
    int gov_w, gov_h;
    SDL_QueryTexture(gameover, NULL, NULL, &gov_w, &gov_h);
    //check TTF
    if (TTF_Init() < 0) {
    // Error handling code
            return 0;
    }

    //setting ttf
    TTF_Font *FONT = TTF_OpenFont("fonts/LibreFranklin-Medium.ttf", 30);
    //int params to calculate scores
    int lines = 0, level = 1;
    // Rect contain text
    SDL_Rect Text_c[3];
    for(int i = 0; i < 3; i++){
        Text_c[i].w = 100;
        Text_c[i].h = 30;
        Text_c[i].x = 700;
        Text_c[i].y = 60*(i+1);
    }


    //START MUSIC OOOOOO!!!!!!!!!!
     if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        cout << "ERROR!" << endl;
    }
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
        cout << "EERRR!" << Mix_GetError() << endl;
    }
    Mix_Music *bgm = Mix_LoadMUS("music/Tetris_theme.ogg.mp3");


    // settle some texttures
    SDL_Texture *direct  = loadTexture("images/direct.png", renderer);
    SDL_Rect cropDR[2], blockDR[2];
    int drw, drh;
    SDL_QueryTexture(direct, NULL, NULL, &drw, &drh);
    //define button size
    int bdr_w = 270;
    int bdr_h = 93;
    int margin_l = 255;
    int margin_top = 195;
    // init params
    for(int i = 0; i < 2; i++){
        cropDR[i].w = drw/2;
        cropDR[i].h = drh;
        blockDR[i].w = 70;
        blockDR[i].h = 60;
        if(i == 0){
            cropDR[i].x = margin_l - 80;
            blockDR[i].x = 0;
        }else{
            cropDR[i].x = margin_l + bdr_w + 10;
            blockDR[i].x = 75;
        }
        blockDR[i].y = 0;
    }
    Points prev[4];
    SDL_Rect crop, crop_next;
    // init variable

    int iw, ih;
    SDL_QueryTexture(image, NULL, NULL, &iw, &ih);
    srand(time(NULL));


    //init variable to handle nextblock
    int color, n;
    for(int i = 0; i < 2; i++){
        n=rand()%7;
        color = rand()% 7 + 1;
        shape.enqueue(n);
        colors.enqueue(color);
    }
    //define block width and height
    SDL_Rect prev_block, crop_a;
    prev_block.w = b_w;
    prev_block.h = b_h;
    crop_a.w = iw/8;
    crop_a.h = ih;
    crop_a.y = 0;

    //init gameplay handle
    bool rotates = false;
    bool running = true;
    SDL_Event e;
    int delay = 500;
    unsigned int current_time , last_time=0;
    int dx=0, dy=1;


    //load videocapture
    cv::VideoCapture cap(0);
    if(!cap.isOpened()){
        cout << "Cant open videostream" << endl;
        return -1;
    }
    //get size of frame
//    int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
//    int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    cv::Mat frame;
    char keys;
    //set ROI
    int rect_size = 300;
    cv::Rect roi;
    roi.x = 200;
    roi.y = 120;
    roi.width = rect_size;
    roi.height = rect_size;
    cv::Mat cropROI, maskSkin, copyframe;
    cv::Point centerC;
    int diff;
    string inform = "";
    // start game
    while(running){
        if(isMain){
            game_menu(menu, renderer, direct,music_off, cropDR, blockDR, margin_top, margin_l, bdr_w, bdr_h);
            isMain = false;
        }
        if (isRestart)
            {
                isRestart = false;
                GAME_OVER = false;
                level = 0;
                sum_lines = 0;
                CalPoint = 0;
                for(int i = M-1; i >= 0; i--){
                    for(int j = 0; j < N; j++){
                            maps[i][j] = 0;
                    }
                }
                createBlock(renderer, image, crop, crop_next, iw, ih);
                isPlay = true;

            }
        if(isPlay){
            while(isPlay){
                cap >> frame;
                fps++;
                cropROI = frame.clone()(roi);

                maskSkin = transforms(cropROI);
                centerC = get_center(maskSkin);
                centerC.x += 200;
                centerC.y += 120;
                cv::circle(frame, centerC, 5, cv::Scalar(0, 255,0), -1);

                cv::rectangle(frame, roi, cv::Scalar(0, 255, 0), 2);
                pointSTR.enqueue(centerC.x);
                if(fps = 60){
                    if(pointSTR.sizeQ() < 30){
                    for(int i = 0; i < pointSTR.sizeQ(); i++){
                        if(pointSTR.top() != 0 && pointSTR.bottom() != 0){
                            diff = pointSTR.top() - pointSTR.bottom();
                            if(diff > 50){
                                cv::putText(frame, "Move right", cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0,0,255),2);
                                dx = 1;
                            }else if(diff < -50){
                                cv::putText(frame, "Move left", cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0,0,255),2);
                                dx = -1;
                            }else{
                                cv::putText(frame, "Not moving", cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0,0,255),2);
                            }
                    fps = 0;
                    }
                    }
                }else{
                    for(int i = 0; i < pointSTR.sizeQ(); i++) pointSTR.dequeue();
                }
                }




                cv::imshow("a", maskSkin);

                while( SDL_PollEvent(&e) ){
                    if(MUSIC){
                        if( Mix_PlayingMusic() == 0 )
                        {
                                Mix_PlayMusic( bgm, -1 );
                                MUSIC = true;
                        }
                        else
                        {
                            if( Mix_PausedMusic() == 1 )
                            {
                             Mix_ResumeMusic();
                             }
                        }
                    }else{
                        Mix_HaltMusic();

                    }
                    switch( e.type ){
                        // Look for a keypress
                        case SDL_KEYDOWN:
                            switch( e.key.keysym.sym ){
                                case SDLK_LEFT:
                                    dx -= 1;
                                    break;
                                case SDLK_RIGHT:
                                    dx += 1;
                                    break;
                                case SDLK_UP:
                                    rotates = true;
                                    break;
                                case SDLK_DOWN:
                                    delay = 1;
                                    break;
                                case SDLK_p:
                                    isPause = true;
                                    isPlay = false;
                                    break;
                                case SDLK_q:
                                    END_GAME = true;
                                    isPlay = false;
                                    break;
                                case SDLK_SPACE:
                                    mergeShadow = true;
                                    break;
                                case SDLK_m:
                                    isSubmenu = true;
                                    isPlay = false;
                                    break;
                                default:
                                    break;
                            }
                        }
                }


                //previous position
                for(int p = 0; p < 4; p++){
                    prev[p].x = block[p].x;
                    prev[p].y = block[p].y;

                }

                // move horizontal direction <-->
                if(dx != 0){
                    SDL_RenderClear(renderer);
                    renderTexture(back_ground, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                    next_block(renderer, image, crop_next, iw, ih, shape.top(), colors.top());
                    render_score(renderer, font_Texture, level, lines, Text_c, FONT);

                    for(int i = 0; i < 4; i++){
                        block[i].x += dx*b_w;
                        SDL_RenderCopy(renderer, image, &crop, &block[i]);

                    }
                    if(!valid()){

                        SDL_RenderClear(renderer);
                        renderTexture(back_ground, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                        next_block(renderer, image, crop_next, iw, ih, shape.top(), colors.top());
                        render_score(renderer, font_Texture, level, lines, Text_c, FONT);
                        for(int i =0; i < 4; i++){
                            block[i].x = prev[i].x;
                            block[i].y = prev[i].y;
                            SDL_RenderCopy(renderer, image, &crop, &block[i]);
                        }
                    }
                    dx = 0;
                }


                //init center to rotate
                // default center block[1]
                if(rotates){
                    rotates = false;
                    SDL_RenderClear(renderer);
                    renderTexture(back_ground, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                    next_block(renderer, image, crop_next, iw, ih, shape.top(), colors.top());
                    render_score(renderer, font_Texture, level, lines, Text_c, FONT);

                    Points center;
                    center.x = block[1].x;
                    center.y = block[1].y;
                    for(int i = 0; i < 4; i++){
                        int y = block[i].x - center.x;
                        int x = block[i].y - center.y;
                        //rotate 90 degree matrix
                        //[[0,-1],
                        // [1,0]]
                        block[i].x = -x + center.x;
                        block[i].y = y + center.y;
                        SDL_RenderCopy(renderer, image, &crop, &block[i]);

                    }
                    if(!valid()){
                        SDL_RenderClear(renderer);
                        renderTexture(back_ground, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                        next_block(renderer, image, crop_next, iw, ih, shape.top(), colors.top());
                        render_score(renderer, font_Texture, level, lines, Text_c, FONT);

                        for(int i =0; i < 4; i++){
                            block[i].x = prev[i].x;
                            block[i].y = prev[i].y;
                            SDL_RenderCopy(renderer, image, &crop, &block[i]);
                        }
                    }
                }




                current_time = SDL_GetTicks();
                //store what we already played
                if(current_time > delay + last_time){

                    SDL_RenderClear(renderer);
                    renderTexture(back_ground, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                    next_block(renderer, image, crop_next, iw, ih, shape.top(), colors.top());
                    render_score(renderer, font_Texture, level, lines, Text_c, FONT);

                    for(int i = 0; i < 4; i++){
                        block[i].y += dy*b_h;
                        SDL_RenderCopy(renderer, image, &crop, &block[i]);
                    }

                    if(!valid()){
                        for(int i =0; i < 4; i++){
                            block[i].x = prev[i].x;
                            block[i].y = prev[i].y;
                            SDL_RenderCopy(renderer, image, &crop, &block[i]);
                        }
                        for(int i = 0; i < 4; i++){
                            maps[block[i].y/b_h][block[i].x/b_w] = colors.bottom();
                        }

                        SDL_RenderClear(renderer);
                        renderTexture(back_ground, renderer, 0, 0, W_WIDTH, W_HEIGHT);
                        next_block(renderer, image, crop_next, iw, ih, shape.top(), colors.top());
                        render_score(renderer, font_Texture, level, lines, Text_c, FONT);


                        if(!GAME_OVER){
                            createBlock(renderer, image, crop, crop_next, iw, ih);
                        }



                    }
                    last_time = current_time;
                    // delay - level


                    set_delay(delay, level);

                }
                //check line
                int cut = M-1;
                for(int i = M-1; i > 0; i--){
                    int same = 0;
                    for(int j = 0; j < N; j++){
                        if(maps[i][j]) same+= 1;
                        maps[cut][j] = maps[i][j];
                    }
                    if(same < N) cut -= 1;
                    else lines += 1;
                }
                //calculate point per lines
                sum_lines += lines;
                CalPoint += compute_points(level, lines);
                lines = 0;


                for(int i = M-1; i >= 0; i--){
                    for(int j = 0; j < N; j++){
                        if(maps[i][j] != 0){
                            // + 2 cho block dep hon :D
                            crop_a.x = (maps[i][j]-1)*18 + 2;
                            prev_block.x = j*b_w;
                            prev_block.y = i*b_h;
                            SDL_RenderCopy(renderer, image, &crop_a, &prev_block);
                        }
                    }


                }
                //draw the shadowbox
                draw_shadow(renderer, crop, image);



                if(GAME_OVER){
    //                renderTexture(back_ground, renderer, 0, 0, W_WIDTH, W_HEIGHT);

                    show_gameover(renderer, gameover, gov_w, gov_h);
                    SDL_RenderPresent(renderer);
                    //sleep(3);

                    if (e.type == SDL_KEYDOWN )
                    {

                        if (e.key.keysym.sym == SDLK_ESCAPE)
                        {
                            if(isRestart == false)
                            {
                                isPlay = false;
                                isRestart =true;
                                cout<<"restart";
                            }
                        }
                    }



                }

                SDL_RenderPresent(renderer);
                imshow("Frame",frame);

                keys = (char)cv::waitKey(1);
                // press e to end...
                if(keys == 101){
                    break;
                }


            }
        }
        if(isPause){
            while(isPause){

                show_pause(renderer, pause, gov_w, gov_h);
                while( SDL_PollEvent(&e) ){
                    if (e.type == SDL_KEYDOWN )
                        {

                            if (e.key.keysym.sym == SDLK_r)
                            {
                                isPause = false;
                                isPlay = true;
                                break;
                            }else if(e.key.keysym.sym == SDLK_q){
                                END_GAME = true;
                                break;
                            }
                        }

            }
            SDL_RenderPresent(renderer);



        }
        }
        //Submenu
        if(isSubmenu){
            sub_menu(subMenu, renderer, direct, cropDR, blockDR, margin_top, margin_l, bdr_w, bdr_h);
            isSubmenu = false;
        }
        if(END_GAME){
            break;
        }



    }

    waitUntilKeyPressed();
    cap.release();
    cv::destroyAllWindows();

    SDL_DestroyTexture(menu);
    SDL_DestroyTexture(gameover);
    SDL_DestroyTexture(music_off);
    SDL_DestroyTexture(image);
    SDL_DestroyTexture(back_ground);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(pause);

    quitSDL(window, renderer);
    Mix_Quit();
    IMG_Quit();
    return 0;


}















