#ifndef CommProfiler_H
#define CommProfiler_H

#include "secCommInfo.H"
#include "timeStepSecCommInfo.H"
#include "iterSecCommInfo.H"
#include "baseCommInfo.H"
#include "LIFOStack.H"
#include "IOstreams.H"
namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class CommProfiler Declaration
\*---------------------------------------------------------------------------*/

class CommProfiler
{
    FIFOStack<timeStepSecCommInfo*> profileTree_;
	LIFOStack<secCommInfo*> secStack_;

	/*******Howe*****start**********/
	// blocking,schedule,nonBlocking,reduce
	// times && time
	int commTimes[4];
	double commTime[4];
	/*******Howe*****end************/	
public:

   CommProfiler();
   
   CommProfiler(Foam::word timeName);

   secCommInfo* curSec();

   timeStepSecCommInfo* curTimeSecPtr();


   bool stopRecordComm();

   
   void enterNewTimeStep(word time);

   label enterIterSec();

   void leaveIterSec(label iterid);

   void endSingleIter();
   

   void leaveOldTimeStep();


   void enterTimeStep(word time);


   void enterSec(Foam::word secName);


   void leaveSec(Foam::word secName);

   void leaveSec();


   void commRecord(baseCommInfo* baseCommInfoPtr);


   baseCommInfo* commRecord(label src,label dst,label size, Foam::UPstream::commsTypes type);


   void clearAll();

   void clearFinishedTime();


   void writeAll(Ostream& os) const;

   void writeFinishedTime(Ostream& os) const;


   void writeAndClearFinishedTime(Ostream& os);


   void writeAndClearAll(Ostream& os);

   /*******Howe********/
   void testTT() const;
   void sendRecord(int sendType,double tmpT);
   /*******Howe********/

    ~CommProfiler();

   
   friend Ostream& operator<<(Ostream& os, const CommProfiler& profiler);
   
};

}

#endif

