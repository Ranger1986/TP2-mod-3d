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
bool display_surface_cylindrique;
bool display_surface_reglee;
bool display_surface_bezier;

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
    display_surface_cylindrique = false;
    display_surface_reglee = false;
    display_surface_bezier = true;
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


long fact(long n){
    long f = 1;
    for (n; n>0;--n){
        f*=n;
    }
    return f;
}
Point* BezierCurveBernstein(Point* ctrlPts, long nbctrlPts, long nbu) {

    Point* res = new Point[nbu];

    long deg = nbctrlPts - 1;

    for(int i = 0; i < nbu; i++) {
        double u = (double)i / (double)(nbu-1);
        for(int j = 0; j <= deg; j++) {
            double polynome = ((double)fact(deg)/(float)(fact(j)*(fact(deg-j)))) * pow(u,j) * pow((1-u),deg-j);
            res[i].x += polynome * ctrlPts[j].x;
            res[i].y += polynome * ctrlPts[j].y;
            res[i].z += polynome * ctrlPts[j].z;
        }
    }

    return res;

}

void drawCurve(Point* curvePoints, long nbPoints, float* c) {
    glBegin(GL_LINE_STRIP);
    glColor3f(c[0],c[1],c[2]);
    for(int i = 0; i < nbPoints; i++) {
        glVertex3f(curvePoints[i].x, curvePoints[i].y, curvePoints[i].z);
    }
    glEnd();
}
/*
void drawQuad(Point* Points, long v, long u) {
    for (int i = 0; i<u+2; ++i){
        for (int j = 0; j<v+2; ++j){
            glBegin(GL_QUADS);
            glColor3f(0.5,0.5,0.5);
            Point P;
            P.x= Points[i*v+j].x;
            P.y= Points[i*v+j].y;
            P.z= Points[i*v+j].z;
            glVertex3f(P.x, P.y, P.z);
            P.x= Points[i*v+j+1].x;
            P.y= Points[i*v+j+1].y;
            P.z= Points[i*v+j+1].z;
            glVertex3f(P.x, P.y, P.z);
            P.x= Points[(i+1)*v+j].x;
            P.y= Points[(i+1)*v+j].y;
            P.z= Points[(i+1)*v+j].z;
            glVertex3f(P.x, P.y, P.z);
            P.x= Points[(i+1)*v+j+1].x;
            P.y= Points[(i+1)*v+j+1].y;
            P.z= Points[(i+1)*v+j+1].z;
            glVertex3f(P.x, P.y, P.z);
            glEnd();
        }
    }
}
*/
void drawLine(Point A, Point B, float* c){
    glBegin(GL_LINE_STRIP);
    glColor4f(c[0],c[1],c[2],0.5);
    glVertex3f(A.x,A.y,A.z);
    glVertex3f(B.x,B.y,B.z);
    glEnd();
}
void drawPoint(Point A, float* c){
    glBegin(GL_POINTS);
    glColor3f(c[0],c[1],c[2]);
    glVertex3f(A.x,A.y,A.z);
    glEnd();
}
Point* calcSurfCyl(Point* A, int v, int u, Vec3 d){
    Point* res= new Point[u*v];
    for (int i = 0; i<u; ++i){
        for (int j = 0; j<v; ++j){
            res[i*v+j].x= A[j].x + (d[0]*i/u);
            res[i*v+j].y= A[j].y + (d[1]*i/u);
            res[i*v+j].z= A[j].z + (d[2]*i/u);
        }
    }
    return res;
}
void drawSurfCyl(Point* A, int v, int u, float* c){
    for (int i = 0; i<u; ++i){
        Point bez[v];
        for (int j = 0; j<v; ++j){
            bez[j].x=A[i*v+j].x;
            bez[j].y=A[i*v+j].y;
            bez[j].z=A[i*v+j].z;
        }
        drawCurve(bez, v, c);
    }
    for (int j = 0; j<v; ++j){
        Point droite[2];
        droite[0].x=A[j].x;
        droite[0].y=A[j].y;
        droite[0].z=A[j].z;
        droite[1].x=A[v*(u-1)+j].x;
        droite[1].y=A[v*(u-1)+j].y;
        droite[1].z=A[v*(u-1)+j].z;
        drawCurve(droite, 2, c);
    }
}
void drawSurfReg(Point* A, int v, int u, float* c){
    for (int i = 0; i<u; ++i){
        Point bez[v];
        for (int j = 0; j<v; ++j){
            bez[j].x=A[i*v+j].x;
            bez[j].y=A[i*v+j].y;
            bez[j].z=A[i*v+j].z;
        }
        drawCurve(bez, v, c);
    }
    for (int i = 0; i<v; ++i){
        Point bez[u];
        for (int j = 0; j<u; ++j){
            bez[j].x=A[j*v+i].x;
            bez[j].y=A[j*v+i].y;
            bez[j].z=A[j*v+i].z;
        }
        drawCurve(bez, u, c);
    }    
    
}
Point* calcSurfReg(Point* A, Point* B, int v, int u){
    Point* res= new Point[u*v];
    for (int i = 0; i<u; ++i){
        for (int j = 0; j<v; ++j){
            res[i*v+j].x= A[j].x * (1-(i)/(u-1)) + B[j].x * ((i)/(u-1)) ;
            res[i*v+j].y= A[j].y * (1-(i)/(u-1)) + B[j].y * ((i)/(u-1)) ;
            res[i*v+j].z= A[j].z * (1-(i)/(u-1)) + B[j].z * ((i)/(u-1)) ;
        }
    }
    return res;
}
Point* BezierCurveDeCasteljau(Point* ctrlPts, long nbctrlPts, long nbu) {

    Point* res = new Point[nbu];
    

    for(int i = 0; i < nbu; i++) {
        double u = (double)i / (double)(nbu - 1);
        
        Point* Q = new Point[nbctrlPts]; // On repart du tableau de points de contrôle de base
            for(int j = 0; j < nbctrlPts; j++) {
                Q[j] = ctrlPts[j];
            }

        // De Casteljau
        for(int k = 1; k < nbctrlPts; k++) {
            for(int j = 0; j < nbctrlPts - k; j++) {
                //drawLine(Q[j],Q[j+1], blue);
                Q[j].x = (1 - u) * Q[j].x + u * Q[j + 1].x;
                Q[j].y = (1 - u) * Q[j].y + u * Q[j + 1].y;
                Q[j].z = (1 - u) * Q[j].z + u * Q[j + 1].z;
                //drawPoint(Q[j],red);
            }
        }
        res[i] = Q[0];
        
    }

    return res;

}
Point* calcSurfBez(Point* A, long v, long u, long newv, long newu){
    Point* surface= new Point[newv*newu];
    Point* BezierPrems = new Point[newv*u];
    for (int i = 0; i < u; i++)
    {
        Point* ligne= new Point[v];
        for (int j = 0; j < v; j++)
        {
            ligne[j]=A[i*v+j];
        }
        Point* B = BezierCurveDeCasteljau(ligne,v,newv);
        for (int j = 0; j < newv; j++)
        {
            BezierPrems[i*newv+j]=B[j];
        }
    }
    //float cB[]={0.5,0.5,0.f};
    //drawSurfReg(BezierPrems,newv,u, cB);
    for (int i = 0; i < newv; i++)
    {
        Point* ligne= new Point[u];
        for (int j = 0; j < u; j++)
        {
            ligne[j]=BezierPrems[j*newv+i];
        }
        Point* B = BezierCurveDeCasteljau(ligne,u,newu);
        for (int j = 0; j < newu; j++)
        {
            surface[j*newv+i]=B[j];
        }
    }
    
    return surface;
}
Point* genGrid(int u, int v){
    Point* grid= new Point[u*v];
    for (int i = 0; i < u; i++)
    {
        for (int j = 0; j < v; j++)
        {
            grid[i*5+j].x= rand() % 5 ;
            grid[i*5+j].y= i;
            grid[i*5+j].z= j;
        }
    }
    return grid;
}

