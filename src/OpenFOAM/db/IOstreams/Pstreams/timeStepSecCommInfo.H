#ifndef timeStepSecCommInfo_H
#define timeStepSecCommInfo_H

#include "secCommInfo.H"
#include "IOstreams.H"
#include "List.H"
namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class CommInfoSec Declaration
\*---------------------------------------------------------------------------*/

class timeStepSecCommInfo
: 
	public secCommInfo
{
    scalar time_;

	myLabelList * procfaceCellsPtr_; // changed by Howe
	
      
public:

    inline scalar stringToScalar(const string& s)
    {
        string& str = const_cast<string&>(s);
        str.replaceAll(" ", "");
        str.replaceAll("D", "e");
        str.replaceAll("d", "e");
        return atof(str.c_str());
    }

	  timeStepSecCommInfo(word time)
	  :secCommInfo(time),
	  time_(stringToScalar(time))
	  {
        procfaceCellsPtr_ = NULL;
	  }

	  const scalar time() const
	  {
          return time_;
	  }


      void writeData(Ostream& os) const
      {
          secCommInfo::writeData(os);
		  //Pout << procfaceCellsPtr_ << "----------------------------\n";
		  os  << endl;
		  os  << "processorFvPatch: ";
		  os  << *procfaceCellsPtr_;
	  }
      myLabelList* getProcFaceCellsPtr()// changed by Howe
      {
          return procfaceCellsPtr_;
	  }

	  void setProcFaceCellsPtr(const myLabelList* cellPtr) //changed by Howe
	  {
          procfaceCellsPtr_ = const_cast<myLabelList*>(cellPtr);//changed by Howe
	  }


	  friend Ostream& operator<<(Ostream& os, const timeStepSecCommInfo& timeSec)
	  {
          // os<<nl<< timeSec.time_<<nl;
		  timeSec.writeData(os);
		  
		  return os;
	  }


	  ~timeStepSecCommInfo()
	  {
	      myLabelList* temp = procfaceCellsPtr_;
          delete temp;
		  procfaceCellsPtr_ = NULL;
	  }
	  
};
}

#endif

