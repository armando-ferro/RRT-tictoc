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
#include <algorithm>
#include <proto_m.h>


using namespace omnetpp;
using namespace std;


vector<std::string>ctrlName={"","ACK","NACK","INFO"};

/**
 * In the previous model we just created another packet if we needed to
 * retransmit. This is OK because the packet didn't contain much, but
 * in real life it's usually more practical to keep a copy of the original
 * packet so that we can re-send it without the need to build it again.
 */



class MyLink : public cSimpleModule
{
  private:
    cMessage *timeoutPacket;  // holds pointer to the timeout self-message
    cPacket *packet;  // message that has to be re-sent on timeout
    cQueue queue;
    int retransmission=0;
    int protId=0;
    int lenCtrlPacket;
    int ns=0;
    int nr=0;
    vector<std::string>protocol={"None","S&W","GBN"};

  public:
    MyLink();
    virtual ~MyLink();

  protected:
    void sendCopyOf(cPacket *pkt);
    void sendPacket(cPacket *pkt);
    void sendCtrl(int type,char *link);
    void checkNewPacket();
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(MyLink);

MyLink::MyLink()
{
    timeoutPacket = nullptr;

    packet = nullptr;
}

MyLink::~MyLink()
{
    cancelAndDelete(timeoutPacket);
    delete packet;
}

void MyLink::initialize()
{

    string proto = par("protocol");
    lenCtrlPacket = par("lenCtrlPacket");
    protId = std::find(protocol.begin(),protocol.end(),proto)-protocol.begin();
    EV << "Initialization of NODE: " << getName() << "  PROT:" << protId << "\n";

    timeoutPacket = new(cMessage);
}

void MyLink::sendCtrl(int type,char *link)
{
    Proto *pkt=new Proto(ctrlName[type].c_str());
    pkt->setType(type);
    pkt->setBitLength(lenCtrlPacket);
    send(pkt,link);
}

void MyLink::handleMessage(cMessage *msg)
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
        Proto *pkt=check_and_cast<Proto *>(msg);

         if (uniform(0, 1) < 0.1) {
             EV << "\"Losing\" message " << msg << endl;
             bubble("message lost");
             delete msg;
         }
         else {
             if(pkt->getType()==TYPE_ACK){
                 EV << "RECIBIDO ACK\n";
                 delete packet;
                 packet = nullptr;      // To indicate finalization of Tx
                 cancelEvent(timeoutPacket);
                 checkNewPacket();
             } else if(pkt->getType()==TYPE_NACK){
                 EV << "NACK\n";
                ++retransmission;
                 cancelEvent(timeoutPacket);
                 sendCopyOf(packet);
                 scheduleAt(simTime()+par("timeoutPacket"), timeoutPacket);

             } else {
                 /* Take care with line occupation */
                 if(pkt->hasBitError()) {
                     EV << msg << " received with ERROR, sending back NACK.\n";
                     sendCtrl(TYPE_NACK,"link$o");
                 } else {
                     EV << msg << " received, sending back ACK.\n";
                     sendCtrl(TYPE_ACK,"link$o");
                 }
             }
             delete msg;

         }
    }
}

void MyLink::sendCopyOf(cPacket *pkt)
{
    // Duplicate packet and send the copy.
    cPacket *copy = (cPacket *)pkt->dup();
    send(copy, "link$o");
}

void MyLink::sendPacket(cPacket *pkt)
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
void MyLink::checkNewPacket()
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
    int id_Flow;

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
    id_Flow = par("id_Flow");
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
    sprintf(pktname, "%s%d-%d",ctrlName[TYPE_INFO].c_str(),id_Flow,seq);
    Proto *pkt = new Proto(pktname);
    pkt->setBitLength(len);
    pkt->setId_flow(id_Flow);
    pkt->setSeq(seq++);

    return pkt;
}

