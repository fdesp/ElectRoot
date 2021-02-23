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
        timer = new omnetpp::cMessage("timer", MsgKind::TIMER);
        scheduleAt(par("startTime"), timer);
    }
    status = Status::IDLE;
    neighborhoodSize = gateSize("port$o");
    id = getIndex();
}

void ElectRoot::handleMessage(omnetpp::cMessage* recvMsg){
    if (status == Status::IDLE){
        if (recvMsg->getKind() == MsgKind::TIMER){ // A1
            auto wakeupMsg = new ElectRootMsg("activate", MsgKind::ACTIVATION);
            wakeupMsg->setId(id);
            for (int i = 0; i < neighborhoodSize - 1; i++){
                send(wakeupMsg->dup(), "port$o", i);
                waitingList.push_front(i);
            }
            send(wakeupMsg, "port$o", neighborhoodSize - 1);
            waitingList.push_front(neighborhoodSize - 1);

            if(waitingList.size() == 1)
                becomeProcessing();
            else
                status = Status::WAITING;
        } else if (recvMsg->getKind() == MsgKind::ACTIVATION){ // A2
            for (int i = 0; i < neighborhoodSize - 1; i++)
                waitingList.push_front(i);
            waitingList.push_front(neighborhoodSize - 1); 
            if(waitingList.size() == 1){
                becomeProcessing();
                delete(recvMsg);
            }else{
                localFlooding(recvMsg);
                status = Status::WAITING;
            }
        } else if(recvMsg->getKind() == MsgKind::SATURATION){
            waitingList.remove(recvMsg->getArrivalGate()->getIndex());
            if(waitingList.size() == 1){
                becomeProcessing();
                delete(recvMsg);
            }else{
                localFlooding(recvMsg);
                status = Status::WAITING;
            }
        }
        else
            omnetpp::cRuntimeError("Invalid event for status idle\n");
    }
    else if (status == Status::WAITING){
        if (recvMsg->getKind() == MsgKind::SATURATION){ // A3
            waitingList.remove(recvMsg->getArrivalGate()->getIndex());
            if(waitingList.size() == 1){
                becomeProcessing();
                delete(recvMsg);
            }
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
                for (int i = 0; i < neighborhoodSize; i++){
                    if(i != parent){
                        auto termMsg = new ElectRootMsg("term", MsgKind::TERMINATION);
                        termMsg->setId(id);
                        send(termMsg, "port$o", i);
                    }
                }
                status = Status::LEADER;
            }else{
                for (int i = 0; i < neighborhoodSize; i++){
                    if(i != parent){
                        auto termMsg = new ElectRootMsg("term", MsgKind::TERMINATION);
                        termMsg->setId(electRootMsg->getId());
                        send(termMsg, "port$o", i);
                    }
                }
                status = Status::FOLLOWER;
            }
            delete recvMsg;
        }
    }
}


void ElectRoot::becomeProcessing(){  
    parent = waitingList.front();
    waitingList.pop_front();
    auto saturationMsg = new ElectRootMsg("saturation", MsgKind::SATURATION);
    saturationMsg->setId(id);
    send(saturationMsg, "port$o", parent);
    status = Status::PROCESSING;
}


void ElectRoot::refreshDisplay() const{
    std::string info = status.str();
    getDisplayString().setTagArg("t", 0, info.c_str());
    if(info == "LEADER"){
        getDisplayString().setTagArg("t", 2, "red");
    }
}
