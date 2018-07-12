/* BranchTypeIdentification.h : 
   A helper class to identify which of the four branch types 
   to which the RNAStructure and/or pairing belongs.
   Author:  Maxie D. Schmidt (maxieds@gmail.com)
   Created: 2018.06.16
*/

#ifndef __RNABRANCHTYPEIDENT_H__
#define __RNABRANCHTYPEIDENT_H__

#include <cairo.h>
#include <vector>
#include <algorithm>
using std::vector;
using std::sort;

#include "RNAStructure.h" 

#define NUM_BRANCHES                  (4)

typedef enum {
     BRANCH1 = 1, 
     BRANCH2 = 2, 
     BRANCH3 = 3, 
     BRANCH4 = 4, 
     BRANCH_UNDEFINED = 0
} BranchID_t;

typedef struct {
     int index;
     int intValue;
} IntIndexPair_t;

inline int compareIntegerIndexPair(const void *ip1, const void *ip2) {
     int i1 = ((IntIndexPair_t*) ip1)->intValue, i2 = ((IntIndexPair_t*) ip2)->intValue;
     if(i1 < i2) 
          return -1;
     else if(i1 == i2)
          return 0;
     else
          return 1;
}

inline int compareMostEnclosingArcs(const void *arc1, const void *arc2) {
     RNAStructure::BaseData *rnaStruct1 = (RNAStructure::BaseData *) arc1;
     RNAStructure::BaseData *rnaStruct2 = (RNAStructure::BaseData *) arc2;
     unsigned int arc1PairDist = rnaStruct1->getPairDistance();
     unsigned int arc2PairDist = rnaStruct2->getPairDistance();
     if(arc1PairDist < arc2PairDist)
          return -1;
     else if(arc1PairDist == arc2PairDist)
          return 0;
     else
          return 1;
}

class RNABranchType_t {

     protected:
          static vector<RNAStructure::BaseData *> getEnclosingArcs( 
                 RNAStructure * &rnaStructBase, bool removeTopFour);

     public: 
          RNABranchType_t(BranchID_t bid, class RNAStructure::BaseData *bparent); 
 
          RNABranchType_t & operator=(const BranchID_t &rhs); 
         
          bool operator==(const RNABranchType_t &rhs) const;
          bool operator==(const BranchID_t &rhs) const;

          const BranchID_t & getBranchID() const;
          void setBranchID(BranchID_t bid); 
          
          const RNAStructure::BaseData* getBranchParent() const;
          void setBranchParent(class RNAStructure::BaseData* bparent); 

          static void SetBranchColor(cairo_t * &cr, BranchID_t bt);

          static bool PerformBranchClassification(class RNAStructure * &rnaStructArray, unsigned int alength);

     protected:
          BranchID_t branchID;
          RNAStructure::BaseData *branchParent;

          
     public:
          typedef struct {
               inline bool operator()(RNAStructure::BaseData *bd1, RNAStructure::BaseData *bd2) { // sorts in decreasing order (i.e., largest arcs first): 
                    int bd1ArcDist = (bd1->m_pair == RNAStructure::UNPAIRED) ? 0 : MAX(bd1->m_index, bd1->m_pair) - MIN(bd1->m_index, bd1->m_pair);
                    int bd2ArcDist = (bd2->m_pair == RNAStructure::UNPAIRED) ? 0 : MAX(bd2->m_index, bd2->m_pair) - MIN(bd2->m_index, bd2->m_pair);
                    return bd1ArcDist > bd2ArcDist;
               }
          } RNAStructureBaseDataArcLengthSort;

          typedef struct {
               inline bool operator()(RNAStructure::BaseData *bd1, RNAStructure::BaseData *bd2) { 
                    return bd1->m_index < bd2->m_index;
               }
          } RNAStructureBaseDataIndexSort;

};

#endif
