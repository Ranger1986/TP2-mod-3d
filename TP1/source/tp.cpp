// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <GL/glut.h>
#include <float.h>
#include "src/Vec3.h"
#include "src/Triangle.h"
#include "src/Mesh.h"
#include "src/MeshIO.h"
#include "src/Camera.h"


enum DisplayMode {
    WIRE = 0, SOLID = 1, LIGHTED_WIRE = 2, LIGHTED = 3
};

//Transformation made of a matrix multiplication and translation
struct Transform {

protected:
    Mat3 m_transformation; //Includes rotation + scale
    Vec3 m_translation; //translation applied to points
public:

    //Constructor, by default Identity and no translation
    Transform(Mat3 i_transformation = Mat3::Identity(), Vec3 i_translation = Vec3(0., 0., 0.))
        : m_transformation(i_transformation), m_translation(i_translation) {
    }

    Vec3 const &translation() { return m_translation; }

    Mat3 const &transformation_matrix() { return m_transformation; }
};

//Input mesh loaded at the launch of the application
Mesh mesh;
//Mesh on which a transformation is applied
Mesh transformed_mesh;

bool display_normals;
bool display_smooth_normals;
bool display_mesh;
bool display_transformed_mesh;

DisplayMode displayMode;
// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 1600;
static unsigned int SCREENHEIGHT = 900;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX = 0, lastY = 0, lastZoom = 0;
static bool fullScreen = false;



// ------------------------------------
// Application initialization
// ------------------------------------
void initLight() {
    GLfloat light_position1[4] = {22.0f, 16.0f, 50.0f, 0.0f};
    GLfloat direction1[3] = {-52.0f, -16.0f, -50.0f};
    GLfloat color1[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};

    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, color1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, color1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
}

void init() {
    camera.resize(SCREENWIDTH, SCREENHEIGHT);
    initLight();
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    display_normals = false;
    display_smooth_normals = false;
    display_mesh = true;
    display_transformed_mesh = true;
    displayMode = LIGHTED;

}

// ------------------------------------
// Rendering.
// ------------------------------------

void drawVector(Vec3 const &i_from, Vec3 const &i_to) {

    glBegin(GL_LINES);
    glVertex3f(i_from[0], i_from[1], i_from[2]);
    glVertex3f(i_to[0], i_to[1], i_to[2]);
    glEnd();
}

void drawAxis(Vec3 const &i_origin, Vec3 const &i_direction) {

    glLineWidth(4); // for example...
    drawVector(i_origin, i_origin + i_direction);
    glLineWidth(1); // for example...
}

//Fonction de dessin en utilisant les normales au sommet
void drawSmoothTriangleMesh(Mesh const &i_mesh) {
    glBegin(GL_TRIANGLES); //Fonction OpenGL de dessin de triangles
    //Iterer sur les triangles
    for(unsigned int tIt = 0 ; tIt < i_mesh.triangles.size(); ++tIt) {
        //Récupération des positions des 3 sommets du triangle pour l'affichage
        //Vertices --> liste indexée de sommets
        //i_mesh.triangles[tIt][i] --> indice du sommet vi du triangle dans la liste de sommet
        //pi --> position du sommet vi du triangle
        //ni --> normal du sommet vi du triangle pour un affichage lisse
        Vec3 p0 = i_mesh.vertices[i_mesh.triangles[tIt][0]];
        Vec3 n0 = i_mesh.normals[i_mesh.triangles[tIt][0]];

        Vec3 p1 = i_mesh.vertices[i_mesh.triangles[tIt][1]];
        Vec3 n1 = i_mesh.normals[i_mesh.triangles[tIt][1]];

        Vec3 p2 = i_mesh.vertices[i_mesh.triangles[tIt][2]];
        Vec3 n2 = i_mesh.normals[i_mesh.triangles[tIt][2]];

        //Passage des positions et normales à OpenGL
        glNormal3f( n0[0] , n0[1] , n0[2] );
        glVertex3f( p0[0] , p0[1] , p0[2] );
        glNormal3f( n1[0] , n1[1] , n1[2] );
        glVertex3f( p1[0] , p1[1] , p1[2] );
        glNormal3f( n2[0] , n2[1] , n2[2] );
        glVertex3f( p2[0] , p2[1] , p2[2] );
    }
    glEnd();

}

//Fonction de dessin en utilisant les normales au triangles
void drawTriangleMesh(Mesh const &i_mesh) {
    glBegin(GL_TRIANGLES);
    //Iterer sur les triangles
    for(unsigned int tIt = 0 ; tIt < i_mesh.triangles.size(); ++tIt) {
        //Récupération des positions des 3 sommets du triangle pour l'affichage
        //Vertices --> liste indexée de sommets
        //i_mesh.triangles[tIt][i] --> indice du sommet vi du triangle dans la liste de sommet
        //pi --> position du sommet vi du triangle
        Vec3 p0 = i_mesh.vertices[i_mesh.triangles[tIt][0]];
        Vec3 p1 = i_mesh.vertices[i_mesh.triangles[tIt][1]];
        Vec3 p2 = i_mesh.vertices[i_mesh.triangles[tIt][2]];

        //Normal au triangle
        Vec3 n = i_mesh.triangle_normals[tIt];

        glNormal3f( n[0] , n[1] , n[2] );

        glVertex3f( p0[0] , p0[1] , p0[2] );
        glVertex3f( p1[0] , p1[1] , p1[2] );
        glVertex3f( p2[0] , p2[1] , p2[2] );
    }
    glEnd();

}

