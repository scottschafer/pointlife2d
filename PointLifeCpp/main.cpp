#include <cstdio>
#include <cassert>
#include <vector>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "glm/glm.hpp"

#include "WorldThreadRunner.h"
#include "World.h"
#include "globals.h"

using namespace glm;

struct Context
{
    int width, height;
    GLuint vert_id, frag_id;
    GLuint prog_id, geom_id;
    GLint u_time_loc;
    
    enum { Position_loc, Color_loc };
    
    Context():
    width(400), height(300),
    vert_id(0), frag_id(0),
    prog_id(0), geom_id(0),
    u_time_loc(-1)
    {}
    
} g_context;

void init()
{
    printf("init()\n");
}

void resize(int width, int height)
{
    printf("resize(%d, %d)\n", width, height);
    
    g_context.width = width;
    g_context.height = height;
}

void draw()
{
    glViewport(0, 0, g_context.width, g_context.height);
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

void update()
{
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    printf("main()\n");
    
    glutInit(&argc, argv);
    glutInitWindowSize(g_context.width, g_context.height);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    
    glutReshapeFunc(resize);
    glutDisplayFunc(draw);
    glutIdleFunc(update);
    
    init();
    
    glutMainLoop();
    
    return 0;
}
