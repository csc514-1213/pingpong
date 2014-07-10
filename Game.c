
#include<GL/glut.h>
#include<stdio.h>
/*
 * @author Essiennta Emmanuel (colourfulemmanuel@gmail.com)
#include<unistd.h>
*/
#include<time.h>
#include<sys/time.h>
#include<string.h>
#include <math.h>
#include <unistd.h>

/*
 * configuration panel
 *      instructions
 *      settings
 *      color - foreground and background, etc
 * Accounting page in graphic mode
 *      score
 *      history
 */

#define RIGHT 1
#define LEFT 0
#define DOWN 1
#define UP 0

GLint clipWidth=400,clipHeight=400;
GLint isPaused=0;
GLint incrementX=1;
GLint incrementY=3;

GLint didCollide=1;
GLint numHits=0;


int vertical_pixel;
int horizontal_pixel;

int DEFAULT_VERTICAL_SLATED_TIME=2001;//the number of milliseconds taken to move vertically
int DEFAULT_HORIZONTAL_SLATED_TIME=3210;//the number of milliseconds taken to move horizontally

int slatedTimeForVerticalAnimation;
int slatedTimeForHorizontalAnimation;

long long verticalStartTimeForAnimation;

long long horizontalStartTimeForAnimation;

int verticalDistance;
int verticalDirection;

int horizontalDistance;
int horizontalDirection;

long long lastAnimationTime;

int gameEnded=0;

GLfloat COLOR_RED[]={1.0, 0.0, 0.0};
GLfloat COLOR_BLACK[]={0.0, 0.0, 0.0};
GLfloat COLOR_BLUE[]={0.0, 0.0, 1.0};
GLfloat COLOR_YELLOW[]={1.0, 1.0, 0.0};
GLfloat COLOR_TEXT[] = {0.1, 0.5, 0.2};
GLfloat COLOR_BUTTON_TEXT[] = {0.1, 1.0, 0.2};
GLfloat COLOR_BACKGROUND[] = {0.8, 0.8, 0.8};
GLfloat COLOR_BUTTON[] = {0.3, 0.3, 0.3};
GLfloat COLOR_COMP_FIELD[] = {0.1, 0.1, 0.1};
GLfloat COLOR_HUMAN_FIELD[] = {1.0, 0.41, 0.0};

int buttonY;
int buttonHeight;

int playAgainButtonX;
int playAgainButtonWidth;

int exitButtonX;
int exitButtonWidth;



GLint scorePanelWidth;
GLint scorePanelHeight;


struct{
    GLint x,y;
    GLsizei w,h;
    GLfloat* hitColor;
    GLint verticalTimeElapsedForAnimation;
}rect;

struct{
    GLint x, y;
    GLsizei w, h;
}ball;

struct{
    GLint x, y;
    GLsizei w, h;
}comp;

void paintRect(void){/*The human tab at the bottom*/
    glRecti(rect.x,rect.y,rect.x+rect.w-1,rect.y+rect.h-1);
}

void paintBall(void){/*The ball*/
    glColor3fv(COLOR_YELLOW);
    glBegin(GL_LINES);
    int L;
    for(L = 0; L <= ball.w/2; L++){
        GLdouble B = sqrt(ball.w*ball.w/4 - L*L);
        glVertex2d(ball.x+ball.w/2+-B,ball.y+ball.w/2+L);
        glVertex2d(ball.x+ball.w/2+B,ball.y+ball.w/2+L);
        glVertex2d(ball.x+ball.w/2+-B,ball.y+ball.w/2-L);
        glVertex2d(ball.x+ball.w/2+B,ball.y+ball.w/2-L);
    }
    glEnd();
/*
    glRecti(ball.x,ball.y,ball.x+ball.w-1,ball.y+ball.h-1);
*/
}
void paintComp(void){/*Computer tab at the top*/
    glColor3fv(COLOR_HUMAN_FIELD);
    glRecti(comp.x,comp.y,comp.x+comp.w-1,comp.y+comp.h-1);
}

void adjustComp(){
    comp.x=ball.x-(comp.w-ball.w>>1);
    if(comp.x<0)
        comp.x=0;
    else if(comp.x+comp.w>clipWidth)
        comp.x=clipWidth-comp.w;
}

GLint ballCollidedWithRect(){
    if(rect.y+rect.h==ball.y && ball.x+ball.w>rect.x && ball.x<rect.x+rect.w)
        return 1;
    return 0;
}



void startBackgroundMusic(){
    system("mpg123 -qCZ ~/Downloads/bgsound2.mp3 &");//Assuming you have  console-based player already installed
}
void killBackgroundMusic(){
    system("killall mpg123");
}

