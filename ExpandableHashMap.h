// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
#include <list>
#include <vector>
#include <utility>

//for debugging func dump
#include <iostream>
using namespace std;

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    void reset();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);

      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;

      // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }

      // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;
    

private:
    double m_nAssociations;
    double m_maxLoadFactor;
    int getBucket(const KeyType& key) const;
    
    unsigned int hasher(const KeyType& key);
    void expand();
    std::vector<std::list<pair<KeyType,ValueType>>> m_map; //table holds lists of assosiations
    int m_nBuckets;
    
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType,ValueType>::ExpandableHashMap(double maximumLoadFactor)
    :m_maxLoadFactor(maximumLoadFactor), m_nBuckets(8)
//make hash table with 8 empty buckets
{
    m_map.resize(8);
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
//delete dynamically allocated lists
{
    while (!m_map.empty())
    {
        m_map.pop_back();
    }
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::reset()
//delete everything in hash table and reset it to one with 8 buckets
{
    //empty the vector to size 0
    m_map.clear();
    //reset data members
    m_nAssociations = 0;
    m_nBuckets = 8;
    //resize the vector to size 8
    m_map.resize(m_nBuckets);
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType,ValueType>::size() const
//count the number of assosiations
{
    return m_nAssociations;
    //return the sum of the sizes of each list
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::associate(const KeyType& key, const ValueType& value)
{
    ValueType* v = find(key);
    //find returns nullptr if no assosiation for given key is found
    unsigned int bNum = getBucket(key);
    if (v == nullptr)
    {
        // If no association currently exists with that key, create new association
        m_map[bNum].push_back(make_pair(key,value));
        m_nAssociations++;
    }
    else
    {
        for (auto it = m_map[bNum].begin(); it != m_map[bNum].end(); it++)
            //find the key and change its value
        {
            if (it->first == key)
            {
                it->second = value;
            }
        }
    }
    double loadFactor = m_nAssociations/m_nBuckets;
    if (loadFactor > m_maxLoadFactor)
        //make sure that load factor doesn't exceed max
        expand();
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::expand()
{
    std::vector<std::list<pair<KeyType,ValueType>>> temp(m_map);
    int tempBuckets = m_nBuckets;
    
    //create new hash map with 2x buckets
    m_map.clear();
    m_nBuckets *= 2;
    m_map.resize(m_nBuckets);
    
    for (int i = 0; i < tempBuckets; i++)
        //go through old hash map
    {
        for (auto it = temp[i].begin(); it != temp[i].end(); it++)
            //rehash into m_map
        {
            int bNum = getBucket(it->first);
            m_map[bNum].push_back(pair<KeyType,ValueType>(it->first,it->second));
        }
    }
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType,ValueType>::find(const KeyType& key) const
{
    int bNum = getBucket(key);
    //std::list<pair<KeyType,ValueType>> Bucket = m_map[bNum];
    //typename std::list<Association>::const_iterator it;
    
    //if the list at that bucket is not empty, find the key in the list
    for (auto it = m_map[bNum].begin(); it != m_map[bNum].end(); it++)
    {
        if (it->first == key)
        {
            //return ptr to the value
            return &(it->second);
        }
    }
    //if not found, return nullptr
    return nullptr;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType,ValueType>::getBucket(const KeyType &key) const
{
    unsigned int hasher(const KeyType& key);
    unsigned int h = hasher(key);
    return h % m_map.size();
}
