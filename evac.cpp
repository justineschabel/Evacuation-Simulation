
#include <cstdlib>
#include "evac.h"
#include "EvacRunner.h"
#include "theQueue.h"
#include "string.h" 

using namespace std;

Evac::Evac(City *citie, int numCitie, int numRoads) : numCities(numCitie)
{    
    myCity* theCity;
    visited = new bool[numCities];
    inPath = new bool[numCities];
    cities = new myCity[numCities];   
    for(int i=0; i<numCities; i++){
        theCity = &cities[i];
        theCity->layer = -1;
        theCity->roads = new myRoad[citie[i].roadCount];
        theCity->ID = citie[i].ID;
        theCity->population = citie[i].population;
        theCity->evacuees = citie[i].evacuees;
        theCity->roadCount = citie[i].roadCount;
        // Copy roads too
        for(int j=0; j<theCity->roadCount; j++){
            theCity->roads[j].destinationCityID = citie[i].roads[j].destinationCityID;
            theCity->roads[j].peoplePerHour = citie[i].roads[j].peoplePerHour;
            theCity->roads[j].ID = citie[i].roads[j].ID;
        }
    }
} // Evac()

int bestRoad(const void *a, const void * b){
    myRoad* A = ((myRoad*)a);
    myRoad* B = ((myRoad*)b);
    if (A->destinationCityLayer == B->destinationCityLayer){
        return B->destinationCityScore - A->destinationCityScore;
    }
    return B->destinationCityLayer - A->destinationCityLayer;
}

void Evac::evacuate(int *_evacIDs, int _numEvacs, EvacRoute *_evacRoutes,
    int &_routeCount)
{   
    evacIDs = _evacIDs;
    evacRoutes = _evacRoutes;
    numEvacs = _numEvacs;
    routeCount = &_routeCount;
    evacCities = new myCity*[numEvacs];
    evacRouteIndex = 0;
    
    setLayers();
    setScores();
  
    t = 1;
    while(t >= 0){
    // while(t < 100){
        t = runHour(t);  // if finish return -1, else return t+1;
    }
    
    *routeCount = evacRouteIndex;
    
} // evacuate

void Evac::setLayers(){
    short theCityID;
    short destCityID; 
    myCity* theCity;
    memset(visited, false, numCities);
    theQueue queue(numCities);
    // set evac Cities as layer 0
    for(int i=0; i<numEvacs; i++){
        evacCities[i] = &cities[evacIDs[i]];
        evacCities[i]->layer = 0;
        visited[evacIDs[i]] = true;
        evacCities[i]->evacuees = cities[evacIDs[i]].population;
        queue.enQueue(evacIDs[i]);
    } 

    // bsf and set everyones layer
    for(int i=0; i<numCities; i++){
        theCityID = queue.deQueue();
        theCity = &cities[theCityID];
        for(int j=0; j<theCity->roadCount; j++){
            destCityID = theCity->roads[j].destinationCityID;
            if (!visited[destCityID]){
                cities[destCityID].layer = theCity->layer + 1;
                queue.enQueue(destCityID);
                visited[destCityID] = true;
            }
        }
    }
    bool higher;
    for(int i=0; i<numEvacs; i++){
        higher = false;
        theCity = &cities[evacIDs[i]];
        for(int j=0; !higher && j<theCity->roadCount; j++){
            destCityID = theCity->roads[j].destinationCityID;
            if(cities[destCityID].layer > theCity->layer){
                higher = true;
            }
        }
        if (!higher)
            theCity->layer--;
    } 
}   

