#ifndef secCommInfo_H
#define secCommInfo_H

#include "CommInfoNode.H"
#include "baseCommInfo.H"
#include "Section.H"
#include "FIFOStack.H"
#include "typeInfo.H"

#include <sstream>



namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class CommInfoSec Declaration
\*---------------------------------------------------------------------------*/

class secCommInfo
: 
	public CommInfoNode
{
   
   Section sec_;
   FIFOStack<CommInfoNode*> commPtrList_;

public:
   secCommInfo()
   :
      sec_(Section()),
	  commPtrList_(FIFOStack<CommInfoNode*>())
   {}

   secCommInfo(Section sec)
   :
      sec_(sec),
	  commPtrList_(FIFOStack<CommInfoNode*>())
   {}


   secCommInfo(Foam::word name)
   :
      sec_(Section(name, 1)),
	  commPtrList_(FIFOStack<CommInfoNode*>())
   {}

   
   static Foam::word toStr(const scalar t)
   {
    std::ostringstream buf;
    buf.setf(ios_base::fmtflags(0), ios_base::floatfield);
    buf.precision(6);
    buf << t;
    return buf.str();
   }


   const Section& sec() const
   {
       return sec_;
   }


   const FIFOStack<CommInfoNode*>& commList() const
   {
       return commPtrList_;
   }


   Section& sec()
   {
       return sec_;
   }


   FIFOStack<CommInfoNode*>& commPtrList()
   {
       return commPtrList_;
   }


   void enterSec()
   {
       sec_.enter();
   }


   void leaveSec()
   {
       sec_.leave();
   }


   void push(CommInfoNode* nodePtr)
   {
       commPtrList_.push(nodePtr);
   }


   void pop()
   {
       commPtrList_.pop();
   }


   CommInfoNode* top()
   {
       return commPtrList_.top();
   }


   CommInfoNode* bottom()
   {
       return commPtrList_.bottom();
   }


   void clear()
   {
       CommInfoNode* tmpPtr;
	   
       for
	   (
	       FIFOStack<CommInfoNode*>::const_iterator iter = commPtrList_.begin();
	       iter != commPtrList_.end();
	       ++iter
	   )
	   {
	       tmpPtr=iter();
		   delete tmpPtr;
	   }
       commPtrList_.clear();
   }
   

	void writeData(Ostream& os) const
	{
		 
		 //os<<nl<<sec_.secName()<< ": begin" ;

		 
		 for
		(
			FIFOStack<CommInfoNode*>::const_iterator iter = commPtrList_.begin();
			iter != commPtrList_.end();
			++iter
		)
		{
			(*(iter())).writeData(os);
		}

		/*
		 if(!sec_.isInSec())
		 	os<<nl<<sec_.secName()<< ": end" ;
		*/
	}


   	friend Ostream& operator<<(Ostream& os, const secCommInfo& node)
   	{
        node.writeData(os);
		return os;
	}

	
	~secCommInfo()
	{
		this->clear();
	}
   
};

}

#endif

