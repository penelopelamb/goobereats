#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include "ExpandableHashMap.h"
using namespace std;


unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_streetMap;
    void addToMap(GeoCoord& g, StreetSegment& seg);
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

void StreetMapImpl::addToMap(GeoCoord &g, StreetSegment &seg)
{
    vector<StreetSegment> temp;
    
    if (m_streetMap.find(g) == nullptr) //if the segment is not found, make new association
    {
        temp.push_back(seg);
        m_streetMap.associate(g, temp);
    }
    else
        //if there already exists another mapping with same key, we must add the value without deleting the pre existing mapping
    {
        //add the segment to the vector
        temp = *(m_streetMap.find(g));
        temp.push_back(seg);
        //re associate the new vector containing all previous segments plus our new segment
        m_streetMap.associate(g, temp);
    }
    
   
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream mapdata(mapFile);
    if (!mapdata)
        //if data fails to load, return false
        return false;
    string name,temp;
    while (getline(mapdata,name))
        //getline increments file line with every call
    {
        int C;
        mapdata >> C;
        string s_lat, s_long, e_lat, e_long;
        
        for (int i = 0; i < C; i++)
        {
            mapdata >> s_lat >> s_long >> e_lat >> e_long;
            //create segment
            GeoCoord start(s_lat,s_long);
            GeoCoord end(e_lat,e_long);
            StreetSegment forward(start,end,name);
            StreetSegment backward(end,start,name);
            
            //add to the map a segment mapping its starting geocoord to the forward segment
            addToMap(start, forward);
            //add to the map a segment mapping its ending geocoord to its reverse segment
            addToMap(end, backward);
        }
        getline(mapdata,temp); //skip a line
    }
    return true;
   }

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    if (m_streetMap.find(gc) == nullptr)
    {
        //if they key was not found
        return false;
    }
    
    if (!segs.empty()) //incase the segs vector passed in already has streetsegments, clear it
    {
        segs.clear();
    }

    segs = *m_streetMap.find(gc);
    return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
