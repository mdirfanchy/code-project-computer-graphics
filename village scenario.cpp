// Village Scenery with Moving Boat
// Single-file C++ OpenGL (FreeGLUT) program
// Implements DDA line, Midpoint circle, 2D transformations (translation, scaling, rotation)
// Animation: moving boat, drifting clouds, rotating windmill blades

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cmath>
#include <iostream>

// Window size
const int WIN_W = 800;
const int WIN_H = 600;

// Animation state
float boatX = -200.0f;   // starts left of screen
float cloudX = -100.0f;
float bladeAngle = 0.0f;

// Utility: plot a point for algorithms (in window coordinates)
void plotPoint(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// DDA Line Drawing (integer-friendly but using float steps)
void ddaLine(int x0, int y0, int x1, int y1) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int steps = std::max(abs(dx), abs(dy));
    if (steps == 0) { plotPoint(x0, y0); return; }
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    float x = x0, y = y0;
    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; ++i) {
        glVertex2i((int)round(x), (int)round(y));
        x += xInc; y += yInc;
    }
    glEnd();
}

// Midpoint circle algorithm (integer) -- draws points on circumference
void midpointCircle(int cx, int cy, int r) {
    int x = 0, y = r;
    int d = 1 - r;
    glBegin(GL_POINTS);
    while (x <= y) {
        glVertex2i(cx + x, cy + y);
        glVertex2i(cx - x, cy + y);
        glVertex2i(cx + x, cy - y);
        glVertex2i(cx - x, cy - y);
        glVertex2i(cx + y, cy + x);
        glVertex2i(cx - y, cy + x);
        glVertex2i(cx + y, cy - x);
        glVertex2i(cx - y, cy - x);
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y -= 1;
        }
        x += 1;
    }
    glEnd();
}

// Draw sun using midpoint circle (filled by drawing many concentric circles)
void drawSun(int cx, int cy, int r) {
    glColor3f(1.0f, 0.85f, 0.0f);
    // simple fill by concentric circles
    for (int rr = r; rr > 0; rr -= 1) {
        midpointCircle(cx, cy, rr);
    }
}

// Draw cloud (several overlapping circles) using midpoint circle
void drawCloud(int cx, int cy) {
    glColor3f(1.0f, 1.0f, 1.0f);
    midpointCircle(cx, cy, 22);
    midpointCircle(cx + 25, cy + 6, 20);
    midpointCircle(cx - 25, cy + 6, 20);
    midpointCircle(cx + 55, cy, 18);
}

// Draw a simple house using polygons and DDA for roof/edges
void drawHouse(int x, int y, int w, int h) {
    // body
    glColor3f(0.78f, 0.6f, 0.4f);
    glBegin(GL_POLYGON);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();

    // roof (triangle) drawn with filled polygon (but edges with DDA)
    glColor3f(0.55f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2i(x - 10, y + h);
    glVertex2i(x + w + 10, y + h);
    glVertex2i(x + w/2, y + h + h/2);
    glEnd();

    // roof edges using DDA for demonstration
    glColor3f(0.0f, 0.0f, 0.0f);
    ddaLine(x - 10, y + h, x + w/2, y + h + h/2);
    ddaLine(x + w + 10, y + h, x + w/2, y + h + h/2);

    // door
    glColor3f(0.35f, 0.2f, 0.1f);
    glBegin(GL_POLYGON);
    glVertex2i(x + w/3, y);
    glVertex2i(x + w*2/3, y);
    glVertex2i(x + w*2/3, y + h/2);
    glVertex2i(x + w/3, y + h/2);
    glEnd();

    // windows (small squares)
    glColor3f(0.2f, 0.6f, 0.9f);
    glBegin(GL_POLYGON);
    glVertex2i(x + 10, y + h - 30);
    glVertex2i(x + 10 + 25, y + h - 30);
    glVertex2i(x + 10 + 25, y + h - 5);
    glVertex2i(x + 10, y + h - 5);
    glEnd();
}
// Draw tree (simple triangle foliage + trunk rectangle). Accepts scale.
void drawTree(int x, int y, float scale) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f);

    // trunk
    glColor3f(0.45f, 0.26f, 0.07f);
    glBegin(GL_POLYGON);
    glVertex2i(-8, 0);
    glVertex2i(8, 0);
    glVertex2i(8, 30);
    glVertex2i(-8, 30);
    glEnd();
// foliage (three triangles)
    glColor3f(0.13f, 0.55f, 0.13f);
    glBegin(GL_POLYGON);
    glVertex2i(-40, 30);
    glVertex2i(40, 30);
    glVertex2i(0, 90);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2i(-30, 50);
    glVertex2i(30, 50);
    glVertex2i(0, 110);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2i(-20, 70);
    glVertex2i(20, 70);
    glVertex2i(0, 130);
    glEnd();

    glPopMatrix();
}