void Evac::setScores(){
    short theCityID;
    short destCityID; 
    myCity* theCity;
    myCity* destCity; 
    // assign score based on layer and in/out flows.
    for(theCityID=0; theCityID<numCities; theCityID++){
        theCity = &cities[theCityID];
        theCity->score = 0;
        for (int j=0; j<theCity->roadCount; j++){
            destCityID = theCity->roads[j].destinationCityID;
            destCity = &cities[destCityID];
            if (destCity->layer > theCity->layer){
                theCity->score += theCity->roads[j].peoplePerHour;
            }else if(destCity->layer > theCity->layer){
                theCity->score -= theCity->roads[j].peoplePerHour;
            }
        }
    }
    
    for(int i=0; i<numCities;i++){
        theCity = &cities[i];
        for(int j=0; j<theCity->roadCount;j++){
            theCity->roads[j].destinationCityScore = cities[theCity->roads[j].destinationCityID].score;
            theCity->roads[j].destinationCityLayer = cities[theCity->roads[j].destinationCityID].layer;
        }
        qsort(theCity->roads, theCity->roadCount, sizeof(myRoad), bestRoad);
    }
}

void Evac::addRoute(int theRoadID, int evacs){
    evacRoutes[evacRouteIndex].numPeople = evacs;
    evacRoutes[evacRouteIndex].roadID = theRoadID;
    evacRoutes[evacRouteIndex].time = t;
    evacRouteIndex++;
}

int Evac::depthFS(int cityID, int requested){
    short destCityID; 
    myCity* city;
    myCity* destCity;
     // = new myCity;
    // myCity *destCity; // = new myCity;
    myRoad *road; // = new myRoad;
    int evacsNotSent;
    int evacsToSend;
    city = &cities[cityID];
    inPath[city->ID] = true;

    city->evacuees += requested;
    if (!visited[city->ID]){
        city->roadIndex = 0;
        visited[city->ID] = true;
    }

    // if city is not bottom, we will ask it to be at least half or some % full to start evacuating
    if ((city->layer > 0 && 1*city->evacuees > city->population) || (city->layer <= 0)){
        
        for(; city->evacuees > 0 && city->roadIndex < city->roadCount;){
            road = &city->roads[city->roadIndex];
            destCityID = road->destinationCityID;
            destCity = &cities[destCityID];
            if (!inPath[destCityID]){
                if (destCity->layer >= city->layer){// || (destCity->layer == city->layer && destCity->score > destCity->score)){
                    evacsToSend = min(road->peoplePerHour, city->evacuees);
                    evacsNotSent = depthFS(destCityID, evacsToSend);
                    // cout << evacsNotSent << endl;
                    if(evacsToSend != evacsNotSent){
                        addRoute(road->ID, evacsToSend-evacsNotSent);
                    }
                    city->evacuees -= evacsToSend;
                    city->evacuees += evacsNotSent;
                    city->roadIndex++;
                }else{
                    city->roadIndex = city->roadCount;
                }
            }else{
                city->roadIndex++;
            }
        }
    }
        
    inPath[city->ID] = false;

    if (city->layer > 0){
        evacsNotSent = max(city->evacuees - city->population, 0);
        city->evacuees -= evacsNotSent;
        return evacsNotSent;
    }else{
        evacsNotSent = max(city->evacuees - 2 * city->population, 0);
        city->evacuees -= evacsNotSent;
        return evacsNotSent;
    }
    return 0;

}
 
int worseCity(const void *a, const void *b){
    myCity* A = (*(myCity**)a);
    myCity* B = (*(myCity**)b);
    if (A->layer == B->layer){
        return B->evacuees - B->score - A->evacuees + A->score;
    }
    return A->layer - B->layer; 
}

int Evac::runHour(int t){
    memset(visited, false, numCities);
    memset(inPath, false, numCities);
    
    // first sort evacCities based on score.
    qsort(evacCities, numEvacs, sizeof(myCity*), worseCity);
    evacuatedCount = 0;
    for (int i = 0; i < numEvacs; i++){
        depthFS(evacCities[i]->ID, 0);
        if(evacCities[i]->evacuees == 0){
            evacuatedCount++;
        }
    }
    if (evacuatedCount == numEvacs){
        return -1;
    }
    return t + 1;
    // depth first search prioritizing better score, never go to lower score        
}

 

    
