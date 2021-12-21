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


bool isProbEnd(double p)
{
        if(p<=1.0)
                return(false);
        return(true);
}

void acumVectorDouble(std::vector<double>&array)
{
double acum=0.0;

        for(auto &it:array) {
                it = (acum += (double) it);
        }
        if(acum < 1.0)
                array.push_back(1.0);
}

void CoreSwitch::initialize()
{
    const char *routing = par("routing").stringValue();
    EV << "ROUTING:" << routing << endl;
    weigth = cStringTokenizer(routing).asDoubleVector();
     EV << "WEIGTH:";

    if(weigth.size())
        acumVectorDouble(weigth);

    for (auto it = weigth.cbegin(); it != weigth.cend(); it++) {
        EV << *it << ' ';
    }
    EV << endl;

}

void CoreSwitch::handleMessage(cMessage *msg)
{
    double rnd;
    int i;
    if(weigth.size()==0){
        delete msg;
    } else {
        rnd=uniform(0, 1);
        for(i=0; i< weigth.size();i++) {
            if(rnd <= weigth[i])
                break;
        }
        send(msg,"out",i);
    }
}
