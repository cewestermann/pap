#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define EARTH_RADIUS 6372.8

#define X_LOWER -180
#define X_UPPER 180

#define Y_LOWER -90
#define Y_UPPER 90

#define CLUSTER_AREA 20 

typedef double f64;

/* ----- Casey's reference haversine distance calculation ----- */
static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577f * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */
    
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    
    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}
/* ----- End of Casey's reference haversine distance calculation ----- */

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

errno_t write_json_and_results(Point* points_c1, Point* points_c2, size_t n_points) {
  FILE* json_file;
  FILE* binary_file;

  errno_t json_err = fopen_s(&json_file, "data.json", "w");
  if (json_err != 0) {
    char error_msg[100];
    strerror_s(error_msg, sizeof(error_msg), json_err);
    fprintf(stderr, "Failed to open JSON file for writing: %s\n", error_msg);
    return json_err;
  }

  errno_t binary_err = fopen_s(&binary_file, "results.f64", "wb");
  if (binary_err != 0) {
    char error_msg[100];
    strerror_s(error_msg, sizeof(error_msg), binary_err);
    fprintf(stderr, "Failed to open binary file for writing: %s\n", error_msg);
    return binary_err;
  }

  fprintf(json_file, "{\"pairs\":[\n");

  f64 distance_sum = 0;

  for (size_t i = 0; i < n_points; i++) {
    Point p1 = points_c1[i];
    Point p2 = points_c2[i];

    f64 distance = ReferenceHaversine(p1.x, p1.y, p2.x, p2.y, EARTH_RADIUS);
    size_t written = fwrite(&distance, sizeof(distance), 1, binary_file); // Bytes written
    if (written != 1) {
      fprintf(stderr, "Failed to write to binary file\n");
      fclose(json_file);
      fclose(binary_file);
      return EIO;
    }

    fprintf(json_file, " {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}", 
            p1.x, p1.y, p2.x, p2.y);

    if (i < n_points - 1)
      fprintf(json_file, ",\n");
    else
      fprintf(json_file, "\n");

    distance_sum += distance;
  }

  f64 avg_distance = distance_sum / (f64)n_points;

  fprintf(stdout, "Average Haversine distance: %.16f\n", avg_distance);

  fprintf(json_file, "]}");
  fclose(json_file);
  fclose(binary_file);
  return EXIT_SUCCESS; 
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

  errno_t err = write_json_and_results(points_c1, points_c2, n_points);
  if (err) {
    perror("Error writing JSON");
  }

  free(points_c1);
  free(points_c2);

  return EXIT_SUCCESS;
}
