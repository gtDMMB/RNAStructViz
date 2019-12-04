/* BranchTypeIdentification.cpp 
   Author:  Maxie D. Schmidt (maxieds@gmail.com)
   Created: 2018.06.16
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <algorithm>
using std::vector;
using std::sort;

#include "BranchTypeIdentification.h"

#if PERFORM_BRANCH_TYPE_ID
#pragma message "Compiling in support for branch-type ID in " __FILE__ "..."

vector<RNAStructure::BaseData *> RNABranchType_t::getEnclosingArcs( 
       RNAStructure * &rnaStructBase, bool removeTopFour = false) { 
     vector<RNAStructure::BaseData *> rBaseDataVec;
     for(int bd = 0; bd < rnaStructBase->GetLength(); bd++) {
          bool isAlreadyEnclosed = false;
          RNAStructure::BaseData *curBaseData = rnaStructBase->GetBaseAt(bd);
          for(int v = 0; v < rBaseDataVec.size(); v++) {
               if(curBaseData->m_pair == RNAStructure::UNPAIRED) { 
                    isAlreadyEnclosed = true;
                    break;
               }
               else if(curBaseData->isContainedIn(*(rBaseDataVec[v]))) {
                    isAlreadyEnclosed = true;
                    break;
               }
               else if(rBaseDataVec[v]->isContainedIn(*curBaseData)) {
                    rBaseDataVec[v] = curBaseData;
                    isAlreadyEnclosed = true;
                    break;
               }
          }
          if(!isAlreadyEnclosed && (rBaseDataVec.size() > 0 || curBaseData->m_pair != RNAStructure::UNPAIRED)) {
               rBaseDataVec.push_back(curBaseData);
          }
     }
     sort(rBaseDataVec.begin(), rBaseDataVec.end(), RNAStructureBaseDataArcLengthSort());
     if(removeTopFour) {
          rBaseDataVec.erase(rBaseDataVec.begin(), rBaseDataVec.begin() + 4);
     }
     return rBaseDataVec;
}

RNABranchType_t::RNABranchType_t() {
     branchID = BRANCH_UNDEFINED;
     branchParent = NULL;
}

RNABranchType_t::RNABranchType_t(BranchID_t bid, class RNAStructure::BaseData *bparent = NULL) {
     branchID = bid;
     branchParent = bparent;
} 
 
RNABranchType_t & RNABranchType_t::operator=(const BranchID_t &rhs) {
     setBranchID(rhs);
     branchParent = NULL;
     return *this;
} 
         
bool RNABranchType_t::operator==(const RNABranchType_t &rhs) const {
     return branchID == rhs.branchID && branchParent == rhs.branchParent;
}
          
bool RNABranchType_t::operator==(const BranchID_t &rhs) const {
     return branchID == rhs;
}

const BranchID_t & RNABranchType_t::getBranchID() const {
     return branchID;
} 
          
void RNABranchType_t::setBranchID(BranchID_t bid) {
     branchID = bid;
}
          
const RNAStructure::BaseData* RNABranchType_t::getBranchParent() const {
     return branchParent;
}
          
void RNABranchType_t::setBranchParent(class RNAStructure::BaseData* bparent) {
     branchParent = bparent;
}

void RNABranchType_t::SetBranchColor(cairo_t * &cr, BranchID_t bt) {
     switch(bt) {
          case BRANCH_UNDEFINED:
               cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); 
               break;
          case BRANCH1:
               cairo_set_source_rgb(cr, 92.0 / 255, 160.0 / 255, 215.0 / 255);
               break;
          case BRANCH2:
               cairo_set_source_rgb(cr, 183.0 / 255, 127.0 / 255, 77.0 / 255);
               break;
          case BRANCH3:
               cairo_set_source_rgb(cr, 243.0 / 255, 153.0 / 255, 193.0 / 255);
               break;
          case BRANCH4:
               cairo_set_source_rgb(cr, 123.0 / 255, 204.0 / 255, 153.0 / 255);
               break;
          default:
               break;
     }
}

bool RNABranchType_t::PerformBranchClassification(class RNAStructure * &rnaStructBase, unsigned int alength) {

     if(alength < 4)
          return false;     

     // first we determine the four most enclosing arcs on the circle: 
     RNAStructure::BaseData* mostEnclosingArcs[4] = {NULL, NULL, NULL, NULL};
     unsigned int mostEnclosingArcsSize = 0;
     for(int rs = 0; rs < alength; rs++) {
          RNAStructure::BaseData* rnaStruct = rnaStructBase->GetBaseAt(rs);
          if(rnaStruct->m_pair == RNAStructure::UNPAIRED) 
               continue;
          else if(mostEnclosingArcsSize == 0) {
               mostEnclosingArcs[0] = rnaStructBase->GetBaseAt(rs);
               mostEnclosingArcsSize++;
               continue;
          }
          bool isEnclosedInLargerBranch = false;
          for(int mea = 0; mea < mostEnclosingArcsSize; mea++) {
               if(rnaStruct->isContainedIn(*(mostEnclosingArcs[mea]))) {
                    isEnclosedInLargerBranch = true;
                    break;
               }
          }
          if(isEnclosedInLargerBranch) 
               continue; // this cannot be an outer containing branch
          RNAStructure::BaseData *currentBaseData = rnaStructBase->GetBaseAt(rs);;
          unsigned int pairDistance = ABS(MAX(currentBaseData->m_pair, currentBaseData->m_index) - 
                                          MIN(currentBaseData->m_pair, currentBaseData->m_index));
          for(int mea = 0; mea < mostEnclosingArcsSize; mea++) { 
               RNAStructure::BaseData *meaBaseData = mostEnclosingArcs[mea];
               unsigned int meaPairDistance = ABS(MAX(meaBaseData->m_pair, meaBaseData->m_index) - 
                                                  MIN(meaBaseData->m_pair, meaBaseData->m_index)); 
               bool needToResort = false;
               if(meaPairDistance < pairDistance && meaBaseData->m_pair > meaBaseData->m_index) {
                    if(mostEnclosingArcsSize < 4) {
                         mostEnclosingArcs[mostEnclosingArcsSize] = mostEnclosingArcs[mea];
                         mostEnclosingArcsSize++;
                         needToResort = true;
                    }
                    mostEnclosingArcs[mea] = rnaStructBase->GetBaseAt(rs);
                    if(needToResort) {
                         qsort(&mostEnclosingArcs[0], mostEnclosingArcsSize, 
                               sizeof(RNAStructure::BaseData*), compareMostEnclosingArcs);
                    }
                    break;
               }
          }
     }
     // sort the identified branches according to their natural order on the circle:
     IntIndexPair_t branchOrderMappings[mostEnclosingArcsSize];
     for(int i = 0; i < mostEnclosingArcsSize; i++) { 
          branchOrderMappings[i].intValue = MIN(mostEnclosingArcs[i]->m_index, mostEnclosingArcs[i]->m_pair); 
          branchOrderMappings[i].index = i;
     }
     qsort(&branchOrderMappings[0], mostEnclosingArcsSize, sizeof(IntIndexPair_t), compareIntegerIndexPair); 
     RNAStructure::BaseData* mostEnclosingArcsTemp[4];
     for(int j = 0; j < mostEnclosingArcsSize; j++) { 
          mostEnclosingArcsTemp[j] = mostEnclosingArcs[branchOrderMappings[j].index];
     }
     for(int k = 0; k < mostEnclosingArcsSize; k++) { 
          mostEnclosingArcs[k] = mostEnclosingArcsTemp[k];
     }
     // now that we've identified most of the the enclosing branches, 
     // we reset the branch types by number on all (except for the nubbins, 
     // see below) entries in the array: 
     vector<RNAStructure::BaseData *> enclosingArcs = getEnclosingArcs(rnaStructBase, false);
     if(enclosingArcs.size() < 7 || mostEnclosingArcsSize < 4) {
          return false;
     }
     sort(enclosingArcs.begin(), enclosingArcs.begin() + 4, RNAStructureBaseDataIndexSort());
     // handle labeling of the arc nubbins (and contained pairs) to the sides of the big arcs: 
     vector<RNAStructure::BaseData *> enclosingArcsNubbins(enclosingArcs.begin() + 4, enclosingArcs.begin() + 7);
     sort(enclosingArcsNubbins.begin(), enclosingArcsNubbins.end(), RNAStructureBaseDataIndexSort());
     BranchID_t nubbinBranchIDs[3] = {BRANCH1, BRANCH2, BRANCH4};
     for(int n = 0; n < 3; n++) {
          rnaStructBase->GetBranchTypeAt(enclosingArcsNubbins[n]->m_index)->setBranchID(nubbinBranchIDs[n]);
          rnaStructBase->GetBranchTypeAt(enclosingArcsNubbins[n]->m_index)->setBranchParent(enclosingArcs[(int) (nubbinBranchIDs[n] - 1)]);
          rnaStructBase->GetBranchTypeAt(enclosingArcsNubbins[n]->m_pair)->setBranchID(nubbinBranchIDs[n]);
          rnaStructBase->GetBranchTypeAt(enclosingArcsNubbins[n]->m_pair)->setBranchParent(enclosingArcs[(int) (nubbinBranchIDs[n] - 1)]);
          for(int b = 0; b < alength; b++) {
               RNAStructure::BaseData *curBase = rnaStructBase->GetBaseAt(b);
               if(curBase->isContainedIn(*(enclosingArcsNubbins[n])) || 
                  curBase->m_pair == RNAStructure::UNPAIRED && 
                  curBase->m_index < MAX(enclosingArcsNubbins[n]->m_index, enclosingArcsNubbins[n]->m_pair) && 
                  curBase->m_index > MIN(enclosingArcsNubbins[n]->m_index, enclosingArcsNubbins[n]->m_pair)) {
                    rnaStructBase->GetBranchTypeAt(b)->setBranchID(nubbinBranchIDs[n]);
                    rnaStructBase->GetBranchTypeAt(b)->setBranchParent(enclosingArcs[(int) (nubbinBranchIDs[n] - 1)]);
               }
          }
     }
     // handle labeling of the big arcs (and contained pairs and unpaired components contained within):
     for(int k = 0; k < 4; k++) { 
          rnaStructBase->GetBranchTypeAt(enclosingArcs[k]->m_index)->setBranchID((BranchID_t) (k + 1));
          rnaStructBase->GetBranchTypeAt(enclosingArcs[k]->m_index)->setBranchParent(enclosingArcs[k]);
          rnaStructBase->GetBranchTypeAt(enclosingArcs[k]->m_pair)->setBranchID((BranchID_t) (k + 1));
          rnaStructBase->GetBranchTypeAt(enclosingArcs[k]->m_pair)->setBranchParent(enclosingArcs[k]);
          for(int b = 0; b < alength; b++) {
               RNAStructure::BaseData *curBase = rnaStructBase->GetBaseAt(b);
               if(curBase->isContainedIn(*(enclosingArcs[k])) || 
                  (curBase->m_pair == RNAStructure::UNPAIRED && curBase->m_index < MAX(enclosingArcs[k]->m_index, enclosingArcs[k]->m_pair) && 
                  curBase->m_index > MIN(enclosingArcs[k]->m_index, enclosingArcs[k]->m_pair))) {
                    rnaStructBase->GetBranchTypeAt(b)->setBranchID((BranchID_t) (k + 1));
                    rnaStructBase->GetBranchTypeAt(b)->setBranchParent(enclosingArcs[k]);
               }
          }
     }
     // handle labeling of the unpaired components in between the big arcs:
     for(int b = 0; b < 4; b++) { 
          int unpairedBetweenBranchCount = 0;
          vector<RNAStructure::BaseData *> unpairedBetweenVec;
          for(int v = 0; v < alength; v++) { 
               RNAStructure::BaseData *curUnpaired = rnaStructBase->GetBaseAt(v);
               if(curUnpaired->isContainedIn(*(enclosingArcs[b])) || 
                  curUnpaired->m_index == enclosingArcs[b]->m_pair && curUnpaired->m_pair == enclosingArcs[b]->m_index) { 
                    rnaStructBase->GetBranchTypeAt(v)->setBranchID((BranchID_t) (b + 1));
                    rnaStructBase->GetBranchTypeAt(v)->setBranchParent(enclosingArcs[b]);
               } 
               else if(curUnpaired->m_pair == RNAStructure::UNPAIRED &&
                  curUnpaired->m_index > MAX(enclosingArcs[b]->m_pair, enclosingArcs[b]->m_index) && (b == 3 ||  
                  curUnpaired->m_index < MIN(enclosingArcs[b + 1]->m_pair, enclosingArcs[b + 1]->m_index))) { // we want the unpaired bases between branches <- : 
                    unpairedBetweenBranchCount++;
                    unpairedBetweenVec.push_back(curUnpaired);
               }
          }
          for(int up = 0; up < unpairedBetweenVec.size(); up++) { 
               if(b < 3 && up <= unpairedBetweenVec.size() / 2) { 
                    rnaStructBase->GetBranchTypeAt(unpairedBetweenVec[up]->m_index)->setBranchID((BranchID_t) (b + 1));
                    rnaStructBase->GetBranchTypeAt(unpairedBetweenVec[up]->m_index)->setBranchParent(enclosingArcs[b]);
               }
               else if(b < 3) {
                    rnaStructBase->GetBranchTypeAt(unpairedBetweenVec[up]->m_index)->setBranchID((BranchID_t) (b + 2));
                    rnaStructBase->GetBranchTypeAt(unpairedBetweenVec[up]->m_index)->setBranchParent(enclosingArcs[b + 1]);
               }
               else {
                    rnaStructBase->GetBranchTypeAt(unpairedBetweenVec[up]->m_index)->setBranchID(BRANCH4);
                    rnaStructBase->GetBranchTypeAt(unpairedBetweenVec[up]->m_index)->setBranchParent(enclosingArcs[3]);
               }
          }
     }
     // handle labeling of the unpaired components before the first big arc:
     for(int up = 0; up < alength; up++) { 
          RNAStructure::BaseData *curUnpaired = rnaStructBase->GetBaseAt(up);
          if(curUnpaired->m_index < MIN(enclosingArcs[0]->m_index, enclosingArcs[0]->m_pair) && curUnpaired->m_pair == RNAStructure::UNPAIRED) { 
               rnaStructBase->GetBranchTypeAt(up)->setBranchID(BRANCH1);
               rnaStructBase->GetBranchTypeAt(up)->setBranchParent(enclosingArcs[0]);
          }
     }
     return true;
}

#endif
