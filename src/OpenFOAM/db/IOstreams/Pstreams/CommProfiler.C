#include "CommProfiler.H"

   Foam::CommProfiler::CommProfiler()
   :
       profileTree_ (FIFOStack<timeStepSecCommInfo*>()),
       secStack_( LIFOStack<secCommInfo*>())
 
  {
       timeStepSecCommInfo* timeCommSecPtr = new timeStepSecCommInfo("-1");
	   timeCommSecPtr->enterSec();
	   profileTree_.push(timeCommSecPtr);
	   secStack_.push(timeCommSecPtr);
   	
	/*********Howe*****start***********/
	for(int i=0; i<4;i++)
		{
			commTimes[i]=0;
			commTime[i]=0;
		}
	/*********Howe*****end************/
   }


   Foam::secCommInfo* Foam::CommProfiler::curSec()
   {
       return secStack_.top();
   }


   Foam::timeStepSecCommInfo* Foam::CommProfiler::curTimeSecPtr()
   {
       return (timeStepSecCommInfo*)(secStack_.bottom() );
   }



   bool Foam::CommProfiler::stopRecordComm()
   {
        if(secStack_.top()->sec().secName() == "ITER")
        {
             iterSecCommInfo* iterCommSecPtr = (iterSecCommInfo*)curSec();
			 if(iterCommSecPtr->iterNum())
			 {
                 return 1;
			 }
			 else
			 {
                 return 0;

			 }

		}
		return 0;

   }

   void Foam::CommProfiler::enterNewTimeStep(word time)
   {   
        //if(!stopRecordComm())
        {
		   timeStepSecCommInfo* timeCommSecPtr = new timeStepSecCommInfo(time);
		   timeCommSecPtr->enterSec();
		   profileTree_.push(timeCommSecPtr);
		   // secStack_.clear();
		   secStack_.push(timeCommSecPtr); //?????Âµ?Ê±??Á´??Ö¸??

      	}
   }

   Foam::label Foam::CommProfiler::enterIterSec()
   {
       if(!stopRecordComm()){
            iterSecCommInfo* iterCommSecPtr = new iterSecCommInfo();
			iterCommSecPtr->enterSec();
			curSec()->push(iterCommSecPtr);
			secStack_.push(iterCommSecPtr);

			return iterCommSecPtr->iterIdPlus(); //return the iterId
			

       	}
   }

   void Foam::CommProfiler::leaveIterSec(Foam::label iterid )
   {


	   if(secStack_.top()->sec().secName() == "ITER")
	   {
			iterSecCommInfo* iterCommSecPtr = (iterSecCommInfo*)curSec();
			if(iterid == iterCommSecPtr->iterId())
			{
               curSec()->leaveSec();
	           secStack_.pop();
			   iterCommSecPtr->iterIdDecr();

			}
	   
	   }
	   
	   else
	   {

			Pout<<"ERROR! Unfinished Section in leaveIterSec! Not ITER!"<<endl;

	   }


   }

   void Foam::CommProfiler::endSingleIter()
   {
       if(secStack_.top()->sec().secName() == "ITER" )
       {
       	   iterSecCommInfo* iterTemp = (iterSecCommInfo*)curSec();
		   iterTemp->iterNumPlus();
       }
	   else
	   {
           Pout<<"ERROR! Unfinished Section in endSingleIter! Not ITER!"<<endl;
	   } 
   }


   void Foam::CommProfiler::leaveOldTimeStep()
   {
       //if(!stopRecordComm())
       {
           if((curSec()!=curTimeSecPtr())||!curSec()->sec().isInSec())
           {

			   Pout<<"ERROR! Unfinished Section in leaveOldTimeStep! "<<(curSec()!=curTimeSecPtr()) 
			   	   << (curSec()->sec().isInSec())<<endl<<secStack_.size()<<endl;
	       }
	       else
	       {
		       curTimeSecPtr()->leaveSec();
		       secStack_.clear();//??????Ò»??Ê±?ä²½Á´?í£¬??????sectionÕ»
	       }
       	}
   }


   void Foam::CommProfiler::enterTimeStep(word time)
   {   
       //if(!stopRecordComm())
       {
	       leaveOldTimeStep();
           enterNewTimeStep(time);  
       	}
   }


   void Foam::CommProfiler::enterSec(Foam::word secName)
   {   
       if(!stopRecordComm()){
	       secCommInfo* commSecPtr = new secCommInfo(secName);
	       commSecPtr->enterSec();
	       curSec()->push(commSecPtr);
	       secStack_.push(commSecPtr);  
       	}
   }


   void Foam::CommProfiler::leaveSec(Foam::word secName)
   {
       if(!stopRecordComm()){
            if(secStack_.top()->sec().secName() == secName)
           {
               curSec()->leaveSec();
		       secStack_.pop();
	       }
          else
	      {
              Pout<< "ERROR! No Section Match!"<<endl;
		  
	      } 
       	}
   }

   void Foam::CommProfiler::leaveSec()
   {
       if(!stopRecordComm()){
           curSec()->leaveSec();
	       secStack_.pop();
   	   }
   }


   void Foam::CommProfiler::commRecord(baseCommInfo* baseCommInfoPtr)
   {
       if(!stopRecordComm()){
	       CommInfoNode* newNode = (CommInfoNode*)(baseCommInfoPtr);
	       curSec()->push(newNode);
   	    }
   }


   Foam::baseCommInfo* Foam::CommProfiler::commRecord(label src,label dst,label size, Foam::UPstream::commsTypes type)
   {
       if(!stopRecordComm()){
           baseCommInfo* baseInfoPtr = new baseCommInfo(src,dst,size, type);
	     commRecord(baseInfoPtr);
		   return baseInfoPtr;
       	}
	   return NULL;
   }


   void Foam::CommProfiler::clearAll()
   {
       timeStepSecCommInfo* tmpPtr;
	   for
	   (
	       FIFOStack<timeStepSecCommInfo*>::const_iterator iter = profileTree_.begin();
	       iter != profileTree_.end();
	       ++iter
	   )
	   {
           if(tmpPtr!=NULL)
           {
			   tmpPtr = iter();
			   delete tmpPtr;
			   
           }
	   }
	   profileTree_.clear();
   }

   void Foam::CommProfiler::clearFinishedTime()
   {

       if(profileTree_.top()->sec().isInSec())
       {
           timeStepSecCommInfo* tmpPtr;
		   for
		   (
		       FIFOStack<timeStepSecCommInfo*>::const_iterator iter = profileTree_.begin();
		       iter != profileTree_.end();
		       ++iter
		   )
		   {
			   tmpPtr = iter();
			   if(tmpPtr!=profileTree_.top())
			   {
			       if(tmpPtr!=NULL)
	               {
					   delete tmpPtr;
			       }
			   }
		   }
		   tmpPtr=profileTree_.top();
		   profileTree_.clear();
		   profileTree_.push(tmpPtr);
	   }
	   else 
	   {
	   clearAll();
	   }
	   
   }


   void Foam::CommProfiler::writeAll(Ostream& os) const
   {

       //Pout<< nl << "RXG1 size: " <<  profileTree_.size()<<nl;
	   for
	   (
	       FIFOStack<timeStepSecCommInfo*>::const_iterator iter = profileTree_.begin();
	       iter != profileTree_.end();
	       ++iter
	   )
	   {
           //Pout<<iter()<<"oooooooooo"<<endl;
           os<< *(iter());
//	   os<< "********Howe*********" << tt << "*********" << endl;
	   
	   }
   }


   void Foam::CommProfiler::writeFinishedTime(Ostream& os) const
   {

       //Pout<< nl << "RXG2 size: " <<  profileTree_.size()<<endl;
       for
	   (
	       FIFOStack<timeStepSecCommInfo*>::const_iterator iter = profileTree_.begin();
	       iter != profileTree_.end();
	       ++iter
	   )
	   {
	      
		   //Pout<<iter()<<"xxxxxxxoooxxxxxx"<<endl;
		   
		   if(!(iter()->sec().isInSec()))
		   {
			  os<<*(iter());
		   }
	//os<< "********Howe*********" << tt << "*********" << endl;

	   }
   }


   void Foam::CommProfiler::writeAndClearFinishedTime(Ostream& os)
   {
       writeFinishedTime(os);
	   clearFinishedTime();
   }


   void Foam::CommProfiler::writeAndClearAll(Ostream& os)
   {
       leaveOldTimeStep();
	   writeAll(os);
	   clearAll();
   }

   void Foam::CommProfiler::testTT() const // added by Howe
   {
	Pout << "**************************The total send times are:****************************" << endl
             << "Bsend:" << commTimes[0] << "    Total time:" << commTime[0] << endl
             << "Send:" << commTimes[1] << "   Total time:" << commTime[1] << endl
             << "Isend:" << commTimes[2] << " Total time:" << commTime[2] << endl
             << "Allreduce:" << commTimes[3] << "      Total time:" << commTime[3] << endl;
   }  

   void Foam::CommProfiler::sendRecord(int sendType,double tmpT) 
   {
	switch (sendType)
	{
		case 0:commTimes[0]++;	commTime[0]+=tmpT;  	break; // Bsend
		case 1:commTimes[1]++;	commTime[1]+=tmpT;	break; // Send
		case 2:commTimes[2]++;	commTime[2]+=tmpT;	break; // Isend
		case 3:commTimes[3]++;	commTime[3]+=tmpT;	break; // Allreduce
		default:	break;
	}
   }

    Foam::CommProfiler::~CommProfiler()
   {
	   this->clearAll();
   }

   
   Foam::Ostream& Foam::operator<<(Ostream& os, const CommProfiler& profiler)
   {
       profiler.writeAll(os);
	   return os;
   }
	
