#include "IPCPackets.h"

using namespace simplefs;


int Packet::getTotalLength()
{
	return this->getBaseLength() + this->getRemainderLength();
}

