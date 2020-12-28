#include <GL/glut.h>    
#include <AR/gsub.h>    
#include <AR/video.h>   
#include <AR/param.h>   
#include <AR/ar.h>
#include <AR/arMulti.h>
#include <math.h>
#include <vlc/vlc.h>

#include "../include/utils.h"

ARMultiMarkerInfoT *mMarker;
TObject *objects = NULL;
libvlc_instance_t *inst;
libvlc_media_player_t *mp;
libvlc_media_t *m;
int nobjects = 0;
mode_t mode = MODE_LIBRE;
uint8_t isPlaying = 0;
static double angle = 0;

void print_error(char *error) { 
    printf("%s\n", error);
    exit(0);
}

void cleanup() {
    arVideoCapStop();
    arVideoClose();
    argCleanup();
    exit(0);
}

void addObject(char *p, double w, double c[2], void (*drawme)(void)) {
    int pattid;

    if ((pattid = arLoadPatt(p)) < 0)
        print_error("Error en la carga de patron\n");
    
    nobjects++;
    objects = (TObject *)realloc(objects, sizeof(TObject) * nobjects);

    objects[nobjects-1].id = pattid;
    objects[nobjects-1].width = w;
    objects[nobjects-1].center[0] = c[0];
    objects[nobjects-1].center[1] = c[1];
    objects[nobjects-1].drawme = drawme;
}

static void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 0x1B:
        case 'Q':
        case 'q':
            cleanup();
            break;
    }
}

void libreFunc(void) {

}

void melodia1Func(void) {

}

void melodia2Func(void) {

}

void claveSolFunc(void) {

}

void playSound(int soundId) {
    if (isPlaying == 0) {
        m = libvlc_media_new_path(inst, "");
        mp = libvlc_media_player_new_from_media(m);
        libvlc_media_release(m);
        libvlc_media_player_play(mp);
        isPlaying = 1;
    }
}

void stopSound() {
    if (isPlaying == 1) {
        libvlc_media_player_stop(mp);
        isPlaying = 0;
    } 
}

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
    glLoadMatrix(gl_para);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    if (calculateDistance(objects[0].patt_trans, objects[3].patt_trans) < 90.0)
        mode = MODE_LIBRE;
    if (calculateDistance(objects[1].patt_trans, objects[3].patt_trans) < 90.0)
        mode = MODE_MELODIA1;
    if (calculateDistance(objects[2].patt_trans, objects[3].patt_trans) < 90.0)
        mode = MODE_MELODIA2;

    u_mod = sqrt(pow(objects[3].patt_trans[0][0], 2)
               + pow(objects[3].patt_trans[0][0], 2)
               + pow(objects[3].patt_trans[0][0], 2));

    ux = objects[3].patt_trans[0][0];
    angle = (acos(ux / u_mod)) * (180.0 / M_PI);

    if (mode == MODE_LIBRE) {
        for (i = 0; i < mMarker->marker_num; i++) {
            glPushMatrix();
            argConvGlpara(mMarker->marker[i].trans, gl_para);
            glMultMatrixd(gl_para);

            if (mMarker->marker[i].visible < 0) {
                material[0] = 1.0;
                material[1] = 0.0;
                material[2] = 0.0;
                playSound(i);
            } else {
                material[0] = 0.0;
                material[1] = 1.0;
                material[2] = 0.0;
                stopSound();
            }
            glMaterialfv(GL_FRONT, GL_AMBIENT, material);
            glTranslatef(0.0, 0.0, 25.0);
            glutSolidCube(50.0);
            glPopMatrix();
        }
    } else if (mode == MODE_MELODIA1) {

    } else if (mode == MODE_MELODIA2) {

    }
        
    glDisable(GL_DEPTH_TEST);
}

static void init(void) {
    ARParam wparam, cparam;
    int xsize, ysize;
    double center[2] = {0.0, 0.0};

    if (arVideoOpen("-dev=/dev/video2") < 0) exit(0);
    if (arVideoInqSize(&xsize, &ysize) < 0)  exit(0);

    if (arParamLoad("data/camera_para.dat", 1, &wparam) < 0)
        print_error("Error en carga de parametros de camara\n");

    arParamChangeSize(&wparam, xsize, ysize, &cparam);
    arInitCparam(&cparam);

    if ((mMarker = arMultiReadConfigFile("data/teclado.dat")) == NULL) 
        print_error("Error en fichero teclado.dat\n");

    addObject("data/libre.patt", 80.0, center, libreFunc);
    addObject("data/melodia_1.patt", 80.0, center, melodia1Func);
    addObject("data/melodia_2.patt", 80.0, center, melodia2Func);
    addObject("data/clave_sol.patt", 80.0, center, claveSolFunc);

    argInit(&cparam, 1.0, 0, 0, 0, 0);

}

static void initVLC(void) {
    inst = libvlc_new(0, NULL);
}

static void mainLoop(void) {
    ARUint8 *dataPtr;
    ARMarkerInfo *marker_info;
    int marker_num, i, j, k;

    if ((dataPtr = (ARUint8)arVideoGetImage()) == NULL) {
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

int main(int argc, char *argv) {
    glutInit(&argc, argv);
    init();
    initVLC();
    arVideoCapStart();
    argMainLoop(NULL, keyboard, mainLoop);
    return 0;
}