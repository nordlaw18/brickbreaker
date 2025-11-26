#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>

enum PowerType { NONE, EXPAND_PADDLE, SHRINK_PADDLE };

#define ROWS 5
#define COLS 8
#define TOTAL_BRICKS (ROWS * COLS)

#define BASE_PADDLE_W 80
#define MAX_BALLS 1
#define HITS_PER_BRICK 2

struct Brick {
    int x, y, w, h;
    int alive;
    int hits;
    int points;
    PowerType power;
};

struct Ball {
    float x, y;
    float dx, dy;
    int r;
    int active;
};

struct PlayerScore {
    char name[50];
    int score;
};

Brick bricks[TOTAL_BRICKS];
Ball balls[MAX_BALLS];
int activeBalls = 1;

int paddleX, paddleY;
int paddleW = BASE_PADDLE_W;
int paddleH = 10;

int score = 0;
int lives = 3;

int expandTimer = 0;
int shrinkTimer = 0;

char playerNameGlobal[50];

const char SCORE_FILE_PATH[] = "C:\\TURBOC3\\PROJECTS\\cp\\scores.txt";

void askPlayerName() {
    cleardevice();
    setcolor(YELLOW);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(140, 80, "WELCOME TO BRICK BREAKER");
    outtextxy(120, 150, "Enter your name : ");
    gotoxy(40, 12);
    gets(playerNameGlobal);
    if(strlen(playerNameGlobal)==0) strcpy(playerNameGlobal,"PLAYER");
    cleardevice();
    outtextxy(150, 200, "Press any key to start...");
    getch();
}

void saveScoreTop5(const char *name, int sc) {
    PlayerScore list[50];
    int count = 0;
    int i, j;

    FILE *fin = fopen(SCORE_FILE_PATH, "r");
    if(fin!=NULL) {
        while(fscanf(fin,"%49s %d",list[count].name,&list[count].score)==2 && count<50)
            count++;
        fclose(fin);
    }

    if(count < 50) {
        strcpy(list[count].name, name);
        list[count].score = sc;
        count++;
    }

    for(i=0;i<count-1;i++)
        for(j=i+1;j<count;j++)
            if(list[j].score > list[i].score) {
                PlayerScore tmp = list[i];
                list[i] = list[j];
                list[j] = tmp;
            }

    FILE *fout = fopen(SCORE_FILE_PATH,"w");
    if(fout != NULL) {
        int limit = (count < 5) ? count : 5;
        for(i=0; i<limit; i++)
            fprintf(fout,"%s %d\n",list[i].name,list[i].score);
        fclose(fout);
    }
}

void showLeaderboardScreen() {
    cleardevice();
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(180,40,"----- TOP 5 LEADERBOARD -----");

    FILE *fin=fopen(SCORE_FILE_PATH,"r");
    if(fin==NULL) {
        outtextxy(200,150,"No scores yet.");
        outtextxy(200,200,"Press any key...");
        getch();
        return;
    }

    char name[50], buf[70];
    int sc, rank = 1, y = 100;

    while(fscanf(fin,"%49s %d",name,&sc)==2 && rank<=5) {
        sprintf(buf,"%d. %s - %d",rank,name,sc);
        outtextxy(150,y,buf);
        rank++; 
        y += 30;
    }
    fclose(fin);

    outtextxy(150,y+30,"Press any key...");
    getch();
}

void initBricks() {
    int w = 60, h = 20;
    int sx = 50, sy = 10;
    int r, c, k = 0;
    int i;

    for(r=0;r<ROWS;r++) {
        int pts = r + 1;   // new scoring 1–5
        for(c=0;c<COLS;c++) {
            bricks[k].x = sx + c*(w+5);
            bricks[k].y = sy + r*(h+2);
            bricks[k].w = w;
            bricks[k].h = h;
            bricks[k].alive = 1;
            bricks[k].hits = HITS_PER_BRICK;
            bricks[k].points = pts;
            bricks[k].power = NONE;
            k++;
        }
    }

    for(i=0; i<6; i++) {
        int ri = rand() % TOTAL_BRICKS;
        bricks[ri].power = (rand()%2==0)?EXPAND_PADDLE:SHRINK_PADDLE;
    }
}

void drawBricks() {
    int i;
    for(i=0;i<TOTAL_BRICKS;i++) {
        if(!bricks[i].alive) continue;

        if(bricks[i].hits==1) setcolor(WHITE);
        else setcolor(LIGHTBLUE);

        rectangle(bricks[i].x,bricks[i].y,
                  bricks[i].x+bricks[i].w,
                  bricks[i].y+bricks[i].h);
    }
}