//Fonction de dessin sans les normales
void drawFlatMesh(Mesh const &i_mesh) {
    glBegin(GL_TRIANGLES);
    for (unsigned int tIt = 0; tIt < i_mesh.triangles.size(); ++tIt) {
        Vec3 p0 = i_mesh.vertices[i_mesh.triangles[tIt][0]];
        Vec3 p1 = i_mesh.vertices[i_mesh.triangles[tIt][1]];
        Vec3 p2 = i_mesh.vertices[i_mesh.triangles[tIt][2]];

        glVertex3f(p0[0], p0[1], p0[2]);
        glVertex3f(p1[0], p1[1], p1[2]);
        glVertex3f(p2[0], p2[1], p2[2]);
    }
    glEnd();

}

void drawMesh(Mesh const &i_mesh) {

    if (display_smooth_normals){
        if(i_mesh.normals.size() > 0 )
            drawSmoothTriangleMesh(i_mesh); //Smooth display with vertices normals
        else
            drawFlatMesh (i_mesh);

    } else
        if(i_mesh.triangle_normals.size() > 0 )
            drawTriangleMesh(i_mesh); //Display with triangle normals
        else
            drawFlatMesh (i_mesh);

}

void drawVectorField(std::vector < Vec3 >
                     const & i_positions,
                     std::vector <Vec3> const &i_directions
                     ) {
    glLineWidth(1.);
    for( unsigned int pIt = 0; pIt<i_directions.size(); ++pIt ) {
        Vec3 to = i_positions[pIt] + 0.02 * i_directions[pIt];
        drawVector(i_positions[pIt], to );
    }
}

void drawNormals(Mesh const &i_mesh) {

    if (display_smooth_normals) {
        drawVectorField(i_mesh.vertices, i_mesh.normals);
    } else {
        std::vector <Vec3> triangle_baricenters;
        for (const Triangle &triangle: i_mesh.triangles) {
            Vec3 triangle_baricenter(0., 0., 0.);
            for (unsigned int i = 0; i < 3; i++)
                triangle_baricenter += i_mesh.vertices[triangle[i]];
            triangle_baricenter /= 3.;
            triangle_baricenters.push_back(triangle_baricenter);
        }

        drawVectorField(triangle_baricenters, i_mesh.triangle_normals);
    }
}

struct Point {
    float x;
    float y;
    float z;
};

Point* HermiteCubicCurve(Point P0, Point P1, Vec3 V0, Vec3 V1, long nbu) {

    Point* res = new Point[nbu];

    for(int i = 0; i < nbu; i++) {
        double u = (double)i / (double)(nbu-1);
        double u2 = u*u;
        double u3 = u2*u;
        res[i].x = (2*u3 - 3*u2 + 1)*P0.x + (-2*u3 + 3*u2)*P1.x + (u3 - 2*u2 + u)*V0[0] + (u3 - u2)*V1[0];
        res[i].y = (2*u3 - 3*u2 + 1)*P0.y + (-2*u3 + 3*u2)*P1.y + (u3 - 2*u2 + u)*V0[1] + (u3 - u2)*V1[1];
        res[i].z = (2*u3 - 3*u2 + 1)*P0.z + (-2*u3 + 3*u2)*P1.z + (u3 - 2*u2 + u)*V0[2] + (u3 - u2)*V1[2];
    }
    
    return res;

}
Point* BezierCurveByBernstein(Point* TabControlPoint, long nbControlPoint, long nbU){
    
}

void drawCurve(Point* curvePoints, long nbPoints) {
    glBegin(GL_LINE_STRIP);
    glColor3f(0.5,0.5,0.5);
    for(int i = 0; i < nbPoints; i++) {
        glVertex3f(curvePoints[i].x, curvePoints[i].y, curvePoints[i].z);
    }
    glEnd();
}

