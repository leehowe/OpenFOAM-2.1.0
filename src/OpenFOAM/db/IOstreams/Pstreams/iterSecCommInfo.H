#ifndef iterSecCommInfo_H
#define iterSecCommInfo_H

#include "secCommInfo.H"
#include "IOstreams.H"

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class CommInfoSec Declaration
\*---------------------------------------------------------------------------*/

class iterSecCommInfo
: 
	public secCommInfo
{
    label iterNum_;

	static label iterId_;
	
public:


	  iterSecCommInfo()
	  :secCommInfo(word("ITER")),
	  iterNum_(0)
	  {}

	  const label iterNum() const
	  {
	      return iterNum_;
	  }

      label iterIdPlus()
      {
          return ++iterId_;
	  }

	  label iterIdDecr()
	  {
          return --iterId_;
	  }

      label iterId()
      {
          return iterId_;
	  }
	  label iterNumPlus()
	  {
	      return ++iterNum_;
	  }


      void writeData(Ostream& os) const
      {
          os << nl<< "IterNum:" << iterNum_;
          secCommInfo::writeData(os);
		  os << nl<< "IterEnd" <<nl;
          
	  }

	  friend Ostream& operator<<(Ostream& os, const iterSecCommInfo& iterSec)
	  {
          // os<<nl<< timeSec.time_<<nl;
		  iterSec.writeData(os);
		  
		  return os;
	  }


	  ~iterSecCommInfo()
	  {
	  }
	  
};


}


#endif

