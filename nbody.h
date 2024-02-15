#include <GL/glut.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>



#define G  0.000006  // Gravitational constant ofr this sim
#define SHOW_PATHS 1 // ideally only show paths for <50 bodies , else it gets very slow
#define MAX_POINTS 1000 // Maximum number of points per body
#define USE_OPENMP 1 
#define STAR_MASS 100000.0
#define ORBITAL_SPEED_CONSTANT 2000


typedef struct {
    double x, y, z; 
    double vx, vy, vz; 
    double mass; 
} Body;


typedef struct {
    double x, y, z;
} Point;


typedef struct{
    Body* bodies;
    double* body_colors;
    Point* paths;
    int* numPoints;

} DisplayData;