//Draw fonction
void draw() {

    if (displayMode == LIGHTED || displayMode == LIGHTED_WIRE) {

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);

    } else if (displayMode == WIRE) {

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_LIGHTING);

    } else if (displayMode == SOLID) {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }

    if (display_mesh) {
        int NbP=10;
        Point P0;
        P0.x=0.f;
        P0.y=0.f;
        P0.z=0.f;
        Point P1;
        P1.x=2.f;
        P1.y=0.f;
        P1.z=0.f;
        Vec3 V0=Vec3(1,1,0);
        Vec3 V1=Vec3(1,-1,0);
        drawCurve(HermiteCubicCurve(P0,P1,V0,V1, NbP),NbP);
    }
    if (display_transformed_mesh) {
        glBegin(GL_LINE_STRIP);
        glColor3f(1, 0, 0);
        glVertex3f(0,0,0);
        glVertex3f(10,0,0);
        glEnd();
        glBegin(GL_LINE_STRIP);
        glColor3f(0, 1, 0);
        glVertex3f(0,0,0);
        glVertex3f(0,10,0);
        glEnd();
        glBegin(GL_LINE_STRIP);
        glColor3f(0, 0, 1);
        glVertex3f(0,0,0);
        glVertex3f(0,0,10);
        glEnd();
    }

    if (displayMode == SOLID || displayMode == LIGHTED_WIRE) {
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glPolygonOffset(-2.0, 1.0);

        glColor3f(0., 0., 0.);
        if (display_mesh) {
            drawMesh(mesh);
        }
        if (display_transformed_mesh) {
            drawMesh(transformed_mesh);
        }

        glDisable(GL_POLYGON_OFFSET_LINE);
        glEnable(GL_LIGHTING);
    }

    glDisable(GL_LIGHTING);
    if (display_normals) {
        glColor3f(1., 0., 0.);
        if (display_mesh) {
            drawNormals(mesh);
        }
        if (display_transformed_mesh) {
            drawNormals(transformed_mesh);
        }
    }

    glEnable(GL_LIGHTING);

}

void changeDisplayMode() {
    if (displayMode == LIGHTED)
        displayMode = LIGHTED_WIRE;
    else if (displayMode == LIGHTED_WIRE)
        displayMode = SOLID;
    else if (displayMode == SOLID)
        displayMode = WIRE;
    else
        displayMode = LIGHTED;
}

void display() {
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply();
    draw();
    glFlush();
    glutSwapBuffers();
}

void idle() {
    glutPostRedisplay();
}

// ------------------------------------
// User inputs
// ------------------------------------
//Keyboard event
void key(unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow(SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen();
            fullScreen = true;
        }
        break;


    case 'w': //Change le mode d'affichage
        changeDisplayMode();
        break;

    case 'n': //Press n key to display normals
        display_normals = !display_normals;
        break;

    case '1': //Toggle loaded mesh display
        display_mesh = !display_mesh;
        break;

    case '2': //Toggle transformed mesh display
        display_transformed_mesh = !display_transformed_mesh;
        break;

    case 's': //Switches between face normals and vertices normals
        display_smooth_normals = !display_smooth_normals;
        break;

    default:
        break;
    }
    idle();
}

//Mouse events
void mouse(int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate(x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }

    idle();
}

//Mouse motion, update camera
void motion(int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate(x, y);
    } else if (mouseMovePressed == true) {
        camera.move((x - lastX) / static_cast<float>(SCREENWIDTH), (lastY - y) / static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    } else if (mouseZoomPressed == true) {
        camera.zoom(float(y - lastZoom) / SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize(w, h);
}

// ------------------------------------
// Start of graphical application
// ------------------------------------
int main(int argc, char **argv) {
    if (argc > 2) {
        exit(EXIT_FAILURE);
    }
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow("TP HAI702I");

    init();
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutReshapeFunc(reshape);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);
    key('?', 0, 0);

    //Mesh loaded with precomputed normals
    openOFF("../../data/elephant_n.off", mesh.vertices, mesh.normals, mesh.triangles, mesh.triangle_normals);

    //Ajout des sommets et triangles au maillage gris (i.e. le maillage transformé)
    transformed_mesh.vertices = mesh.vertices;
    transformed_mesh.triangles = mesh.triangles;

    //Calcul des normales
    transformed_mesh.computeNormals();

    //Appliquer une matrice de transformation aux points
    //Changer la matrice pour appliquer une transformation aux sommets du maillage gris
    Mat3 transformation = Mat3 (10., 0., 0.,
                                0., 1., 0.,
                                0., 0., 5.);
    //Add a translation
    Vec3 translation = Vec3(10., 0., 0.);

    for (unsigned int i = 0; i < transformed_mesh.vertices.size(); i++) {
        transformed_mesh.vertices[i] = transformation*transformed_mesh.vertices[i] + translation;
    }

    //Utilisation de la matrice de transformation pour transformer les normales du maillage
    //Que constatez-vous si vous appliquez une mise à l'echelle non-uniforme ?
    /*
    Il y a des problèmes lors de l'affichage et la norme
    */
    for (unsigned int i = 0; i < transformed_mesh.normals.size(); i++) {
        transformed_mesh.triangle_normals[i] = transformation*transformed_mesh.triangle_normals[i];
        transformed_mesh.normals[i] = transformation*transformed_mesh.normals[i];
    }

    glutMainLoop();
    return EXIT_SUCCESS;
}

