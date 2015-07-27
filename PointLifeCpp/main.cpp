/* Use the following line on a Windows machine:
 #include <GL/glut.h>
 */
/* This line is for Max OSX  */
#include <GLUT/glut.h>
#include "WorldThreadRunner.h"
#include "World.h"
#include "globals.h"
#include <vector>

/*! GLUT display callback function */
void display(void);
/*! GLUT window reshape callback function */
void reshape(int, int);

const int WINDOW_SIZE = 768;

void keyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:             // ESCAPE key
            break;
        case 'f':
            Globals::inFitnessTest = ! Globals::inFitnessTest;
            break;
            
        case '+':
            Globals::magnification *= 1.1;
            break;
        case '-':
            Globals::magnification *= .9;
            break;
            
        case '[':
            Globals::minMsPerTurn += 1;
            break;
            
        case ']':
            Globals::minMsPerTurn = MAX(0, Globals::minMsPerTurn - 1);
            
            break;
    }
}


int main(int argc, char** argv)
{
#if USE_SIMULATION_THREAD
    WorldThreadRunner::start();
#endif
    
    glutInit(&argc, argv);
    
    /* set the window size*/
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

inline NUMBER w2s(NUMBER c, NUMBER m = 1.0) {
    return (c-WORLD_DIM/2) * (NUMBER(WINDOW_SIZE)/NUMBER(WORLD_DIM)) * m + WINDOW_SIZE / 2.0;
}

void DrawString(int x, int y, char *string);
void DrawString(int x, int y, char *string)
{
    int len, i;
    
    glRasterPos2f(x, y);
    len = (int) strlen(string);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
    }
}

/*! glut display callback function.  Every time the window needs to be drawn,
 glut will call this function.  This includes when the window size
 changes, or when another window covering part of this window is
 moved so this window is uncovered.
 */
class GLColor  {
public:
    GLColor() {}
    GLColor(GLdouble r, GLdouble g, GLdouble b) : r(r), g(g), b(b) {}
    GLColor(int r, int g, int b) : r(r), g(g), b(b) {}
    GLdouble r, g, b;
};

