#include <GL/glut.h>    
#include <AR/gsub.h>    
#include <AR/video.h>   
#include <AR/param.h>   
#include <AR/ar.h>

typedef struct {
    int id;
    int visible;
    double cf;
    double width;
    double center[2];
    double patt_trans[3][4];
    void (* drawme)(void);
} TObject;