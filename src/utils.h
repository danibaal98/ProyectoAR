typedef struct {
    int id;
    int visible;
    double cf;
    double width;
    double center[2];
    double patt_trans[3][4];
    void (* drawme)(void);
} TObject;

typedef enum {
    MODE_MELODIA1,
    MODE_MELODIA2,
    MODE_LIBRE
} pianoMode;

typedef enum {
    PLAYING,
    STOP
} playingState;

#define MELODIA1_LENGTH 12

double calculateDistance(double patt_trans1[3][4], double patt_trans2[3][4]);