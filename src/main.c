#include <GL/glut.h>    
#include <AR/gsub.h>    
#include <AR/video.h>   
#include <AR/param.h>   
#include <AR/ar.h>
#include <AR/arMulti.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "utils.h"

ARMultiMarkerInfoT *mMarker;
TObject *objects = NULL;
int nobjects = 0;
pianoMode mode = MODE_GUITAR;
static double angle = 0;
uint8_t isPlaying[5] = {0, 0, 0, 0, 0};
pid_t processPlaying = -1;
char *extension = ".wav";
char *mediaFolder = "media/";

/**
 * @brief prints error messages
 * 
 * @param error Text message to be printed out
 */
void print_error(char *error) { 
    printf("%s\n", error);
    exit(0);
}

/**
 * @brief free resources from the system
 */
void cleanup() {
    arVideoCapStop();
    arVideoClose();
    argCleanup();
    exit(0);
}

/**
 * @brief function to add an object
 * 
 * @param p route to the .patt file
 * @param w width of the mark
 * @param c array of the center of the marker
 */
void addObject(char *p, double w, double c[2]) {
    int pattid;

    if ((pattid = arLoadPatt(p)) < 0)
        print_error("Error en la carga de patron\n");
    
    nobjects++;
    objects = (TObject *)realloc(objects, sizeof(TObject) * nobjects);

    objects[nobjects - 1].id        = pattid;
    objects[nobjects - 1].width     = w;
    objects[nobjects - 1].center[0] = c[0];
    objects[nobjects - 1].center[1] = c[1];
}

/**
 * @brief callback function to capture keyboard
 */
static void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 0x1B:
        case 'Q':
        case 'q':
            cleanup();
            break;
    }
}

/**
 * @brief function to play a sound
 * 
 * @param id identifier of the note to be played
 */
void playSound(uint8_t id) {
    char path[20];
    char note[5];
    char playingMode[10];
    uint8_t octave = 0;
    switch (id) {
        case 0:
            sprintf(note, "%s", "do");
            break;
        case 1:
            sprintf(note, "%s", "re");
            break;
        case 2:
            sprintf(note, "%s", "mi");
            break;
        case 3:
            sprintf(note, "%s", "fa");
            break;
        case 4:
            sprintf(note, "%s", "sol");
            break;
    }

    switch(mode) {
        case MODE_GUITAR:
            sprintf(playingMode, "%s_", "guitar");
            break;
        case MODE_WAVE:
            sprintf(playingMode, "%s_", "wave");
            break;
        case MODE_SYNTH:
            sprintf(playingMode, "%s_", "synth");
            break;
    }

    if (angle >= 80.0 && angle < 170.0)
        octave = 1;
    else if (angle >= 170.0 && angle <= 180.0)
        octave = 2;

    sprintf(path, "%s%s%s%d%s", mediaFolder, playingMode, note, octave, extension);

    isPlaying[id] = 1;

    switch (processPlaying = fork()) {
        case -1:
            fprintf(stderr, "Error al crear el proceso hijo\n");
            exit(1);
        case 0:
            if (execl("/usr/bin/aplay", "aplay", path, NULL) < 0) {
                perror("Error al reproducir el sonido\n");
            }
    } 
}

/**
 * @brief function to stop a sound
 * 
 * @param id note to be stopped
 */
void stopSound(uint8_t id) {
    if (isPlaying[id] == 1) {
        kill(processPlaying, SIGKILL);
        isPlaying[id] = 0;
    } 
}

/**
 * @brief function to draw on the marks
 */
