#include "provided.h"
#include <algorithm>
#include <list>
#include <set>
#include <utility>
#include <vector>
#include <queue>
#include "ExpandableHashMap.h"
using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_sm;
  
};


PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
    :m_sm(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

unsigned int hasher(const pair<GeoCoord,GeoCoord>& g)
{
    return std::hash<string>()(g.first.latitudeText + g.first.longitudeText);
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    typedef pair<double,GeoCoord> node;
    vector<StreetSegment> temp, temp2;
    priority_queue<node,vector<node>,greater<node>> nodeQueue; //will be sorted by the double
    ExpandableHashMap<GeoCoord, GeoCoord> m_path;
    ExpandableHashMap<pair<GeoCoord,GeoCoord>, string> m_pathNames;
    set<GeoCoord> closedSet;
    
    if (!route.empty())
        route.clear();
    
    //make sure that the start and end coordinates exist in the map data
    m_sm->getSegmentsThatStartWith(start, temp);
    m_sm->getSegmentsThatStartWith(end, temp2);
    if (temp.empty() || temp2.empty())
        return BAD_COORD;
    
    
    nodeQueue.push(make_pair(0,start)); //start has initial "fval" of 0
    
    while (!nodeQueue.empty())
    {
        //take coord from top of queue
        GeoCoord current = nodeQueue.top().second;
        nodeQueue.pop();
        
        if (current == end) //if we have reached the end
        {
            //retrace steps
            GeoCoord retracer = end;
            totalDistanceTravelled = 0;
            while (retracer != start)
                //loop through until we reach the start
            {
                GeoCoord* s = m_path.find(retracer);
                string* streetName;
                streetName = m_pathNames.find(make_pair(*s, retracer));
                StreetSegment seg(*s,retracer,*streetName);
                route.push_front(seg);
                totalDistanceTravelled += distanceEarthMiles(seg.start, seg.end);
                retracer = *s;
            }
            return DELIVERY_SUCCESS;
        }
        //if we haven't reached the end, get neighbors
        vector<StreetSegment> neighbors;
        m_sm->getSegmentsThatStartWith(current, neighbors);
        
        //for all adjacent segments
        for (int i = 0; i < neighbors.size(); i++)
        {
            if (closedSet.find(neighbors[i].end) != closedSet.end())
                //if the neighbor exists in closed set
                //set::find returns iterator == set::end() if it is not found
                continue;
            m_path.associate(neighbors[i].end, neighbors[i].start);
            m_pathNames.associate(make_pair(neighbors[i].start, neighbors[i].end), neighbors[i].name);
            closedSet.insert(neighbors[i].end); //add to set of coords already visited
            
            //set the second value in pair, which is the distance to the end
            //priority queue will be sorted by this (smallest first)
            double d = distanceEarthMiles(neighbors[i].end, end);
            nodeQueue.push(make_pair(d, neighbors[i].end));
        }
    }
    
    return NO_ROUTE;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
