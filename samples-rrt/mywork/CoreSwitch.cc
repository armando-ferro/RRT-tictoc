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

#include "CoreSwitch.h"

Define_Module(CoreSwitch);


void CoreSwitch::initialize()
{
    const char *routing = par("routing").stringValue();
    EV << "ROUTING:" << routing << endl;
    weigth = cStringTokenizer(routing).asDoubleVector();
    double acum=0.0;
    for(auto &i:weigth) {
        acum+=i;
        if(acum >= 1.0){
            i=1.0;
            break;
        } else
            i=acum;
    }
    EV << "WEIGTH:" << acum << endl;

    // TODO - Generated method body
}

void CoreSwitch::handleMessage(cMessage *msg)
{
    double rnd;
    int i;

    rnd=uniform(0, 1);

    for(i=0; i< weigth.size();i++) {
        if(rnd <= weigth[i])
            break;
    }

    // TODO - Generated method body
    send(msg,"out",i);
}
