
// Author: Sean Davis
#ifndef evacH
#define evacH

#include "EvacRunner.h"

class myRoad
{
public:
  int destinationCityID;
  int destinationCityScore;
  int destinationCityLayer;
  int peoplePerHour;
  int ID;
}; // class Road

class myCity
{
public:
  int ID;
  int population;
  int evacuees;
  myRoad *roads;
  int roadCount;
  int roadIndex;
  //int lastRoadCapacityUsed;
  //int lastRoadIndexAtEvacRoutes;
  int score;
  int layer;

  myCity() : evacuees(0), roads(NULL), roadCount(0){}
  //~myCity(){delete [] roads;}
}; // class City

class Evac
{
public:
    EvacRoute* evacRoutes;
    myCity* cities;
    myCity** evacCities;

    int numCities;
    int numEvacs;
    int* evacIDs;
    int* routeCount;
    int evacRouteIndex;
    bool* visited;
    bool* inPath;

    int evacuatedCount;
    int t;

    void addRoute(int theRoadID, int evacs);
    Evac(City *cities, int numCitie, int numRoads);
    void evacuate(int *_evacIDs, int _numEvacs, EvacRoute *_evacRoutes,
        int &_routeCount); // student sets evacRoutes and routeCount
    void setLayers();
    void setScores();
    int runHour(int t);
    int depthFS(int cityID, int requested);
    // int depthFS(myCity city, int requested);
    // static int worseCity(const void * a, const void * b);


}; // class Evac



#endif
