#ifndef CommInfoNode_H
#define CommInfoNode_H

#include "IOstream.H"


namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class CommInfoNode Declaration
\*---------------------------------------------------------------------------*/

class CommInfoNode
{

public:

    CommInfoNode()
    {}


	virtual ~CommInfoNode()
	{}


	virtual void writeData(Ostream& os) const
	{}

};

}

#endif

