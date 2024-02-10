#include <math.h>

// Utility function for
// converting degrees to radians
double toRadians(const double degree)
{
  double one_deg = (M_PI) / 180;
  return (one_deg * degree);
}

// Utility function for converting radians to degrees
double toDegrees(const double radian)
{
  double one_rad = 180 / (M_PI);
  return (one_rad * radian);
}

// calculates the distance between two lat/long points via the Haversine Formula
double distance(double lat1, double long1,
                     double lat2, double long2)
{
  // Convert the latitudes and longitudes from degree to radians.
  lat1 = toRadians(lat1);
  long1 = toRadians(long1);
  lat2 = toRadians(lat2);
  long2 = toRadians(long2);

  // Haversine Formula
  double dlong = long2 - long1;
  double dlat = lat2 - lat1;
  double ans = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlong / 2), 2);
  ans = 2 * asin(sqrt(ans));

  // Radius of Earth in Kilometers
  double R = 6371;
  ans = ans * R;

  return ans;
}

double bearing(double lat1, double long1, double lat2, double long2)
{
  lat1 = toRadians(lat1);
  long1 = toRadians(long1);
  lat2 = toRadians(lat2);
  long2 = toRadians(long2);

  // Bearing formula
  double del_L = long2 - long1;
  double x = cos(lat2) * sin(del_L);
  double y = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(del_L);
  double bearing = atan2(x, y);
  return toDegrees(bearing);
}

double ascension(double elev_rocket, double elev_gs, double dist)
{
  double y = elev_rocket - elev_gs;
  return toDegrees(atan(y / (dist * 1000)));
}