void faster(){
    slatedTimeForVerticalAnimation -= 500;//reduce speed by a tenth of a second
    slatedTimeForHorizontalAnimation -= 500;
}

long long getTimeInMillis(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    double time_in_millisecs = 
         (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000; // convert tv_sec & tv_usec to millisecond
    
    return (long long)time_in_millisecs;
}


void init(void){
    verticalStartTimeForAnimation = getTimeInMillis();
    horizontalStartTimeForAnimation = getTimeInMillis();

    verticalDirection = DOWN;
    horizontalDirection = RIGHT;
    
    slatedTimeForVerticalAnimation=DEFAULT_VERTICAL_SLATED_TIME;
    slatedTimeForHorizontalAnimation=DEFAULT_HORIZONTAL_SLATED_TIME;
    
    
    scorePanelWidth = clipWidth*2/3;
    scorePanelHeight = clipHeight >> 2;
    
    rect.x=rect.y=3;
    rect.w=70;
    rect.h=20;
    
    comp.w=70;
    comp.h=10;
    comp.y=clipHeight-comp.h;
    
    ball.h=ball.w=30;
    
    ball.x=0;
    ball.y=clipHeight-ball.h;
    
    verticalDistance = (clipHeight-(rect.y+rect.h)-comp.h-ball.h);
    horizontalDistance = (clipWidth - ball.w);
    
    rect.hitColor=COLOR_RED;
    
    glClearColor(COLOR_BACKGROUND[0],COLOR_BACKGROUND[1],COLOR_BACKGROUND[2],1.0);//Set display-window color to blue
/*
    glMatrixMode(GL_MODELVIEW);
*/
/*
    glPushMatrix();//duplicate identity matrix
*/
    
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0,clipWidth,0,clipHeight);
    
    
    /*background sound*/
    startBackgroundMusic();
    lastAnimationTime=getTimeInMillis();
}

void stopAnimation(){
    glutIdleFunc(NULL);
}
void showScore(){
    gameEnded=1;
    stopAnimation(); //stop the animation.
    glutPostRedisplay();//repaint screen
}

void animate(void){
    lastAnimationTime=getTimeInMillis();
    
    long long vTimeElapsedForAnimation=lastAnimationTime-verticalStartTimeForAnimation;
    if (vTimeElapsedForAnimation >= slatedTimeForVerticalAnimation) {
        vTimeElapsedForAnimation = 0;
        verticalStartTimeForAnimation = lastAnimationTime;
        verticalDirection ^= 1;
        if(verticalDirection==DOWN)
            system("mpg123 -q ~/Downloads/tennis3.mp3&");//Assuming you have this console-based player already installed
    }
    vertical_pixel=vTimeElapsedForAnimation*verticalDistance/slatedTimeForVerticalAnimation;
    
    
    long long hTimeElapsedForAnimation=lastAnimationTime-horizontalStartTimeForAnimation;
    if (hTimeElapsedForAnimation >= slatedTimeForHorizontalAnimation) {
        hTimeElapsedForAnimation = 0;
        horizontalStartTimeForAnimation = lastAnimationTime;
        horizontalDirection ^= 1;
    }
    horizontal_pixel=hTimeElapsedForAnimation*horizontalDistance/slatedTimeForHorizontalAnimation;
    
    
    if(verticalDirection == DOWN){
        vertical_pixel = verticalDistance - vertical_pixel;
    }
    
    vertical_pixel += (rect.y+rect.h);
    
    if(horizontalDirection == LEFT){
        horizontal_pixel = horizontalDistance - horizontal_pixel;
    }

    paintComp();
    paintBall();
    
    ball.y=vertical_pixel;
    ball.x=horizontal_pixel;
    
    
    /*
     * At this point, we wanna know if there was a collision.
     * So we run the collision test.
     * Now if the ball's base and the rectangle's top are collinear and there was no collision,
     * then the game just ended. Show the user his score.
     * Otherwise, if there was a collision, then just increment the user's score and show it.
     */
    
    didCollide=ballCollidedWithRect();
/*
    printf("didCollide = %d\n",didCollide);
*/
    
    if(didCollide){
        numHits++;
        system("mpg123 -q ~/Downloads/tennis5.mp3&");//Assuming you have this console-based player already installed
        printf("You have %d hits\n",numHits);
        if(numHits%10==0)faster();
    }else if(ball.y==rect.y+rect.h){
        //Game ended
        printf("Failed to collide\n");
        printf("Number of hits was %d\n",numHits);
        showScore();
    }
    
    adjustComp();
    glutPostRedisplay();//repaint
}

