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
        int id_Flow = default(0);		// Flow identification
        @display("i=block/source");
    gates:
        output out;
}


simple MyLink
{
    parameters:
        volatile double timeoutPacket @unit(s);   // timeout to resend packet
        int lenCtrlPacket @unit(bits);   // Control packet length function
        string protocol = default("S&W");
        @display("i=block/queue");
    gates:
        input in @loose;
        output out @loose;
        inout link;
}

module Switch
{
    parameters:
        @display("i=block/broadcast;bgb=277,114");
        @display("bgb=218,119");
    gates:
        input sourcein @loose;
        output sinkout @loose;
        inout links[];
    submodules:
        switch: CoreSwitch {
            @display("p=95,60;i=block/switch");
        }
        nlink[sizeof(links)]: MyLink {
            @display("p=206,60");
        }
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
    @display("bgb=368,195");
    types:
        channel RealChannel extends ned.DatarateChannel
        {
            delay = default(0.1s);
            datarate = default(9600000bps);
            per = default(0.1);
        }
    submodules:
        injector1: Injector {
            parameters:
                @display("i=,cyan;p=26,103");
                id_Flow = 1;
        }
        injector2: Injector {
            parameters:
                @display("i=,cyan;p=91.840004,12.88");
                id_Flow = 2;
        }
        injector5: Injector {
            parameters:
                @display("i=,cyan;p=91,175");
                id_Flow = 3;
        }
        node1: Switch {
            parameters:
                @display("i=,gold;p=114,103");
        }
        node2: Switch {
            parameters:
                @display("i=,gold;p=171,59");
        }
        node3: Switch {
            parameters:
                @display("i=,gold;p=255,59");
        }
        node4: Switch {
            parameters:
                @display("i=,gold;p=255,138");
        }
        node5: Switch {
            parameters:
                @display("i=,gold;p=171,138");
        }

    connections:
        injector1.out --> IdealChannel --> node1.sourcein;
        injector2.out --> IdealChannel --> node2.sourcein;
        injector5.out --> IdealChannel --> node5.sourcein;
        node1.links++ <--> RealChannel <--> node2.links++;
        node1.links++ <--> RealChannel <--> node5.links++;
        node2.links++ <--> RealChannel <--> node3.links++;
        node2.links++ <--> RealChannel <--> node4.links++;
        node4.links++ <--> RealChannel <--> node3.links++;
        node5.links++ <--> RealChannel <--> node2.links++;
        node5.links++ <--> RealChannel <--> node4.links++;

}

