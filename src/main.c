#include <GL/glut.h>    
#include <AR/gsub.h>    
#include <AR/video.h>   
#include <AR/param.h>   
#include <AR/ar.h>
#include <AR/arMulti.h>

#include "../include/utils.h"

ARMultiMarkerInfoT *mMarker;
TObject *objects = NULL;
int nobjects = 0;

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

static void draw(void) {
    double gl_para[16];
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

    arVideoCapStart();
    argMainLoop(NULL, keyboard, mainLoop);
    return 0;
}