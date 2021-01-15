typedef struct {
    int id;
    int visible;
    double cf;
    double width;
    double center[2];
    double patt_trans[3][4];
} TObject;

typedef enum {
    MODE_GUITAR,
    MODE_SYNTH,
    MODE_WAVE
} pianoMode;

typedef enum {
    PLAYING,
    STOP
} playingState;

#define MELODIA1_LENGTH 12

/**
 * @brief Fuction to calculate the distance between two marks
 * 
 * @param patt_trans1 Transformation matrix for the first mark
 * @param patt_trans2 Transformation matirs for the second mark
 * 
 * @return double With the distance between them
 */
double calculateDistance(double patt_trans1[3][4], double patt_trans2[3][4]);