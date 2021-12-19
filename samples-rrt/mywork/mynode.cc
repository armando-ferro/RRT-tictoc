//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

/**
 * In the previous model we just created another packet if we needed to
 * retransmit. This is OK because the packet didn't contain much, but
 * in real life it's usually more practical to keep a copy of the original
 * packet so that we can re-send it without the need to build it again.
 */
class MyNode : public cSimpleModule
{
  private:
    cMessage *timeoutPacket;  // holds pointer to the timeout self-message
    cPacket *packet;  // message that has to be re-sent on timeout
    cQueue queue;
    int retransmission=0;
    int protId=0;

  public:
    MyNode();
    virtual ~MyNode();

  protected:
    void sendCopyOf(cPacket *pkt);
    void sendPacket(cPacket *pkt);
    void checkNewPacket();
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(MyNode);

MyNode::MyNode()
{
    timeoutPacket = nullptr;

    packet = nullptr;
}

MyNode::~MyNode()
{
    cancelAndDelete(timeoutPacket);
    delete packet;
}

void MyNode::initialize()
{
    EV << "Initialization of NODE: " << getName()<< "\n";
    timeoutPacket = new(cMessage);
}

void MyNode::handleMessage(cMessage *msg)
{
    if (msg == timeoutPacket) {
         EV << "Timeout expired, send a new packet and restarting timer\n";
         sendCopyOf(packet);
         if(msg->isScheduled()){
             EV << "ERROR Message already SHEDULLED" << msg;
             cancelEvent(timeoutPacket);
         }
         scheduleAt(simTime()+par("timeoutPacket"), timeoutPacket);
    } else if(msg->arrivedOn("in")){
        EV << "NEW message" << msg << endl;
        queue.insert(msg);
        checkNewPacket();
    }
    else {  // message arrived
        cPacket *pkt=check_and_cast<cPacket *>(msg);

         if (uniform(0, 1) < 0.1) {
             EV << "\"Losing\" message " << msg << endl;
             bubble("message lost");
             delete msg;
         }
         else {
             if(!strcmp(pkt->getName(),"ACK")){
                 EV << "RECIBIDO ACK\n";
                 delete packet;
                 packet = nullptr;      // To indicate finalization of Tx
                 cancelEvent(timeoutPacket);
                 checkNewPacket();
             } else if(!strcmp(pkt->getName(),"NACK")){
                 EV << "NACK\n";
                ++retransmission;
                 cancelEvent(timeoutPacket);
                 sendCopyOf(packet);
                 scheduleAt(simTime()+par("timeoutPacket"), timeoutPacket);

             } else {
                 /* Take care with line occupation */
                 if(pkt->hasBitError()) {
                     EV << msg << " received with ERROR, sending back NACK.\n";
                     send(new cPacket("NACK",0,par("lenCtrlPacket")),"link$o");
                 } else {
                     EV << msg << " received, sending back ACK.\n";
                     send(new cPacket("ACK",0,par("lenCtrlPacket")),"link$o");
                 }
             }
             delete msg;

         }
    }
}

void MyNode::sendCopyOf(cPacket *pkt)
{
    // Duplicate packet and send the copy.
    cPacket *copy = (cPacket *)pkt->dup();
    send(copy, "link$o");
}

void MyNode::sendPacket(cPacket *pkt)
{
    retransmission=0;
    sendCopyOf(packet);
    if(timeoutPacket->isScheduled()){
        // Trace impossible events
         EV << "ERROR in sendPacket Message already SHEDULLED" << timeoutPacket;
         cancelEvent(timeoutPacket);
    }
    scheduleAt(simTime()+par("timeoutPacket"), timeoutPacket);
}
void MyNode::checkNewPacket()
{
    if(queue.isEmpty()){
          packet = nullptr;
    }else if(!packet){  // Only if previous packet has been completed
          sendPacket(packet = (cPacket *) queue.pop());
    }
}


class Injector : public cSimpleModule
{
  private:
    cMessage *timeoutEvent;  // holds pointer to the timeout self-message
    int seq=0;  // message sequence number
    int lenCtrl; // Control header length

  public:
    Injector();
    virtual ~Injector();

  protected:
    virtual cPacket *generateNewPacket(int);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Injector);

Injector::Injector()
{
    timeoutEvent = nullptr;
}

void Injector::initialize()
{
    simtime_t delay;
    delay =par("delayTime");
    lenCtrl = (int) par("lenCtrlPacket");
    EV << "Initialization of INJECTION nextdelay=" << delay << endl;

    timeoutEvent = new(cMessage);
    scheduleAt(simTime()+ delay, timeoutEvent);
}
Injector::~Injector()
{
    cancelAndDelete(timeoutEvent);
}

void Injector::handleMessage(cMessage *msg)
{
 //   EV << "RECIBIDO EVENTO EN INYEcTOR\n";

    if (msg == timeoutEvent) {
    double len;
        len = par("lenPacket");
  //      EV << "Timeout expired, send(len=" << len << ") a new packet and restarting timer\n";

        send(generateNewPacket(len+lenCtrl), "out");
 //       EV << "Packet SEND len=" << len <<"\n";

        scheduleAt(simTime()+par("delayTime"), timeoutEvent);
    }
}

cPacket *Injector::generateNewPacket(int len)
{
    // Generate a message with a different name every time.
    char pktname[20];
    sprintf(pktname, "PKT-%d", ++seq);
    cPacket *pkt = new cPacket(pktname,0,len);
    return pkt;
}

