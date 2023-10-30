#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define X_LOWER -180
#define X_UPPER 180

#define Y_LOWER -90
#define Y_UPPER 90

#define CLUSTER_AREA 20 

typedef double f64;

typedef struct {
  f64 x;
  f64 y;
} Point;

typedef struct {
  Point center;

  f64 x_min;
  f64 x_max;

  f64 y_min;
  f64 y_max;
} Cluster;


static f64 random(f64 min, f64 max) {
  f64 range = max - min;
  f64 div = RAND_MAX / range;
  return min + (rand() / div);
}

static Cluster* cluster_init(Cluster* c) {
  f64 x = random(X_LOWER + CLUSTER_AREA, X_UPPER - CLUSTER_AREA);
  f64 y = random(Y_LOWER + CLUSTER_AREA, Y_UPPER - CLUSTER_AREA);

  assert(x > X_LOWER && x < X_UPPER);
  assert(y > Y_LOWER && y < Y_UPPER);

  c->center = {x, y};

  c->x_min = x - CLUSTER_AREA;
  c->y_min = y - CLUSTER_AREA;
  c->x_max = x + CLUSTER_AREA;
  c->y_max = y + CLUSTER_AREA;

  return c;
}

static void cluster_print(Cluster* c) {
  printf("Cluster:\n");
  printf("\tCenter: (%f, %f).\n", c->center.x, c->center.y);
  printf("\tX_min: %f.\n", c->x_min);
  printf("\tY_min: %f.\n", c->y_min);
  printf("\tX_max: %f.\n", c->x_max);
  printf("\tY_max: %f.\n", c->y_max);
}

static void sample_points(Cluster* c, size_t n_points, Point result[]) {
  for (size_t i = 0; i < n_points; i++) {
    f64 x = random(c->x_min, c->x_max);
    f64 y = random(c->y_min, c->y_max);
    Point p = {x, y};
    result[i] = p;
  }
}

static void print_points(size_t n_points, Point points[]) {
  for (size_t i = 0; i < n_points; i++) {
    printf("(%f, %f)\n", points[i].x, points[i].y);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <seed>\n, <number of points>", argv[0]);
    return EXIT_FAILURE;
  }

  srand(atoi(argv[1]));

  size_t n_points = atoi(argv[2]);

  Point* points_c1 = (Point*)malloc(sizeof(Point) * n_points);
  Point* points_c2 = (Point*)malloc(sizeof(Point) * n_points);

  Cluster c1 = {};
  cluster_init(&c1);

  sample_points(&c1, n_points, points_c1);

  Cluster c2 = {};
  cluster_init(&c2);

  sample_points(&c2, n_points, points_c2);

  cluster_print(&c1);
  cluster_print(&c2);

  free(points_c1);
  free(points_c2);

  return EXIT_SUCCESS;
}