// Draw a windmill (tower + rotating blades). We'll rotate blades with bladeAngle.
void drawWindmill(int x, int y) {
    // tower
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_POLYGON);
    glVertex2i(x - 10, y);
    glVertex2i(x + 10, y);
    glVertex2i(x + 10, y + 100);
    glVertex2i(x - 10, y + 100);
    glEnd();

    // hub
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i += 30) {
        float rad = i * 3.14159f / 180.0f;
        glVertex2f(x + cos(rad) * 6, y + 100 + sin(rad) * 6);
    }
    glEnd();

    // blades (use rotation transformation)
    glPushMatrix();
    glTranslatef(x, y + 100, 0);
    glRotatef(bladeAngle, 0, 0, 1);

    glColor3f(0.95f, 0.95f, 0.95f);
    // draw 3 blades
    for (int i = 0; i < 3; ++i) {
        glBegin(GL_POLYGON);
        glVertex2f(6, 0);
        glVertex2f(140, 20);
        glVertex2f(140, -20);
        glVertex2f(6, 0);
        glEnd();
        glRotatef(120, 0, 0, 1);
    }

    glPopMatrix();
}

// Draw river as a filled polygon with two banks (edges drawn with DDA)
void drawRiver() {
    glColor3f(0.07f, 0.53f, 0.75f);
    glBegin(GL_POLYGON);
    glVertex2i(0, 160);
    glVertex2i(800, 130);
    glVertex2i(800, 0);
    glVertex2i(0, 0);
    glEnd();

    // river banks with DDA (top and bottom edges)
    glColor3f(0.0f, 0.3f, 0.2f);
    ddaLine(0, 160, 800, 130);
    ddaLine(0, 0, 800, 0);
}

// Draw boat at base position (boatX will be applied as translation)
void drawBoatShape() {
    // hull
    glColor3f(0.55f, 0.27f, 0.07f);
    glBegin(GL_POLYGON);
    glVertex2i(-60, 40);
    glVertex2i(60, 40);
    glVertex2i(40, 20);
    glVertex2i(-40, 20);
    glEnd();

    // cabin
    glColor3f(0.8f, 0.1f, 0.1f);
    glBegin(GL_POLYGON);
    glVertex2i(-20, 50);
    glVertex2i(20, 50);
    glVertex2i(20, 75);
    glVertex2i(-20, 75);
    glEnd();

    // mast
    glColor3f(0.35f, 0.2f, 0.1f);
    glBegin(GL_LINES);
    glVertex2i(0, 75);
    glVertex2i(0, 100);
    glEnd();

    // sail using DDA for one edge
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2i(0, 100);
    glVertex2i(40, 80);
    glVertex2i(0, 60);
    glEnd();

    // optional decoration lines
    glColor3f(0,0,0);
    ddaLine(-60, 40, 60, 40);
}

// Draw full scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // --- Sky background ---
    glColor3f(0.53f, 0.81f, 0.98f);
    glBegin(GL_POLYGON);
    glVertex2i(0, 600);
    glVertex2i(800, 600);
    glVertex2i(800, 300);
    glVertex2i(0, 300);
    glEnd();

    // Sun
    drawSun(680, 520, 40);

    // Clouds (animated)
    glPushMatrix();
    glTranslatef(cloudX, 0, 0);
    drawCloud(120, 520);
    drawCloud(260, 560);
    glPopMatrix();

    // --- Distant hills ---
    glColor3f(0.22f, 0.47f, 0.2f);
    glBegin(GL_POLYGON);
    glVertex2i(0, 300);
    glVertex2i(200, 380);
    glVertex2i(350, 300);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2i(300, 300);
    glVertex2i(450, 420);
    glVertex2i(600, 300);
    glEnd();

    // --- River ---
    drawRiver();

    // --- Banks, grass ---
    glColor3f(0.2f, 0.7f, 0.2f);
    glBegin(GL_POLYGON);
    glVertex2i(0, 300);
    glVertex2i(800, 300);
    glVertex2i(800, 160);
    glVertex2i(0, 160);
    glEnd();

    // Houses
    drawHouse(70, 180, 110, 90);
    drawHouse(220, 190, 100, 80);
    drawHouse(360, 185, 120, 90);

    // Trees with different scaling
    drawTree(520, 180, 0.9f);
    drawTree(620, 170, 0.7f);
    drawTree(720, 170, 0.8f);

    // Windmill with rotation (transformation - rotation + translation)
    drawWindmill(470, 180);
// Boat (translation applied)
    glPushMatrix();
    glTranslatef(boatX, 0, 0); // translation
    drawBoatShape();
    glPopMatrix();

    // Optional: draw a simple reflection of boat (by scaling Y negative with transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix();
    glTranslatef(boatX, 10, 0); // place slightly lower
    glScalef(1.0f, -1.0f, 1.0f); // reflect in Y
    glColor4f(0.55f, 0.27f, 0.07f, 0.4f);
    // hull reflection
    glBegin(GL_POLYGON);
    glVertex2i(-60, 40);
    glVertex2i(60, 40);
    glVertex2i(40, 20);
    glVertex2i(-40, 20);
    glEnd();
    glPopMatrix();
    glDisable(GL_BLEND);

    glutSwapBuffers();
}

// Timer for animation updates
void update(int value) {
    // Move boat to the right; reset if beyond window
    boatX += 1.5f; // translation speed
    if (boatX > WIN_W + 200) boatX = -300.0f;

    // Clouds drift slower
    cloudX += 0.6f;
    if (cloudX > WIN_W + 200) cloudX = -400.0f;

    // Rotate windmill blades
    bladeAngle += 4.0f;
    if (bladeAngle >= 360.0f) bladeAngle -= 360.0f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // approx 60 FPS
}

void init() {
    glClearColor(0.5f, 0.8f, 0.95f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glPointSize(1.5f);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(200, 50);
    glutCreateWindow("Village Scenery with Moving Boat - OpenGL Project");

    init();

    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
