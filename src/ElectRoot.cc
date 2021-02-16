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

#include "ElectRoot.h"

Define_Module(ElectRoot);

ElectRoot::ElectRoot()    
    : id(-1)
    , parent(-1)
    , neighborhoodSize(0)
{ }

void ElectRoot::initialize(){
    if(par("initiator").boolValue()){
        scheduleAt(par("startTime"), timer);
        status = Status::INITIATOR;
    }
    else
        status = Status::IDLE;
    neighborhoodSize = gateSize("port$o");
}

void ElectRoot::handleMessage(omnetpp::cMessage* recvMsg){
    if (status == Status::INITIATOR){
        if (recvMsg->getKind() == MsgKind::TIMER){ // A1
            //auto electRootMsg = dynamic_cast<ElectRootMsg*>(recvMsg);
            id = getIndex();
            neighborhoodSize = gateSize("port$o");
            auto wakeupMsg = new ElectRootMsg("activate", MsgKind::ACTIVATION);
            wakeupMsg->setId(id);
            for (int i = 0; i < neighborhoodSize - 1; i++){
                send(wakeupMsg->dup(), "port$o", i);
                waitingList.push_front(i);
            }
            send(wakeupMsg, "port$o", neighborhoodSize - 1);
            waitingList.push_front(neighborhoodSize - 1);

            if(waitingList.size() == 1){
                parent = waitingList.front();
                waitingList.pop_front();
                auto saturationMsg = new ElectRootMsg("saturation", MsgKind::SATURATION);
                saturationMsg->setId(id);
                send(saturationMsg, "port$o", parent);
                status = Status::PROCESSING;
            }
            else
                status = Status::ACTIVE;
        }
        else
            omnetpp::cRuntimeError("Invalid event for status initiator\n");
    }
    else if (status == Status::IDLE){
        if (recvMsg->getKind() == MsgKind::ACTIVATION){ // A2
           // auto electRootMsg = dynamic_cast<ElectRootMsg*>(recvMsg);
            int sender = recvMsg->getArrivalGate()->getIndex();
            int lastNeighbor = neighborhoodSize - 1;
            for (int i = 0; i < lastNeighbor; i++)
            {
                if(i != sender){
                    send(recvMsg->dup(), "port$o", i);
                    waitingList.push_front(i);
                }	
            }
            if(lastNeighbor != sender){
                send(recvMsg, "port$o", lastNeighbor);
                waitingList.push_front(lastNeighbor);
            }
            if(waitingList.size() == 1){
                parent = waitingList.front();
                waitingList.pop_front();
                auto saturationMsg = new ElectRootMsg("saturation", MsgKind::SATURATION);
                saturationMsg->setId(id);
                send(saturationMsg, "port$o", parent);
                status = Status::PROCESSING;
            }
            else
                status = Status::ACTIVE;        }
        else
            omnetpp::cRuntimeError("Invalid event for status idle\n");
    }
    else if (status == Status::ACTIVE){
        if (recvMsg->getKind() == MsgKind::SATURATION){ // A3
            auto electRootMsg = dynamic_cast<ElectRootMsg*>(recvMsg);
            waitingList.remove(electRootMsg->getId());
            if(waitingList.size() == 1){
                parent = waitingList.front();
                waitingList.pop_front();
                auto saturationMsg = new ElectRootMsg("saturation", MsgKind::SATURATION);
                saturationMsg->setId(id);
                send(saturationMsg, "port$o", parent);
                status = Status::PROCESSING;
            }
             delete recvMsg;
        }
        else
            omnetpp::cRuntimeError("Invalid event for status active\n");
    }
    else if (status == Status::PROCESSING){
        if (recvMsg->getKind() == MsgKind::SATURATION){ // A4
            auto electionMsg = new ElectRootMsg("election", MsgKind::ELECTION);
            electionMsg->setId(id);
            send(electionMsg, "port$o", parent);
            status = Status::SATURATED;
            delete recvMsg;
        }
        else if(recvMsg->getKind() == MsgKind::TERMINATION){
            for (int i = 0; i < neighborhoodSize; i++){
                if(i != parent){
                    auto termMsg = new ElectRootMsg("term", MsgKind::TERMINATION);
                    termMsg->setId(id);
                    send(termMsg, "port$o", i);
                }
            }
            status = Status::FOLLOWER;
            delete recvMsg;
        }
        else
            omnetpp::cRuntimeError("Invalid event for status satured\n");
    }
    else if (status == Status::SATURATED){
        if (recvMsg->getKind() == MsgKind::ELECTION){ // A5
            auto electRootMsg = dynamic_cast<ElectRootMsg*>(recvMsg);
            if(id < electRootMsg->getId()){
                status = Status::LEADER;
                for (int i = 0; i < neighborhoodSize; i++){
                    if(i != parent){
                            auto termMsg = new ElectRootMsg("term", MsgKind::TERMINATION);
                            termMsg->setId(id);
                            send(termMsg, "port$o", i);
                    }
                }
            }else{
                status = Status::FOLLOWER;
                for (int i = 0; i < neighborhoodSize; i++){
                    if(i != parent){
                            auto termMsg = new ElectRootMsg("term", MsgKind::TERMINATION);
                            termMsg->setId(electRootMsg->getId());
                            send(termMsg, "port$o", i);
                    }
                }
            }
            delete recvMsg;
        }
    }
}
