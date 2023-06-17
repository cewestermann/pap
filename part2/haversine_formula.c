#include <stdio.h>
#include <tgmath.h>

typedef double f64;

static f64 square(f64 a) {
    return a * a;
}

static f64 radians_from_degrees(f64 degrees) {
    f64 result = 0.01745329251994329577 * degrees;
    return result;
}

static f64 reference_haversine(f64 x0, f64 y0, f64 x1, f64 y1, f64 earth_radius) {
    f64 lat1 = y0;
    f64 lat2 = y1;
    f64 lon1 = x0;
    f64 lon2 = x1;

    f64 d_lat = radians_from_degrees(lat2 - lat1);
    f64 d_lon = radians_from_degrees(lon2 - lon1);

    lat1 = radians_from_degrees(lat1);
    lat2 = radians_from_degrees(lat2);

    f64 a = square(sin(d_lat / 2.0)) + cos(lat1) * cos(lat2) * square(sin(d_lon / 2));
    f64 c = 2.0 * asin(sqrt(a));

    f64 result = earth_radius * c;

    return result;
}