void continueAnimation(){
    long long lastVerticalDuration=lastAnimationTime-verticalStartTimeForAnimation;
    long long lastHorizontalDuration=lastAnimationTime-horizontalStartTimeForAnimation;
    
    verticalStartTimeForAnimation=getTimeInMillis()-lastVerticalDuration;
    horizontalStartTimeForAnimation=getTimeInMillis()-lastHorizontalDuration;
    gameEnded=0;
    glutIdleFunc(animate);
}

void getStringRepresentation(char*buf,int score){
    if(score==0)
        return;
    getStringRepresentation(buf,score/10);
    buf[strlen(buf)]=(char)('0'+score%10);
}

void writeText(int posX,int posY,char*ch){
    glRasterPos2i(posX,posY);
    int i;
    for (i = 0; i < strlen(ch); i++)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, ch[i]);
}

void drawBoard(){
    glColor3fv(COLOR_HUMAN_FIELD);
    glRecti(0,0,clipWidth,clipHeight>>1);
    
    
    glColor3fv(COLOR_COMP_FIELD);
    glRecti(0,clipHeight>>1,clipWidth,clipHeight);
    
    
}

void displayFcn(void){
    
    glClear(GL_COLOR_BUFFER_BIT);
    drawBoard();
    
    paintComp();
    paintBall();
    
    
    glColor3fv(COLOR_COMP_FIELD);
    /*Decide whether to paint the bottom rect*/
    if(didCollide){//paint to indicate a hit
        glColor3fv(rect.hitColor);
        rect.verticalTimeElapsedForAnimation=5;//Retain the color for 5 repaints
    }else if(rect.verticalTimeElapsedForAnimation){
        glColor3fv(rect.hitColor);
        --rect.verticalTimeElapsedForAnimation;
    }
    paintRect();


    char score[10]="";
    getStringRepresentation(score, numHits);
    glColor3fv(COLOR_TEXT);
    writeText(rect.x+(rect.w-8>>1), rect.y+(rect.h-13>>1),score);
    
    if(gameEnded){
        glColor3fv(COLOR_BLACK);
        GLint lowX=clipWidth - scorePanelWidth >> 1;
        GLint lowY=clipHeight - scorePanelHeight >> 1;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(lowX,lowY,0);
/*
        glRotatef(30,0,0,1);
        glTranslatef(lowX,lowY,0);
*/
        glRecti(0, 0, scorePanelWidth, scorePanelHeight);
        char yourScore[100]="Weldone, you made ";
        if(numHits)
            strcpy(yourScore + strlen(yourScore), score);
        else
            strcpy(yourScore + strlen(yourScore), "0");
        if(numHits!=1)
            strcpy(yourScore + strlen(yourScore), " points!");
        else
            strcpy(yourScore + strlen(yourScore), " point!");
        printf("strlen = %d\n",strlen(yourScore));
        
        char playAgain[]="Play Again";
        char exit[]="Exit";
        
        int verticalPad=5;
        int horizontalPad=5;
        buttonHeight=13 + 2*verticalPad;
        playAgainButtonWidth = strlen(playAgain)*8 + 2*horizontalPad;
        exitButtonWidth = strlen(exit)*8 + 2*horizontalPad;
        exitButtonX=scorePanelWidth-horizontalPad-exitButtonWidth;
        buttonY=verticalPad;
        playAgainButtonX=horizontalPad;
        
        
        glColor3fv(COLOR_BUTTON);//set button color
        
        /*draw 'play again' button*/
        glRectf(horizontalPad, verticalPad, horizontalPad+playAgainButtonWidth, verticalPad+buttonHeight);
        
        glColor3fv(COLOR_BUTTON);
        /*draw exit button*/
        glRecti(exitButtonX, verticalPad, exitButtonX+exitButtonWidth, verticalPad+buttonHeight);
        
        glColor3fv(COLOR_TEXT);
        /*Score - text*/
        writeText(scorePanelWidth-strlen(yourScore)*8>>1,(verticalPad+buttonHeight)+(scorePanelHeight-verticalPad-buttonHeight-13>>1),yourScore);
        
        glColor3fv(COLOR_BUTTON_TEXT);
        /*play again - text*/
        writeText(horizontalPad+horizontalPad, verticalPad+verticalPad, playAgain);
        /*exit - text*/
        writeText(exitButtonX+horizontalPad, verticalPad+verticalPad, exit);
        
        glPopMatrix();
    }

    glutSwapBuffers();
/*
    glFlush();
*/
}

