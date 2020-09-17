#include "provided.h"
#include <vector>
#include <utility>
#include <list>
#include <iostream>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_sm;
    string getProceedAngle(double dir) const;
};

string DeliveryPlannerImpl::getProceedAngle(double dir) const
{
    string s = "";
    if (dir >= 0 && dir < 22.5)
        s = "east";
    else if (dir < 67.5)
        s = "northeast";
    else if (dir < 112.5)
        s = "north";
    else if (dir < 157.5)
        s = "northwest";
    else if (dir < 202.5)
        s = "west";
    else if (dir < 247.5)
        s = "southwest";
    else if (dir < 292.5)
        s = "south";
    else if (dir < 337.5)
        s = "southeast";
    else if (dir >= 337.5)
        s = "east";
    return s;
}

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
:m_sm(sm)
{
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    double d,dd;
    DeliveryOptimizer optimizer(m_sm);
    vector<DeliveryRequest> orderedDeliveries;
    //create a copy of the deliveries vector to optimize
    for (int i = 0; i < deliveries.size(); i++)
    {
        orderedDeliveries.push_back(deliveries[i]);
    }
    optimizer.optimizeDeliveryOrder(depot, orderedDeliveries, d, dd);
    
    bool justTurnedOrDelivered = true;
    bool returned = false;
    PointToPointRouter router(m_sm);
    list<StreetSegment> temp;
    vector<pair<DeliveryRequest,list<StreetSegment>>> plan;
    GeoCoord g = depot;
    totalDistanceTravelled = 0;
    
    //for every delivery request, generate route
    for (int i = 0; i < orderedDeliveries.size(); i++)
    {
        DeliveryResult result = router.generatePointToPointRoute(g, orderedDeliveries[i].location, temp, d);
        if (result == NO_ROUTE || result == BAD_COORD)
            //if the coord is bad or there is no route
            return result;
        //add distance between points to the total distance travelled
        totalDistanceTravelled += d;
        plan.push_back(make_pair(orderedDeliveries[i],temp));
        //the next "start" coord is the old end
        g = orderedDeliveries[i].location;
    }
    
    //return to depot
    DeliveryRequest back("", depot);
    router.generatePointToPointRoute(g, depot, temp, d);
    totalDistanceTravelled += d;
    plan.push_back(make_pair(back,temp));
    
    //for all of the delivery requests
    for (int i = 0; i < plan.size(); i++)
    {
        string s = "";
        //for all of the street segments in each delivery route
        for (auto it = plan[i].second.begin(); it != plan[i].second.end() ; it++)
        {
            //make proceed command
            DeliveryCommand proceed;
            double dis = distanceEarthMiles(it->start, it->end);
                    
            if (!justTurnedOrDelivered && !commands.empty() && commands.back().streetName() == it->name)
            //if we should just be extending the previous proceed command
            {
                //pop the previous command and readjust distance
                proceed = commands.back();
                commands.pop_back();
                proceed.increaseDistance(dis);
                //push back onto commands
                commands.push_back(proceed);
                justTurnedOrDelivered = false;
            }
                     
            //otherwise if we need to initialize a new proceed command (not just add to previous)
            else
            {
                double angle = angleOfLine(*it);
                proceed.initAsProceedCommand(getProceedAngle(angle), it->name, dis);
                commands.push_back(proceed);
                justTurnedOrDelivered = false;
            }
            //if we have reached the end, break
            if (it->end == depot)
            {
                returned = true;
                break;
            }
        
            //get angle between two segments to determine if next command is turn or proceed
            StreetSegment current = *it;
            it++;
            if (it == plan[i].second.end())
                //if this is the last segment don't execute rest of code for a turn
            {
                it--;
                continue;
            }
            
            StreetSegment next = *it;
               
            if (angleBetween2Lines(current, next) < 1|| angleBetween2Lines(current, next) > 359)
                //a proceed command should be made based off of the angle between the two segments
                //but if this segment is the last before delivery, cannot compare to "next" segment, but will always be proceed right before delivery
            {
                it--; //reset iterator
                continue;
            }
    
            else if (current.name != next.name)
                //otherwise, make turn command
            {
                DeliveryCommand turn;
                string dir;
                if (angleBetween2Lines(current, next) < 180)
                    //left turn
                {
                    dir = "left";
                }
                else
                    //right turn
                {
                    dir = "right";
                }
                turn.initAsTurnCommand(dir, next.name); //add turn command
                commands.push_back(turn);
                justTurnedOrDelivered = true;
            }
            it--; //reset iterator
        }
        if (returned)
            //if we have returned, return success
            return DELIVERY_SUCCESS;
        DeliveryCommand deliver;
        deliver.initAsDeliverCommand(plan[i].first.item);
        commands.push_back(deliver);
        justTurnedOrDelivered = true;
        
    }
    return NO_ROUTE;
}
//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
