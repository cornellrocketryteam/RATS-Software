/**
 * @file formulas.hpp
 * @author sj728
 *
 * @brief Interface for conversions used for RATS movement
 */

#ifndef FORMULAS_HPP
#define FORMULAS_HPP

/* 
* Utility function for converting degrees to radians 
*/
double toRadians(const double degree);

/*
* Utility function for converting radians to degrees
*/
double toDegrees(const double radian);

/*
*  Calculates the distance between two lat/long points
*/
double distance(double lat1, double long1, double lat2, double long2);


#endif // FORMULAS_HPP