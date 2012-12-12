#ifndef baseCommInfo_H
#define baseCommInfo_H

#include "UPstream.H"
#include "CommInfoNode.H"



namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class IPstream Declaration
\*---------------------------------------------------------------------------*/

class baseCommInfo
: 
    public CommInfoNode
{
    label srcProc_;
	label dstProc_;
	label size_;

	Foam::UPstream::commsTypes commType_;

    double startTime_;
	double endTime_;

	public:

	baseCommInfo()
	:
        srcProc_(-1),
	    dstProc_(-1),
	    size_(0),
	    commType_(Foam::UPstream::nonBlocking),
	    startTime_(-1),
	    endTime_(-1)
   {}



	baseCommInfo(label src, label dst, label size, Foam::UPstream::commsTypes commType)
	:
	   srcProc_(src),
	   dstProc_(dst),
	   size_(size),
	   commType_(commType),
	   startTime_(UPstream::profClockTime.elapsedTime()),
	   endTime_(-1)
	{}


	baseCommInfo(label src, label dst, label size, Foam::UPstream::commsTypes commType,double startTime)
	:
	   srcProc_(src),
	   dstProc_(dst),
	   size_(size),
	   commType_(commType),
	   startTime_(startTime),
	   endTime_(-1)
	{}

	void setEndTime(double endtime)
	{
      endTime_ = endtime;
	}

	void setEndTime()
	{
      endTime_ = UPstream::profClockTime.elapsedTime();
	}

	const label& srcProc() const
	{
        return srcProc_;
	}

	const label& dstProc() const
	{
        return dstProc_;
	}

	const label& size() const
	{
        return size_;
	}

	const label& commType() const
	{
        return commType_;
	}


	void srcProc(label src)
	{
        srcProc_ = src;
	}

	void dstProc(label dst)
	{
        dstProc_ = dst;
	}

	void size(label size)
	{
        size_ = size;
	}

	void commType(Foam::UPstream::commsTypes commType)
	{
        commType_ = commType;
	}


	void writeData(Ostream& os) const
	{
        os  << nl
			<< "COM_Begin"<<token::TAB
			<< srcProc_  << token::TAB
			<< dstProc_  << token::TAB
			<< size_     << token::TAB
			<< commType_ << token::TAB
			//<< startTime_<< token::TAB
			//<< endTime_  << token::TAB
            << "COM_End";
	}
	
	friend Ostream& operator<<(Ostream& os, const baseCommInfo& baseInfo)
	{
        baseInfo.writeData(os);

		return os;
	}

	~baseCommInfo()
	{}

};

}


#endif