void display()
{
    World & world = WorldThreadRunner::getWorld();
    int generation, testIndex, entityLength = 0;
    
    Cell * cells = world.mCells;
    
    std::vector<Point> points;
    std::vector<Point> connectorLines;
    std::vector<Point> forceLines;
    std::vector<GLColor> colors;
    std::vector<bool> cellActive;
    {
        WorldThreadMutex wtm;
        generation = world.getGeneration();
        testIndex = world.getTestIndex();

        double offsetX = 0;
        double offsetY = 0;
        if (generation && Globals::minMsPerTurn > 0) {
            double length = 0;
            for (int i = 0; i < world.mNumCells; i++) {
                if (cells[i].mOnBoard && cells[i].mIndex == cells[0].mEntityIndex) {
                    offsetX += NUMBER(WORLD_DIM) / 2 - cells[i].mPos.x;
                    offsetY += NUMBER(WORLD_DIM) / 2 - cells[i].mPos.y;
                    ++length;
                }
            }
            
            length *= 1;
            offsetX /= length;
            offsetY /= length;
        }
        int firstEntityIndex = cells[0].mEntityIndex;
        
        for (int i = 0; i < world.mNumCells; i++) {
            Cell & cell = cells[i];
            
            if (cell.mOnBoard) {
                
                cellActive.push_back(cell.mActivated);
                
                Point pos(cell.mPos.x + offsetX, cell.mPos.y + offsetY);
                
                if (cells[i].mEntityIndex != firstEntityIndex) {
                    colors.push_back(GLColor(0, 1, 0));
                }
                else {
                    ++entityLength;
                    if (cells[i].mLastFlagellum) {
                        colors.push_back(GLColor(0, 0, 1));
                        forceLines.push_back(pos);
                        forceLines.push_back(Point(pos.x+cell.mFlagellumVector.x,
                                                   pos.y+cell.mFlagellumVector.y));
                        
                    }
                    else if (cells[i].mLastAte) {
                        colors.push_back(GLColor(1, 0, 0));
                    }
                    else if (cells[i].mAction == actionBite) {
                        colors.push_back(GLColor(.5f, 0.0f, 0.0f));
                    }
                    else if (cells[i].mAction == actionFlagellum) {
                        colors.push_back(GLColor(0, 0, 1));
                    }
                    else if (cells[i].mAction == actionLook) {
                        colors.push_back(GLColor(1, 1, 0));
                    }
                    //                else if (cells[i].mLastCollision) {
                    //                    colors.push_back(GLColor(1, 0, 0));
                    //                }
                    else {
                        colors.push_back(GLColor(1, 1, 1));
                    }
                }
                
                points.push_back(pos);

                for (int j = 0; j < cell.mNumConnections; j++) {
                    if (cell.mConnections[j]) {
                        connectorLines.push_back(pos);
                        Point pos2 = cell.mConnections[j]->mPos;
                        connectorLines.push_back(Point(pos2.x + offsetX, pos2.y + offsetY));
                    }
                }
            }
        }
    }
    
    glEnable( GL_POINT_SMOOTH );
    glEnable( GL_BLEND );
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, WINDOW_SIZE, WINDOW_SIZE);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    
    double magnification = 1.0;
    
    float pointSize = WINDOW_SIZE/WORLD_DIM * CELL_SIZE;

    /* clear the color buffer (resets everything to black) */
    glClear(GL_COLOR_BUFFER_BIT);
    
    glClearColor( 0, 0, 0, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    

    double xo = 0;//(WINDOW_SIZE/2) * mult;
    double yo = 0;//(WINDOW_SIZE/2) * mult;
    
    
    // the fraction of the screen that's magnified
    double magnifyScreenFraction = generation ? 1 : .5;
    
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

        glLineWidth(.2 * (pass ? magnification : 1));
        glColor4f(1.0, 1.0, 1.0, .75);
        glBegin(GL_LINES);
        for (int i = 0; i < connectorLines.size(); i += 2) {
            double x1 = w2s(connectorLines[i].x, magnification);
            double y1 = w2s(connectorLines[i].y, magnification);
            double x2 = w2s(connectorLines[i+1].x, magnification);
            double y2 = w2s(connectorLines[i+1].y, magnification);
            
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
        }
        glEnd();
        

        
        glPointSize(pointSize*magnification * 1.25);
        glBegin( GL_POINTS );
        for (int i = 0; i < points.size(); i++) {
            
            
            glColor4f(1, 1, 1, cellActive[i] ? .75 : .25);
            double x = w2s(points[i].x, magnification);
            double y = w2s(points[i].y, magnification);

            glVertex2f(x, y);
        }
        glEnd();
        
        
        
        glPointSize(pointSize*magnification);
        glBegin( GL_POINTS );
        
        for (int fpass = 0; fpass < 2; fpass++) {
            for (int i = 0; i < points.size(); i++) {
                
                double x = w2s(points[i].x, magnification);
                double y = w2s(points[i].y, magnification);
                
                if (pass == 1) {
                    x = (x - WINDOW_SIZE/2) + WINDOW_SIZE/2;
                    y = (y - WINDOW_SIZE/2) + WINDOW_SIZE/2;
                    if (x < screenMagDestX1 || x > screenMagDestX2 || y < screenMagDestY1 || y > screenMagDestY2) {
                        continue;
                    }
                }
                
                GLColor c = colors[i];
                
                if (fpass == 0 || (c.r == 0 && c.g == 0 && c.b == 1)) {
                    glColor3f(c.r, c.g, c.b);
                    glVertex2f(x, y);
                }
            }
        }
        glEnd();

        
        glColor4f(.3, .3, 1.0, .85);
        glBegin(GL_LINES);
        glLineWidth(.4 * (pass ? magnification : 1));
        for (int i = 0; i < forceLines.size(); i += 2) {
            double x1 = w2s(forceLines[i].x, magnification);
            double y1 = w2s(forceLines[i].y, magnification);
            double x2 = w2s(forceLines[i+1].x, magnification);
            double y2 = w2s(forceLines[i+1].y, magnification);
            
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
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
            magnification = Globals::magnification;
            glLineWidth(.2 * magnification);
            
        }
    }
    
    if (generation) {
        char buffer[200];
        sprintf(buffer, "Generation #%d, index = #%d, length = %d", generation, testIndex, entityLength);
        glColor3f(1, 1, 1);
        DrawString(20, 20, buffer);
    }
    
    glFinish();
    glutSwapBuffers();

    glutPostRedisplay();
    
#if !USE_SIMULATION_THREAD
//    for (int i = 0; i < 100; i++)
        WorldThreadRunner::getWorld().turnCrank();
#endif
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
