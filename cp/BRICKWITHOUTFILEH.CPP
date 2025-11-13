#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include <stdlib.h>

#define ROWS 5
#define COLS 8
#define BRICK_W 60
#define BRICK_H 20

// ---------------- Base Class ----------------
class GameObject {
public:
    virtual void draw() = 0;   // pure virtual function (abstraction + polymorphism)
    virtual void update() {}   // virtual to allow overriding
};

// ---------------- Paddle Class ----------------
class Paddle : public GameObject {
public:
    int x, y, w, h, speed;

    Paddle(int maxx, int maxy) {
        w = 100;
        h = 10;
        x = maxx / 2 - w / 2;
        y = maxy - 40;
        speed = 30;
    }

    void draw() {
        setfillstyle(SOLID_FILL, LIGHTGREEN);
        bar(x, y, x + w, y + h);
    }

    void update() {
        if (kbhit()) {
            char ch = getch();
            if (ch == 75 && x > 10)              // Left arrow
                x -= speed;
            else if (ch == 77 && x + w < getmaxx() - 10)  // Right arrow
                x += speed;
            else if (ch == 27)                   // ESC key
                exit(0);
        }
    }
};

// ---------------- Ball Class ----------------
class Ball : public GameObject {
public:
    int x, y, r, dx, dy;

    Ball(int maxx, int maxy) {
        x = maxx / 2;
        y = maxy / 2;
        r = 8;
        dx = 4;
        dy = -4;
    }

    void draw() {
        setfillstyle(SOLID_FILL, RED);
        fillellipse(x, y, r, r);
    }

    void update() {
        x += dx;
        y += dy;

        if (x < r || x > getmaxx() - r)
            dx = -dx;
        if (y < r)
            dy = -dy;
    }

    void bounce() { dy = -dy; }
};

// ---------------- Brick Class ----------------
class Brick : public GameObject {
public:
    int x, y, alive, color;

    Brick(int bx, int by, int c) {
        x = bx;
        y = by;
        alive = 1;
        color = c;
    }

    void draw() {
        if (alive) {
            setfillstyle(SOLID_FILL, color);
            bar(x, y, x + BRICK_W, y + BRICK_H);
        }
    }
};

// ---------------- Game Class ----------------
class BrickBreaker {
private:
    Paddle paddle;
    Ball ball;
    Brick* bricks[ROWS][COLS];
    int score;
    int maxx, maxy;

public:
    BrickBreaker(int maxx, int maxy) : paddle(maxx, maxy), ball(maxx, maxy) {
        this->maxx = maxx;
        this->maxy = maxy;
        score = 0;

        int i, j;
        for (i = 0; i < ROWS; i++)
            for (j = 0; j < COLS; j++)
                bricks[i][j] = new Brick(j * (BRICK_W + 10) + 50,
                                         i * (BRICK_H + 10) + 50,
                                         i + 1);
    }

    void drawScore() {
        char text[50];
        sprintf(text, "Score: %d", score);
        setcolor(WHITE);
        outtextxy(10, 10, text);
    }

    void play() {
        int gameover = 0;
        int i, j;

        while (!gameover) {
            cleardevice();
            rectangle(0, 0, maxx, maxy);
            drawScore();

            // draw objects
            for (i = 0; i < ROWS; i++)
                for (j = 0; j < COLS; j++)
                    bricks[i][j]->draw();

            paddle.draw();
            ball.draw();

            // update objects
            paddle.update();
            ball.update();

            // collision with paddle
            if (ball.y + ball.r >= paddle.y &&
                ball.x >= paddle.x && ball.x <= paddle.x + paddle.w) {
                ball.bounce();
            }

            // collision with bricks
            for (i = 0; i < ROWS; i++) {
                for (j = 0; j < COLS; j++) {
                    Brick* b = bricks[i][j];
                    if (b->alive &&
                        ball.x > b->x && ball.x < b->x + BRICK_W &&
                        ball.y - ball.r < b->y + BRICK_H && ball.y + ball.r > b->y) {
                        b->alive = 0;
                        ball.bounce();
                        score += 10;
                    }
                }
            }

            // lose condition
            if (ball.y > maxy) {
                cleardevice();
                setcolor(RED);
                outtextxy(maxx / 2 - 50, maxy / 2, "GAME OVER");
                char finalScore[50];
                sprintf(finalScore, "Final Score: %d", score);
                outtextxy(maxx / 2 - 60, maxy / 2 + 30, finalScore);
                delay(3000);
                gameover = 1;
            }

            // win condition
            int allCleared = 1;
            for (i = 0; i < ROWS; i++)
                for (j = 0; j < COLS; j++)
                    if (bricks[i][j]->alive) allCleared = 0;

            if (allCleared) {
                cleardevice();
                setcolor(YELLOW);
                outtextxy(maxx / 2 - 50, maxy / 2, "YOU WIN!");
                delay(3000);
                gameover = 1;
            }

            delay(20);
        }
    }
};

// ---------------- Main Function ----------------
int main() {
    int gd = DETECT, gm;
    initgraph(&gd, &gm, "C:\\TURBOC3\\BGI");

    int maxx = getmaxx();
    int maxy = getmaxy();

    BrickBreaker game(maxx, maxy);
    game.play();

    closegraph();
    return 0;
}