void initGameObjects() {
    int i;
    paddleX = 300;
    paddleY = 450;

    for(i=0;i<MAX_BALLS;i++) {
        balls[i].active = 0;
        balls[i].r = 7;
    }

    balls[0].active = 1;
    balls[0].x = 320;
    balls[0].y = 430;
    balls[0].dx = 2.5;
    balls[0].dy = -2.5;
    activeBalls = 1;
}

void updateBalls() {
    int i;
    for(i=0;i<MAX_BALLS;i++) {
        if(!balls[i].active) continue;
        if(!balls[i].active) continue;

        balls[i].x += balls[i].dx;
        balls[i].y += balls[i].dy;

        if(balls[i].x < balls[i].r || balls[i].x > getmaxx()-balls[i].r) balls[i].dx *= -1;
        if(balls[i].y < balls[i].r) balls[i].dy *= -1;

        if(balls[i].y > getmaxy()) {
            balls[i].active = 0;
            activeBalls--;
            if(activeBalls == 0) {
                lives--;
                initGameObjects();
            }
        }

        if(balls[i].y + balls[i].r >= paddleY &&
           balls[i].x >= paddleX && balls[i].x <= paddleX+paddleW)
            balls[i].dy *= -1;

        if(balls[i].dx > 0 && balls[i].dx < 6) balls[i].dx *= 1.001;
        if(balls[i].dx < 0 && balls[i].dx > -6) balls[i].dx *= 1.001;
        if(balls[i].dy > 0 && balls[i].dy < 6) balls[i].dy *= 1.001;
        if(balls[i].dy < 0 && balls[i].dy > -6) balls[i].dy *= 1.001;
    }
}

void checkBrickCollisions() {
    int i, j;

    for(i=0;i<MAX_BALLS;i++) {
        if(!balls[i].active) continue;

        for(j=0;j<TOTAL_BRICKS;j++) {
            if(!bricks[j].alive) continue;

            if(balls[i].x > bricks[j].x &&
               balls[i].x < bricks[j].x+bricks[j].w &&
               balls[i].y > bricks[j].y &&
               balls[i].y < bricks[j].y+bricks[j].h) {

                bricks[j].hits--;
                balls[i].dy *= -1;

                if(bricks[j].hits <= 0) {
                    bricks[j].alive = 0;
                    score += bricks[j].points;
                }

                if(bricks[j].power == EXPAND_PADDLE) { paddleW=120; expandTimer=300; }
                if(bricks[j].power == SHRINK_PADDLE) { paddleW=50;  shrinkTimer=300; }
            }
        }
    }
}

void drawPaddle() {
    setcolor(WHITE);
    rectangle(paddleX,paddleY,paddleX+paddleW,paddleY+paddleH);
}

int main() {
    int gd=DETECT, gm;
    int i, alive;

    initgraph(&gd, &gm, "C:\\TURBOC3\\BGI");
    srand(time(0));

    askPlayerName();
    initBricks();
    initGameObjects();

    while(1) {
        cleardevice();

        if(kbhit()) {
            char c = getch();
            if(c==75 && paddleX>0) paddleX-=20;
            if(c==77 && paddleX+paddleW<getmaxx()) paddleX+=20;
            if(c==27) break;
        }

        if(expandTimer>0) { expandTimer--; if(expandTimer==0) paddleW=BASE_PADDLE_W; }
        if(shrinkTimer>0) { shrinkTimer--; if(shrinkTimer==0) paddleW=BASE_PADDLE_W; }

        updateBalls();
        checkBrickCollisions();
        drawBricks();
        drawPaddle();

        for(i=0;i<MAX_BALLS;i++)
            if(balls[i].active)
                fillellipse(balls[i].x,balls[i].y,balls[i].r,balls[i].r);

        char scoreText[40], livesText[40];
        sprintf(scoreText,"Score: %d",score);
        outtextxy(10,10,scoreText);
        sprintf(livesText,"Lives: %d",lives);
        outtextxy(10,30,livesText);

        alive = 0;
        for(i=0;i<TOTAL_BRICKS;i++)
            if(bricks[i].alive) alive = 1;

        if(!alive) {
            cleardevice();
            outtextxy(250,200,"YOU WIN!");
            delay(1200);
            saveScoreTop5(playerNameGlobal,score);
            showLeaderboardScreen();
            break;
        }

        if(lives<=0) {
            cleardevice();
            outtextxy(240,200,"GAME OVER");
            char temp[40];
            sprintf(temp,"Final Score: %d",score);
            outtextxy(240,240,temp);
            delay(1200);
            saveScoreTop5(playerNameGlobal,score);
            showLeaderboardScreen();
            break;
        }

        delay(10);
    }

    closegraph();
    return 0;
}
