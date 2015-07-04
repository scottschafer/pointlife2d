/* Use the following line on a Windows machine:
 #include <GL/glut.h>
 */
/* This line is for Max OSX  */
#include <GLUT/glut.h>

#include "WorldThreadRunner.h"
#include "World.h"

/*! GLUT display callback function */
void display(void);
/*! GLUT window reshape callback function */
void reshape(int, int);

const int WINDOW_SIZE = 768;
double magnification = 5;


void keyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:             // ESCAPE key
//            exit (0);
            break;
        case '+':
            magnification *= 1.1;
            break;
        case '-':
            magnification *= .9;
            break;
    }
}


int main(int argc, char** argv)
{
    WorldThreadRunner::start();

    glutInit(&argc, argv);
    
    /* set the window size to 512 x 512 */
    glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);
    
    /* set the display mode to Red, Green, Blue and Alpha
     allocate a depth buffer
     enable double buffering
     */
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    
    /* create the window (and call it Lab 1) */
    glutCreateWindow("Lab 1");
    
    /* set the glut display callback function
     this is the function GLUT will call every time
     the window needs to be drawn
     */
    glutDisplayFunc(display);
    
    /* set the glut reshape callback function
     this is the function GLUT will call whenever
     the window is resized, including when it is
     first created
     */
    glutReshapeFunc(reshape);

    glutKeyboardFunc (keyboardFunc);
    
    /* set the default background color to black */
    glClearColor(0,0,0,1);
    
    /* enter the main event loop so that GLUT can process
     all of the window event messages
     */
    glutMainLoop();
    
    return 0;
}

/*! glut display callback function.  Every time the window needs to be drawn,
 glut will call this function.  This includes when the window size
 changes, or when another window covering part of this window is
 moved so this window is uncovered.
 */
void display()
{
    glEnable( GL_POINT_SMOOTH );
    glEnable( GL_BLEND );
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, WINDOW_SIZE, WINDOW_SIZE);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    double mult = double(WINDOW_SIZE) / double(WORLD_DIM);
        
    float pointSize = mult * CELL_SIZE;

    /* clear the color buffer (resets everything to black) */
    glClear(GL_COLOR_BUFFER_BIT);
    
    /* set the current drawing color to red */
//    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
    glClearColor( 0, 0, 0, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    World & world = WorldThreadRunner::getWorld();
    Cell * cells = world.mCells;

    double xo = 0;
    double yo = 0;
    
    // the fraction of the screen that's magnified
    double magnifyScreenFraction = .5;
    
    // the coordinates of the displayed magnfication rect
    double screenMagDestX1 = WINDOW_SIZE * (1 - magnifyScreenFraction) / 2;
    double screenMagDestY1 = WINDOW_SIZE * (1 - magnifyScreenFraction) / 2;
    double screenMagDestX2 = WINDOW_SIZE * (1 + magnifyScreenFraction) / 2;
    double screenMagDestY2 = WINDOW_SIZE * (1 + magnifyScreenFraction) / 2;

    // the source coordinates of the magnfication rect
    /*
    double screenMagSrcX1 = WINDOW_SIZE * (1 - magnifyScreenFraction/magnification) / 2;
    double screenMagSrcY1 = WINDOW_SIZE * (1 - magnifyScreenFraction/magnification) / 2;
    double screenMagSrcX2 = WINDOW_SIZE * (1 + magnifyScreenFraction/magnification) / 2;
    double screenMagSrcY2 = WINDOW_SIZE * (1 + magnifyScreenFraction/magnification) / 2;
    */
    for (int pass = 0; pass < 2; pass++) {
        
        glPointSize(pass ? (pointSize*magnification) : pointSize);
        
        glBegin( GL_POINTS );
        for (int i = 0; i < NUM_CELLS; i++) {
            
            double x = cells[i].mPos.x * mult + xo;
            double y = cells[i].mPos.y * mult + yo;
            
            if (pass == 1) {
                x = (x - WINDOW_SIZE/2) + WINDOW_SIZE/2;
                y = (y - WINDOW_SIZE/2) + WINDOW_SIZE/2;
                if (x < screenMagDestX1 || x > screenMagDestX2 || y < screenMagDestY1 || y > screenMagDestY2) {
                    continue;
                }
            }

            if (cells[i].mLastCollision) {
                glColor3f(1, 0, 0);
            }
            else {
                glColor3f(1, 1, 1);
            }
            glVertex2f(x, y);
        }
        glEnd();

        glLineWidth(.2 * (pass ? magnification : 1));
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        for (int i = 0; i < NUM_CELLS; i++) {
            Cell & cell = cells[i];
            for (int j = 0; j < cell.mNumConnections; j++) {
                double x1 = cells[i].mPos.x * mult + xo;
                double y1 = cells[i].mPos.y * mult + yo;
                double x2 = cells[i].mConnections[j]->mPos.x * mult + xo;
                double y2 = cells[i].mConnections[j]->mPos.y * mult + yo;

                if (pass == 1) {
                x1 = (x1 - WINDOW_SIZE/2) + WINDOW_SIZE/2;
                y1 = (y1 - WINDOW_SIZE/2) + WINDOW_SIZE/2;
                x2 = (x2 - WINDOW_SIZE/2) + WINDOW_SIZE/2;
                y2 = (y2 - WINDOW_SIZE/2) + WINDOW_SIZE/2;
                }
                if (cell.mOwnsConnection[j]) {
                    glVertex2f(x1, y1);
                    glVertex2f(x2, y2);
                }
            }
        }
        glEnd();
        
        if (pass == 0) {
            glBegin(GL_QUADS);
            glColor3d(0,0,0);
            glVertex2f(screenMagDestX1, screenMagDestY1);
            glVertex2f(screenMagDestX2, screenMagDestY1);
            glVertex2f(screenMagDestX2, screenMagDestY2);
            glVertex2f(screenMagDestX1, screenMagDestY2);
            glEnd();

            glColor3f(1.0, 1.0, 1.0);
            glBegin(GL_LINES);
            glLineWidth(1);
            glVertex2f(screenMagDestX1, screenMagDestY1);
            glVertex2f(screenMagDestX2, screenMagDestY1);

            glVertex2f(screenMagDestX2, screenMagDestY1);
            glVertex2f(screenMagDestX2, screenMagDestY2);

            glVertex2f(screenMagDestX2, screenMagDestY2);
            glVertex2f(screenMagDestX1, screenMagDestY2);

            glVertex2f(screenMagDestX1, screenMagDestY2);
            glVertex2f(screenMagDestX1, screenMagDestY1);
            glEnd();

            
            glScissor(screenMagDestX1, screenMagDestY1,
                      screenMagDestX2-screenMagDestX1, screenMagDestY2-screenMagDestY1);
            glLineWidth(.2 * (pass ? magnification : 1));
            
            mult *= magnification;
        }
    }
    
    glFinish();
    glutSwapBuffers();

    glutPostRedisplay();
}

/*! glut reshape callback function.  GLUT calls this function whenever
 the window is resized, including the first time it is created.
 You can use variables to keep track the current window size.
 */
void reshape(int width, int height)
{
    /* tell OpenGL we want to display in a recangle that is the
     same size as the window
     */
    glViewport(0,0,width,height);
    
    /* switch to the projection matrix */
    glMatrixMode(GL_PROJECTION);
    
    /* clear the projection matrix */
    glLoadIdentity();
    
    /* set the camera view, orthographic projection in 2D */
    gluOrtho2D(0,width,0,height);
    
    /* switch back to the model view matrix */
    glMatrixMode(GL_MODELVIEW);
}
