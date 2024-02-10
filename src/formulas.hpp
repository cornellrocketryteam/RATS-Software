// Utility function for
// converting degrees to radians
double toRadians(const double degree);

// Utility function for converting radians to degrees
double toDegrees(const double radian);

// calculates the distance between two lat/long points via the Haversine Formula
double distance(double lat1, double long1, double lat2, double long2);

double bearing(double lat1, double long1, double lat2, double long2);

double ascension(double elev_rocket, double elev_gs, double dist);
