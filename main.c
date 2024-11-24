#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#define MAX_VERTICES 10000
#define MAX_FACES 10000

typedef struct {
    float x, y, z;
} Vertice3D;

typedef struct {
    float u, v;
} Textura;

typedef struct {
    float x, y, z;
} Normal;

typedef struct {
    int v[4];
    int t[4];
    int n[4];
} Face;

Vertice3D vertices[MAX_VERTICES];
Textura texturas[MAX_VERTICES];
Normal normais[MAX_VERTICES];
Face faces[MAX_FACES];
int numVertices = 0;
int numTexturas = 0;
int numNormais = 0;
int numFaces = 0;

float carX = 0.0;
float carY = -1.4;
float carZ = 0.0;
float obstacleSpeed = 0.1;
float obstacleX[5];
float obstacleZ[5];

int score = 0;
float obstacleColorR[5];
float obstacleColorG[5];
float obstacleColorB[5];

void initLighting() {
    GLfloat ambientLight[] = { 0.2, 0.2, 0.2, 1.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    GLfloat diffuseLight[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat lightPosition[] = { 1.0, 1.0, 1.0, 0.0 };

    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void setMaterialProperties(GLfloat ambient[], GLfloat diffuse[], GLfloat specular[], GLfloat shininess) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void initObstacles() {
    srand(time(NULL));
    for (int i = 0; i < 5; i++) {
        obstacleX[i] = (rand() % 3 - 1) * 1.0;
        obstacleZ[i] = -10.0 - i * 10.0;

        obstacleColorR[i] = (float)rand() / RAND_MAX;
        obstacleColorG[i] = (float)rand() / RAND_MAX;
        obstacleColorB[i] = (float)rand() / RAND_MAX;
    }
}

void carregarOBJ(const char *nomeArquivo) {
    FILE *fp = fopen(nomeArquivo, "r");
    if (!fp) {
        printf("Erro ao abrir o arquivo %s\n", nomeArquivo);
        exit(1);
    }

    char linha[128];
    while (fgets(linha, sizeof(linha), fp)) {
        if (strncmp(linha, "v ", 2) == 0) {
            Vertice3D vertice;
            sscanf(linha, "v %f %f %f", &vertice.x, &vertice.y, &vertice.z);
            vertices[numVertices++] = vertice;
        } else if (strncmp(linha, "vt ", 3) == 0) {
            Textura texCoord;
            sscanf(linha, "vt %f %f", &texCoord.u, &texCoord.v);
            texturas[numTexturas++] = texCoord;
        } else if (strncmp(linha, "vn ", 3) == 0) {
            Normal normal;
            sscanf(linha, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
            normais[numNormais++] = normal;
        } else if (strncmp(linha, "f ", 2) == 0) {
            Face face;
            int n = sscanf(linha, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                           &face.v[0], &face.t[0], &face.n[0],
                           &face.v[1], &face.t[1], &face.n[1],
                           &face.v[2], &face.t[2], &face.n[2],
                           &face.v[3], &face.t[3], &face.n[3]);
            for (int i = 0; i < 4; i++) {
                face.v[i] = (face.v[i] > 0) ? face.v[i] - 1 : -1;
                face.t[i] = (face.t[i] > 0) ? face.t[i] - 1 : -1;
                face.n[i] = (face.n[i] > 0) ? face.n[i] - 1 : -1;
            }
            faces[numFaces++] = face;
        }
    }
    fclose(fp);
    printf("Carregado %d vértices, %d texturas, %d normais e %d faces do arquivo %s\n",
           numVertices, numTexturas, numNormais, numFaces, nomeArquivo);
}

void desenharCarro() {
    for (int i = 0; i < numFaces; i++) {
        Face face = faces[i];
        glBegin(face.v[3] == -1 ? GL_TRIANGLES : GL_QUADS);
        for (int j = 0; j < 4 && face.v[j] != -1; j++) {
            if (face.n[j] != -1) {
                Normal normal = normais[face.n[j]];
                glNormal3f(normal.x, normal.y, normal.z);
            }
            if (face.t[j] != -1) {
                Textura texCoord = texturas[face.t[j]];
                glTexCoord2f(texCoord.u, texCoord.v);
            }
            Vertice3D vertice = vertices[face.v[j]];
            glVertex3f(vertice.x, vertice.y, vertice.z);
        }
        glEnd();
    }
}

void desenharCilindro() {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);

    glPushMatrix();
    glTranslatef(0.0, -1.1, 0.0);
    glRotated(80.0,1.0,0.0,0.0);
    gluCylinder(quad, 0.3, 0.3, 1.0, 20, 20);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    GLfloat pistaAmbient[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat pistaDiffuse[] = { 0.3, 0.3, 0.3, 1.0 };
    GLfloat pistaSpecular[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat pistaShininess = 10.0;
    setMaterialProperties(pistaAmbient, pistaDiffuse, pistaSpecular, pistaShininess);

    glBegin(GL_QUADS);
    for (int i = -10; i < 20; i++) {
        glVertex3f(-2.3, -2.0, -10.0 - i * 10.0);
        glVertex3f(2.3, -2.0, -10.0 - i * 10.0);
        glVertex3f(2.3, -2.0, -i * 10.0);
        glVertex3f(-2.3, -2.0, -i * 10.0);
    }
    glEnd();

    GLfloat obstaculoAmbient[] = { 0.5, 0.25, 0.0, 1.0 };
    GLfloat obstaculoSpecular[] = { 0.3, 0.15, 0.0, 1.0 };
    GLfloat obstaculoShininess = 30.0;

    for (int i = 0; i < 5; i++) {
        glPushMatrix();
        glTranslatef(obstacleX[i], 0.0, obstacleZ[i]);
        GLfloat obstaculoDiffuse[] = { obstacleColorR[i], obstacleColorG[i], obstacleColorB[i], 1.0 };
        setMaterialProperties(obstaculoAmbient, obstaculoDiffuse, obstaculoSpecular, obstaculoShininess);
        desenharCilindro();
        glPopMatrix();
    }

    GLfloat carroAmbient[] = { 0.0, 0.2, 0.0, 1.0 };
    GLfloat carroDiffuse[] = { 0.5, 0.0, 0.0, 1.0 };
    GLfloat carroSpecular[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat carroShininess = 50.0;

    glPushMatrix();
    glTranslatef(carX, carY, carZ);
    float escala = 0.23;
    glScalef(escala, escala, escala);
    glRotated(-180.0, 0, 1, 0);
    setMaterialProperties(carroAmbient, carroDiffuse, carroSpecular, carroShininess);
    desenharCarro();
    glPopMatrix();

    glutSwapBuffers();
}


void closeGame() {
    exit(0);
}

void gameOver() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    GLfloat textColor[] = {0.0, 0.0, 0.0, 1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, textColor);

    glRasterPos2f(-0.25, 0.3);
    const char* message = "GAME OVER";
    for (const char* c = message; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    printf("Score: %d", score);
    char scoreMessage[50];
    sprintf(scoreMessage, "SCORE: %d", score);
    glRasterPos2f(-0.25, -0.1);
    for (const char* c = scoreMessage; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glutSwapBuffers();

    glutTimerFunc(3000, closeGame, 0);
}

void update(int value) {
    obstacleSpeed += 0.0002;
    score++;

    for (int i = 0; i < 5; i++) {
        obstacleZ[i] += obstacleSpeed;
        if (obstacleZ[i] > 10.0) {
            obstacleZ[i] = -50.0;
            obstacleX[i] = (rand() % 3 - 1) * 1.0;
        }
    }


    for (int i = 0; i < 5; i++) {
        if (fabs(carX - obstacleX[i]) < 0.5 && fabs(-obstacleZ[i]) < 0.5) {
            printf("Game Over!\n");
            glutDisplayFunc(gameOver);
            glutPostRedisplay();
            return;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void init() {
    glClearColor(0.2, 0.5, 0.2, 1.0);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0/600.0, 1.0, 100.0);
    gluLookAt(0.0, 3.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    initObstacles();
    initLighting();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    carregarOBJ("carro.obj");

    //PlaySound(TEXT("music.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}


void handleKeypress(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT && carX > -1.0) carX -= 1.0;
    if (key == GLUT_KEY_RIGHT && carX < 1.0) carX += 1.0;
}

int main(int argc, char** argv) {
    system("start /min wmplayer /play /repeat music.mp3");

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    int window = glutCreateWindow("Racing");
    glutSetWindow(window);
    init();
    glutDisplayFunc(display);
    glutSpecialFunc(handleKeypress);
    glutTimerFunc(2000, update, 0);
    glutMainLoop();
    //PlaySound(NULL, 0, 0);
    return 0;
}
