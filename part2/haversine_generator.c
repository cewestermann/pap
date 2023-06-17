#include <stdio.h>
#include <stdlib.h>

#include "haversine_formula.c"

/* Define range of possible longitude and latitude degrees */
#define MAX_X 180
#define MIN_X -180

#define MAX_Y 90
#define MIN_Y -90

typedef struct Point {
    f64 x;
    f64 y;
} Point;

typedef struct Cluster {
    f64 center;
    f64 radius;
} Cluster;


/* Generate a random point within a cluster, given by a 
 * center and a radius. It does not take into account
 * the curvature */
Point get_random_point(Cluster cluster) {

}



int main(int argc, char* argv[]) {
    srand(12345); // Set seed to current time
    
    // Generate number between 0 and 5:
    int random_number = rand() % 50;
    printf("%d\n", random_number);

    return 0;
}



