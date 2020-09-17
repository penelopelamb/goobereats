#include "provided.h"
#include <vector>
#include <ctime>
#include <cstdlib>

using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    double calculateProbability(double currentDis, double copyDis, double temperature) const;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

double DeliveryOptimizerImpl::calculateProbability(double currentDis, double copyDis, double temperature) const
{
    if (copyDis < currentDis)
        //if the new distance is better, accept it
        return 1.0;
    else
        return exp((currentDis-copyDis)/temperature);
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    unsigned seed = time(NULL);
    srand(seed);
   
    double temp;
    double temperature = 10000;
    //vector<DeliveryRequest> potential(deliveries);
    vector<DeliveryRequest> current(deliveries);
    double potentialDis;
    double deliveriesDis, currentDis;
    GeoCoord g(depot);
    //calculate distance
    for (int i = 0; i < deliveries.size(); i++)
    {
        oldCrowDistance += distanceEarthMiles(g, deliveries[i].location);
        g = deliveries[i].location;
    }
    //dont forget back to depot
    oldCrowDistance += distanceEarthMiles(g, depot);
    
    deliveriesDis = currentDis = oldCrowDistance;
    int threshhold = pow(deliveries.size(),3);
    
    
    for (int i = 0; i < threshhold; i++) 
    {
        vector<DeliveryRequest>potential(current);
        
        //get random positions to be swapped
        int pos1 = rand() % potential.size();
        int pos2 = rand() % potential.size();

        //shuffle
        iter_swap(potential.begin()+pos1, potential.begin()+pos2);
        
        //get the new distance
        g = depot;
        potentialDis = 0;
        for (int i = 0; i < potential.size(); i++)
        {
            potentialDis += distanceEarthMiles(g, potential[i].location);
            g = potential[i].location;
        }
        //dont forget back to depot
        potentialDis += distanceEarthMiles(g, depot);
        
        
        //decide if we should accept swap
        temp = calculateProbability(currentDis,potentialDis,temperature);
        
        if (temp > rand()%2)
        {
            current = potential;
            currentDis = potentialDis;
        }
        
        //keep track of best, place in deliveries vector
        if (currentDis < deliveriesDis)
        {
            deliveries = current;
            deliveriesDis = currentDis;
        }

        //cool
        temperature *= 0.8;
    }

    newCrowDistance = deliveriesDis;
    
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
