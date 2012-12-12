#ifndef Section_H
#define Section_H

#include"UPstream.H"


namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class Section Declaration
\*---------------------------------------------------------------------------*/

class Section
{
    Foam::word secName_;
	bool isInSec_;

public:

    Section()
	:
		secName_("Unknow Section"),
		isInSec_(1)
    {}

    Section(Foam::word secName, bool isInSec)
	:
	    secName_(secName),
		isInSec_(isInSec)
    {}

	Section(Foam::word secName)
	:
	    secName_(secName),
		isInSec_(0)
    {}


	Section(bool isInSec)
	:
	    secName_("Unknow Section"),
		isInSec_(isInSec)
    {}


	const Foam::word& secName() const
	{
        return secName_;
	}

	const bool isInSec() const
	{
        return isInSec_;
	}

    void secName(Foam::word name)
	{
        secName_ = name;
	}

	void isInSec(bool flag)
	{
        isInSec_ = flag;
	}

	void enter()
	{
       isInSec_ = 1;
	}

	void leave()
	{
       isInSec_ = 0;
	}

	~Section()
	{}

	friend Ostream& operator<<(Ostream& os, const Section& section)
	{
		os	<< nl
			<< "Section--" <<section.secName_ << ": ";
		if(section.isInSec_)
			os<<"begin";
		else
			os<<"end";

		return os;
	}


};

}

#endif