static void draw(void) {
    double gl_para[16];
    GLfloat material[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light_position[] = {100.0, -200.0, 200.0, 0.0};
    int i;
    double ux, u_mod;

    argDrawMode3D();
    argDraw3dCamera(0, 0);
    glClear(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    argConvGlpara(mMarker->trans, gl_para);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(gl_para);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    if (calculateDistance(objects[0].patt_trans, objects[3].patt_trans) < 98.0)
        mode = MODE_GUITAR;
    if (calculateDistance(objects[1].patt_trans, objects[3].patt_trans) < 98.0)
        mode = MODE_WAVE;
    if (calculateDistance(objects[2].patt_trans, objects[3].patt_trans) < 98.0)
        mode = MODE_SYNTH;

    u_mod = sqrt(pow(objects[3].patt_trans[0][0], 2)
               + pow(objects[3].patt_trans[1][0], 2)
               + pow(objects[3].patt_trans[2][0], 2));

    ux = objects[3].patt_trans[0][0];
    angle = (acos(ux / u_mod)) * (180.0 / M_PI);

    printf("Angulo: %lf\n", angle);

    for (int i = 0; i < mMarker->marker_num; i++) {
        glPushMatrix();
        argConvGlpara(mMarker->marker[i].trans, gl_para);
        glMultMatrixd(gl_para);
        int markersPlaying = 0;
        if (mMarker->marker[i].visible < 0) {
            material[0] = 1.0;
            material[1] = 0.0;
            material[2] = 0.0;
            //playSound(i);
            for (int j = 0; j < 5; j++) 
                if (isPlaying[i] == 1) markersPlaying++;
            if (markersPlaying == 0)
                playSound(i);
        } else {
            material[0] = 0.0;
            material[1] = 1.0;
            material[2] = 0.0;
            stopSound(i);
        }

        glMaterialfv(GL_FRONT, GL_AMBIENT, material);
        glTranslatef(0.0, 0.0, 25.0);
        glutSolidCube(50.0);
        glPopMatrix();
    }
        
    glDisable(GL_DEPTH_TEST);
}

/**
 * @brief function to load marker files, open video stream and camera parameters
 */
static void init(void) {
    ARParam wparam, cparam;
    int xsize, ysize;
    double center[2] = {0.0, 0.0};

    if (arVideoOpen("-dev=/dev/video0") < 0) exit(0);
    if (arVideoInqSize(&xsize, &ysize) < 0)  exit(0);

    if (arParamLoad("data/camera_para.dat", 1, &wparam) < 0)
        print_error("Error en carga de parametros de camara\n");

    arParamChangeSize(&wparam, xsize, ysize, &cparam);
    arInitCparam(&cparam);

    if ((mMarker = arMultiReadConfigFile("data/teclado.dat")) == NULL) 
        print_error("Error en fichero teclado.dat\n");

    addObject("data/libre.patt", 80.0, center);
    addObject("data/melodia_1.patt", 80.0, center);
    addObject("data/melodia_2.patt", 80.0, center);
    addObject("data/clave_sol.patt", 80.0, center);

    argInit(&cparam, 1.0, 0, 0, 0, 0);

}

/**
 * @brief main loop function of the program
 */
static void mainLoop(void) {
    ARUint8 *dataPtr;
    ARMarkerInfo *marker_info;
    int marker_num, i, j, k;

    if ((dataPtr = (ARUint8 *)arVideoGetImage()) == NULL) {
        arUtilSleep(2);
        return;
    }

    argDrawMode2D();
    argDispImage(dataPtr, 0, 0);

    if (arDetectMarker(dataPtr, 100, &marker_info, &marker_num) < 0) {
        cleanup();
        exit(0);
    }
    
    arVideoCapNext();

    for (i = 0; i < nobjects; i++) {
        for (j = 0, k = -1; j < marker_num; j++) {
            if (objects[i].id == marker_info[j].id) {
                if (k == -1) k = j;
                else if (marker_info[k].cf < marker_info[j].cf) k = j;
            }
        }

        if (k != -1) {
            objects[i].visible = 1;
            objects[i].cf = marker_info[k].cf;
            arGetTransMat(&marker_info[k], objects[i].center, 
                            objects[i].width, objects[i].patt_trans);
        } else {
            objects[i].visible = 0;
        }
    }

    if (arMultiGetTransMat(marker_info, marker_num, mMarker) > 0)
        draw();

    argSwapBuffers();
}

/**
 * @brief Entrypoint of the program
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return int = 0 if everything was OK
 */
int main(int argc, char *argv) {
    glutInit(&argc, argv);
    init();

    arVideoCapStart();
    argMainLoop(NULL, keyboard, mainLoop);
    return 0;
}