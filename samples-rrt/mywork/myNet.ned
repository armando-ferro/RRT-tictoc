//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


import ned.DatarateChannel;
import ned.IdealChannel;

simple Injector
{
    parameters:
        volatile double delayTime @unit(s);   // delay before sending back message
        volatile double lenPacket @unit(bits);   // Packet length function
        int lenCtrlPacket @unit(bits);   // Control packet length function
        @display("i=block/source");
    gates:
        output out;
}


simple MyNode
{
    parameters:
        volatile double timeoutPacket @unit(s);   // timeout to resend packet
        int lenCtrlPacket @unit(bits);   // Control packet length function
        @display("i=block/routing");
    gates:
        input in @loose;
    	output out @loose;
        inout link;
}

module Switch
{
    gates:
        input sourcein @loose;
    	output sinkout @loose;        
        inout links[];      
    submodules:
        switch: CoreSwitch;
        nlink[sizeof(links)]:MyNode;
    connections:
        sourcein --> switch.sourcein;
        sinkout <-- switch.sinkout;
        for i=0..sizeof(links)-1 {
            switch.out++ --> nlink[i].in;
            switch.in++ <-- nlink[i].out; 
        	links[i] <--> nlink[i].link;
        }
}

network myNet
{
    @display("bgb=210,108");
    submodules:
        injector: Injector {
            parameters:
              @display("i=,cyan;p=28,26.32");
        }
        node1: Switch {
            parameters:
                @display("i=,cyan;p=85.12,60.48");
        }
        node2: Switch {
            parameters:
                @display("i=,gold;p=185,61");
        }
    connections:
        injector.out --> IdealChannel --> node1.sourcein;
        node1.links++ <--> DatarateChannel {  delay = default(0.1s); datarate = default(9600000bps); per = default(0.1); } <--> node2.links++;
}

