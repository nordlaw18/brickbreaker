#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 5
#define COLS 8
#define BRICK_W 60
#define BRICK_H 20

// ---------------- Base Class ----------------
class GameObject {
public:
    virtual void draw() = 0;
    virtual void update() {}
};

// ---------------- Paddle ----------------
class Paddle : public GameObject {
public:
    int x, y, w, h, speed;
    Paddle(int maxx, int maxy) {
        w = 100; h = 10;
        x = maxx/2 - w/2;
        y = maxy - 40;
        speed = 30;
    }
    void draw() {
        setfillstyle(SOLID_FILL, LIGHTGREEN);
        bar(x, y, x+w, y+h);
    }
    void update() {
        if (kbhit()) {
            char ch = getch();
            if (ch == 75 && x > 10) x -= speed;
            else if (ch == 77 && x + w < getmaxx() - 10) x += speed;
            else if (ch == 27) exit(0);
        }
    }
};

// ---------------- Ball ----------------
class Ball : public GameObject {
public:
    int x, y, r, dx, dy;
    Ball(int maxx, int maxy) {
        x = maxx/2; y = maxy/2;
        r = 8; dx = 4; dy = -4;
    }
    void draw() {
        setfillstyle(SOLID_FILL, RED);
        fillellipse(x, y, r, r);
    }
    void update() {
        x += dx; y += dy;
        if (x < r || x > getmaxx()-r) dx = -dx;
        if (y < r) dy = -dy;
    }
    void bounce() { dy = -dy; }
};

// ---------------- Brick ----------------
class Brick : public GameObject {
public:
    int x, y, alive, color;
    Brick(int bx, int by, int c) {
        x = bx; y = by; alive = 1; color = c;
    }
    void draw() {
        if (alive) {
            setfillstyle(SOLID_FILL, color);
            bar(x, y, x+BRICK_W, y+BRICK_H);
        }
    }
};

// ---------------- Game ----------------
class BrickBreaker {
private:
    Paddle paddle;
    Ball ball;
    Brick* bricks[ROWS][COLS];
    int score;
    int maxx, maxy;
    char playerName[50];
public:
    BrickBreaker(int maxx, int maxy, char name[]) : paddle(maxx,maxy), ball(maxx,maxy) {
        this->maxx = maxx; this->maxy = maxy;
        score = 0;
        strcpy(playerName, name);
        int i,j;
        for (i=0;i<ROWS;i++)
            for (j=0;j<COLS;j++)
                bricks[i][j] = new Brick(j*(BRICK_W+10)+50,
                                         i*(BRICK_H+10)+50, i+1);
    }

    void drawScore() {
        char text[60];
        sprintf(text, "Player: %s   Score: %d", playerName, score);
        setcolor(WHITE);
        outtextxy(10,10,text);
    }

    void saveScore() {
        FILE *fp;
        fp = fopen("C:\\TURBOC3\\PROJECTS\\cp\\scores.txt", "a");
        if (fp != NULL) {
            fprintf(fp, "%s %d\n", playerName, score);
            fclose(fp);
        }
    }

    void showLeaderboard() {
        FILE *fp;
        char name[50];
        int sc, y = 200;
        setcolor(WHITE);
        outtextxy(maxx/2 - 70, 150, "----- LEADERBOARD -----");
        fp = fopen("scores.txt", "r");
        if (fp == NULL) {
            outtextxy(maxx/2 - 80, 180, "No scores recorded yet.");
            return;
        }
        while (fscanf(fp, "%s %d", name, &sc) != EOF) {
            char text[80];
            sprintf(text, "%s  -  %d", name, sc);
            outtextxy(maxx/2 - 60, y, text);
            y += 20;
        }
        fclose(fp);
    }

    void play() {
        int gameover = 0;
        int i,j;

        while (!gameover) {
            cleardevice();
            rectangle(0,0,maxx,maxy);
            drawScore();

            for (i=0;i<ROWS;i++)
                for (j=0;j<COLS;j++)
                    bricks[i][j]->draw();

            paddle.draw();
            ball.draw();

            paddle.update();
            ball.update();

            // collision with paddle
            if (ball.y + ball.r >= paddle.y &&
                ball.x >= paddle.x && ball.x <= paddle.x + paddle.w)
                ball.bounce();

            // collision with bricks
            for (i=0;i<ROWS;i++) {
                for (j=0;j<COLS;j++) {
                    Brick *b = bricks[i][j];
                    if (b->alive &&
                        ball.x > b->x && ball.x < b->x+BRICK_W &&
                        ball.y - ball.r < b->y + BRICK_H && ball.y + ball.r > b->y) {
                        b->alive = 0;
                        ball.bounce();
                        score += 10;
                    }
                }
            }

            // lose
            if (ball.y > maxy) {
                cleardevice();
                setcolor(RED);
                outtextxy(maxx/2-50, maxy/2, "GAME OVER");
                char finalScore[50];
                sprintf(finalScore, "Final Score: %d", score);
                outtextxy(maxx/2-60, maxy/2+30, finalScore);
                saveScore();
                delay(1500);
                cleardevice();
                showLeaderboard();
                getch();
                gameover = 1;
            }

            // win
            int allCleared = 1;
            for (i=0;i<ROWS;i++)
                for (j=0;j<COLS;j++)
                    if (bricks[i][j]->alive) allCleared = 0;
            if (allCleared) {
                cleardevice();
                setcolor(YELLOW);
                outtextxy(maxx/2-50, maxy/2, "YOU WIN!");
                saveScore();
                delay(1500);
                cleardevice();
                showLeaderboard();
                getch();
                gameover = 1;
            }

            delay(20);
        }
    }
};

// ---------------- Start Screen ----------------
void startScreen(char *name) {
    cleardevice();
    setcolor(YELLOW);
    outtextxy(200, 150, "WELCOME TO BRICK BREAKER!");
    outtextxy(200, 200, "Enter your name: ");
    gotoxy(35, 15);
    gets(name);
    cleardevice();
    setcolor(WHITE);
    outtextxy(200, 200, "Press any key to start...");
    getch();
}

// ---------------- Main ----------------
int main() {
    int gd = DETECT, gm;
    initgraph(&gd, &gm, "C:\\TURBOC3\\BGI");

    int maxx = getmaxx();
    int maxy = getmaxy();

    char playerName[50];
    startScreen(playerName);

    BrickBreaker game(maxx, maxy, playerName);
    game.play();

    closegraph();
    return 0;
}