void winReshapeFcn(GLint newWidth,GLint newHeight){
    printf("Entered with %d,%d\n",newWidth,newHeight);
    /*Reset viewport and projection parameters*/
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0,clipWidth,0.0,clipHeight);
    
    glViewport(newWidth-clipWidth>>1,newHeight-clipHeight>>1,clipWidth,clipHeight);
    
    
/*
    glMatrixMode(GL_MODELVIEW);
*/
    
    /*Reset display window size parameters*/
/*
    winWidth=newWidth;
    winHeight=newHeight;
*/
/*
    printf("Left with %d,%d\n",new)
*/
}

void keyFcn(GLubyte key,GLint xMouse,GLint yMouse){
    switch(key){
        case ' '://space bar for pausing the game
            if(gameEnded)
                return;
            isPaused^=1;
/*
            system(" ");
*/
/*
            printf(" ");//command to pause/play background music
*/
            if(isPaused){
                stopAnimation();
/*
                killBackgroundMusic();
*/
            }else{
                continueAnimation();
            }
            break;
        case 'E'://sink down
        case 'e'://for exit
            if(gameEnded){
                killBackgroundMusic();
/*
                printf("q");//command to quit the music player
*/
                exit(0);
            }
        default:
            break;
    }
}

void resetGame(){
    verticalStartTimeForAnimation = getTimeInMillis();
    horizontalStartTimeForAnimation = getTimeInMillis();
    
    verticalDirection = DOWN;
    horizontalDirection = RIGHT;
    
    slatedTimeForVerticalAnimation = DEFAULT_VERTICAL_SLATED_TIME;
    slatedTimeForHorizontalAnimation = DEFAULT_HORIZONTAL_SLATED_TIME;
    
    gameEnded=0;
    numHits=0;
    
    slatedTimeForVerticalAnimation=DEFAULT_VERTICAL_SLATED_TIME;
    slatedTimeForHorizontalAnimation=DEFAULT_HORIZONTAL_SLATED_TIME;
    
    
    rect.x=rect.y=3;
    comp.y=clipHeight-comp.h;
    
    ball.h=ball.w=30;
    
    ball.x=0;
    ball.y=clipHeight-ball.h;
    killBackgroundMusic();
    startBackgroundMusic();
    lastAnimationTime=getTimeInMillis();
    continueAnimation();
    glutPostRedisplay();
}

void onMouseClicked(int button,int state,int x,int y){
    if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
        y=clipHeight-y;
        x-=clipWidth-scorePanelWidth>>1;
        y-=clipHeight-scorePanelHeight>>1;
        
        printf("entered\n");
        printf("point is %d, %d\n",x,y);
        if(y>buttonY+buttonHeight || y<buttonY){
            printf("returned\n");
            return;
        }
        printf("continued %d \n",playAgainButtonX+playAgainButtonWidth);
        if(x>=playAgainButtonX && x<=playAgainButtonX+playAgainButtonWidth){
            printf("play again\n");
            resetGame();
        }
        else if(x>=exitButtonX && x<=exitButtonX+exitButtonWidth) {
            printf("exit\n");
            killBackgroundMusic();
            exit(0);
        }
    }
}


void specialKeyFcn(GLint specialKey,GLint xMouse,GLint yMouse){
    GLint increment=50;
    printf("Entered keyFcn\n");
    glColor3fv(COLOR_BACKGROUND);//Reset to background color
    switch(specialKey){
        case GLUT_KEY_HOME:
            printf("Pressed home key\n");
            break;
        case GLUT_KEY_RIGHT:
            if(isPaused)
                return;
            rect.x+=increment;
            if(rect.x+rect.w-1>clipWidth)
                rect.x=clipWidth-rect.w+1;
            printf("Moving left %d\n",rect.x);
            break;
        case GLUT_KEY_LEFT:
            if(isPaused)
                return;
            rect.x-=increment;
            if(rect.x<0)
                rect.x=0;
            printf("Moving right %d\n",rect.x);
            break;
        default:return;
    }
    glColor3fv(COLOR_YELLOW);//new position color
    paintRect();
    glFlush();
}

void main(int argc,char**argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(clipWidth,clipHeight);
    glutCreateWindow("PingPong Game [CSC514 - 2013/2014]");
    
    init();
    glutDisplayFunc(displayFcn);
    glutReshapeFunc(winReshapeFcn);
    glutKeyboardFunc(keyFcn);
    glutMouseFunc(onMouseClicked);
    glutSpecialFunc(specialKeyFcn);
    glutIdleFunc(animate);
    
    glutMainLoop();
}
