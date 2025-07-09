#include <cstdlib>
#include <iostream>
#include <cmath>
#include <queue>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;

#define imsize 400
#define PI 3.14159265358979323846

static int curState = 1;
static int p0x = 0, p0y = 0, p1x = 0, p1y = 0;
static GLubyte imageToDisplay[imsize][imsize][3];
static GLubyte image[imsize][imsize][3];

void DrawPoint(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x >= 0 && x < imsize && y >= 0 && y < imsize) {
        image[x][y][0] = r;
        image[x][y][1] = g;
        image[x][y][2] = b;
    }
}

void DrawLine(int x0, int y0, int x1, int y1, unsigned char r, unsigned char g, unsigned char b) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        DrawPoint(x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
    glutPostRedisplay();
}

void DrawCircle(int xc, int yc, int R, unsigned char r, unsigned char g, unsigned char b) {
    int x = 0, y = R;
    int d = 1 - R;

    while (x <= y) {
        DrawPoint(xc + x, yc + y, r, g, b);
        DrawPoint(xc - x, yc + y, r, g, b);
        DrawPoint(xc + x, yc - y, r, g, b);
        DrawPoint(xc - x, yc - y, r, g, b);
        DrawPoint(xc + y, yc + x, r, g, b);
        DrawPoint(xc - y, yc + x, r, g, b);
        DrawPoint(xc + y, yc - x, r, g, b);
        DrawPoint(xc - y, yc - x, r, g, b);

        if (d < 0)
            d += 2 * x + 3;
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
    glutPostRedisplay();
}

void floodFill(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= imsize || y < 0 || y >= imsize) return;

    unsigned char targetR = image[x][y][0];
    unsigned char targetG = image[x][y][1];
    unsigned char targetB = image[x][y][2];

    if (targetR == r && targetG == g && targetB == b)
    return;

    queue<pair<int, int>> q;
    q.push({ x, y });

    while (!q.empty()) {
        auto current = q.front(); q.pop();
        int cx = current.first;
        int cy = current.second;

        if (cx < 0 || cx >= imsize || cy < 0 || cy >= imsize)
        continue;

        if (image[cx][cy][0] == targetR &&
            image[cx][cy][1] == targetG &&
            image[cx][cy][2] == targetB) {

            DrawPoint(cx, cy, r, g, b);

            q.push({ cx + 1, cy });
            q.push({ cx - 1, cy });
            q.push({ cx, cy + 1 });
            q.push({ cx, cy - 1 });
        }
    }

    glutPostRedisplay();
}

void clearImage(unsigned char r, unsigned char g, unsigned char b) {
    for (int i = 0; i < imsize; i++)
        for (int j = 0; j < imsize; j++)
            DrawPoint(i, j, r, g, b);
    glutPostRedisplay();
}

void executeScript(void) {
    int cx = imsize / 2, cy = imsize / 2;

    // Draw flower center
    DrawCircle(cx, cy, 30, 255, 215, 0); // Gold center

    // Draw petals (using arcs/circles)
    for (int i = 0; i < 360; i += 45) {
        double rad = i * PI / 180;
        int petalX = cx + (int)(80 * cos(rad));
        int petalY = cy + (int)(80 * sin(rad));

        // Draw petal as a circle
        DrawCircle(petalX, petalY, 35, 255, 20, 147); // Pink petals

        // Add smaller circles to make petals more interesting
        DrawCircle(petalX + 15, petalY + 15, 10, 255, 105, 180); // Lighter pink
        DrawCircle(petalX - 15, petalY - 15, 10, 255, 105, 180); // Lighter pink
    }

    // Draw stem
    DrawLine(cx, cy - 30, cx, cy - 150, 34, 139, 34); // Green stem

    // Draw leaves (using arcs/circles)
    // Left leaf
    for (int y = cy - 80; y > cy - 120; y -= 5) {
        int width = (int)(40 * sqrt(1 - pow((double)(y - (cy - 100)) / 20, 2)));
        DrawCircle(cx - width, y, 3, 34, 139, 34);
        DrawCircle(cx - width - 10, y, 3, 34, 139, 34);
    }

    // Right leaf
    for (int y = cy - 100; y > cy - 140; y -= 5) {
        int width = (int)(30 * sqrt(1 - pow((double)(y - (cy - 120)) / 20, 2)));
        DrawCircle(cx + width, y, 3, 34, 139, 34);
        DrawCircle(cx + width + 10, y, 3, 34, 139, 34);
    }

    glutPostRedisplay();
}

void flip(void) {
    for (int i = 0; i < imsize; i++)
        for (int j = 0; j < imsize; j++) {
            imageToDisplay[i][j][0] = image[j][i][0];
            imageToDisplay[i][j][1] = image[j][i][1];
            imageToDisplay[i][j][2] = image[j][i][2];
        }
}

void display(void) {
    glViewport(0, 0, imsize, imsize);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (GLfloat)imsize, 0.0, (GLfloat)imsize, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glRasterPos2i(0, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    flip();
    glDrawPixels((GLsizei)imsize, (GLsizei)imsize, GL_RGB, GL_UNSIGNED_BYTE, imageToDisplay);
    glFlush();
}

void mouseLeftButtonDown(int x, int y) { p0x = x; p0y = y; }
void mouseLeftButtonUp(int x, int y) {
    p1x = x; p1y = y;
    if (curState == 1) DrawLine(p0x, p0y, p1x, p1y, 255, 255, 255);
    else {
        int R = (int)sqrt((p1x - p0x) * (p1x - p0x) + (p1y - p0y) * (p1y - p0y));
        DrawCircle(p0x, p0y, R, 255, 255, 255);
    }
}

void mouseRightButton(int x, int y) {
    floodFill(x, imsize - y, 255, 0, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 's': executeScript(); break;
    case 'c': clearImage(0, 0, 0); break;
    case '1': curState = 1; break;
    case '2': curState = 2; break;
    default: break;
    }
}

void mouse(int button, int state, int x, int y) {
    switch (button) {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN) mouseLeftButtonDown(x, imsize - y);
        if (state == GLUT_UP) mouseLeftButtonUp(x, imsize - y);
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN) mouseRightButton(x, imsize - y);
        break;
    default: break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(imsize, imsize);
    glutInitWindowPosition(200, 200);
    glutCreateWindow(argv[0]);
    clearImage(0, 0, 0);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}