Point* grid = genGrid(5,5);
//Draw fonction
void draw() {
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

    long NbPC=6;
    Point P[NbPC];
    P[0].x=0.f;
    P[0].y=0.f;
    P[0].z=0.f;

    P[1].x=0.5;
    P[1].y=1.f;
    P[1].z=0.f;

    P[2].x=1.f;
    P[2].y=2.f;
    P[2].z=0.f;

    P[3].x=-1.5;
    P[3].y=3.f;
    P[3].z=0.f;

    P[4].x=1.75;
    P[4].y=4.f;
    P[4].z=0.f;

    P[5].x=2.f;
    P[5].y=5.f;
    P[5].z=0.f;

    Point P2[NbPC];
    P2[0].x=-2.f;
    P2[0].y=0.f;
    P2[0].z=0.f;

    P2[1].x=1.f;
    P2[1].y=0.f;
    P2[1].z=1.f;

    P2[2].x=2.f;
    P2[2].y=0.f;
    P2[2].z=2.f;

    P2[3].x=-0.5;
    P2[3].y=0.f;
    P2[3].z=3.f;

    P2[4].x=1.f;
    P2[4].y=0.f;
    P2[4].z=4.f;

    P2[5].x=3.f;
    P2[5].y=0.f;
    P2[5].z=5.f;

    Vec3 vecteur_directeur= Vec3(1.f,0.f,0.f);
    Point* courbe_bezier1 = BezierCurveDeCasteljau(P, 6 ,10);
    Point* courbe_bezier2 = BezierCurveDeCasteljau(P2, 6 ,10);

    /*
    Point* pt2 = BezierCurveDeCasteljau(P2, 6 ,10);
    Point* listePtsR= calcSurfReg(pt, pt2, 10, 10);
    Point* listePts= calcSurfCyl(pt, 10, 2, dir1);
    Point* listePts2= calcSurfCyl(pt2, 10, 2, dir1);
    */

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

    if (display_surface_cylindrique) {
        Point* surface_cylindrique1= calcSurfCyl(courbe_bezier1, 10, 10, vecteur_directeur);
        float blanc[]={1.f,1.f,1.f};
        float jaune[]={0.5,0.5,0.f};
        drawCurve(courbe_bezier1, 10, blanc);
        drawSurfReg(surface_cylindrique1, 10, 10, jaune);
    }
    if (display_surface_reglee) {
        Point* surface_regle1= calcSurfReg(courbe_bezier1, courbe_bezier2, 10, 10);
        float blanc[]={1.f,1.f,1.f};
        float jaune[]={0.5,0.5,0.f};
        drawCurve(courbe_bezier1, 10, blanc);
        drawCurve(courbe_bezier2, 10, blanc);
        drawSurfReg(surface_regle1, 10, 10, jaune);
    }
    if (display_surface_bezier) {
        Point* surface_bezier1= calcSurfBez(grid,5,5, 10, 10);
        float blanc[]={1.f,1.f,1.f};
        float jaune[]={0.5,0.5,0.f};
        drawSurfReg(grid, 5, 5, blanc);
        drawSurfReg(surface_bezier1, 10, 10, jaune);
    }

    if (displayMode == SOLID || displayMode == LIGHTED_WIRE) {
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glPolygonOffset(-2.0, 1.0);

        glColor3f(0., 0., 0.);
        if (display_surface_cylindrique) {
            drawMesh(mesh);
        }
        if (display_surface_reglee) {
            drawMesh(transformed_mesh);
        }

        glDisable(GL_POLYGON_OFFSET_LINE);
        glEnable(GL_LIGHTING);
    }

    glDisable(GL_LIGHTING);
    if (display_normals) {
        glColor3f(1., 0., 0.);
        if (display_surface_cylindrique) {
            drawNormals(mesh);
        }
        if (display_surface_reglee) {
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
    case 'r':
        grid=genGrid(5,5);
        break;
    case '+':
        //NbP++;
        break;
    case '-':
        //if (NbP>6)NbP--;
        break;
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
    case '1': //Toggle loaded mesh display
        display_surface_cylindrique = !display_surface_cylindrique;
        break;

    case '2': //Toggle transformed mesh display
        display_surface_reglee = !display_surface_reglee;
        break;

    case '3': //Toggle transformed mesh display
        display_surface_bezier = !display_surface_bezier;
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

