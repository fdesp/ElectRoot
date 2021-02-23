//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#if !defined(ELECTROOT_H)
#define ELECTROOT_H

#include "BaseNode.h"
#include "ElectRoot_m.h"
#include <list>

class ElectRoot : public BaseNode{


protected: 
    /** @brief Unique identifier of the node **/
    int id;

    /** @brief The neighbors of the node  **/
    std::list<int> waitingList;

    /** @brief The node parent port  **/
    int parent;

    /** @brief The neighborhood size */
    size_t neighborhoodSize;

protected:
   
    /** @brief  **/
    virtual void becomeProcessing();

    /** @brief Prints the node starts in the simulation canvas */
    virtual void refreshDisplay() const override;

public:
    ElectRoot();

    /** @brief Assings the initial node status: idle or initiator **/
    virtual void initialize() override;

    /** @brief Assings the initial node status: idle or initiator **/
    virtual void handleMessage(omnetpp::cMessage*);
};

#endif