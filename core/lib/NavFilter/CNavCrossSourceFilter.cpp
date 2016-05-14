#include "CNavCrossSourceFilter.hpp"

namespace gpstk
{
   CNavCrossSourceFilter ::
   CNavCrossSourceFilter()
   {
   }

   void CNavCrossSourceFilter ::
   validate(NavMsgList& msgBitsIn, NavMsgList& msgBitsOut)
   {
      NavMsgList::const_iterator nmli;
      for (nmli = msgBitsIn.begin(); nmli != msgBitsIn.end(); nmli++)
      {
         CNavFilterData *fd = dynamic_cast<CNavFilterData*>(*nmli);
         if (fd->timeStamp != currentTime)
         {
               // different time, so check out what we have
            examineMessages(msgBitsOut);
            groupedNav.clear();
            currentTime = fd->timeStamp;
         }
            // add the message to our collection
         groupedNav[fd->prn][fd].push_back(*nmli);
      }
   }

   void CNavCrossSourceFilter ::
   finalize(NavMsgList& msgBitsOut)
   {
      examineMessages(msgBitsOut);
      groupedNav.clear();
      currentTime.reset();
   }

   void CNavCrossSourceFilter ::
   examineMessages(NavMsgList& msgBitsOut)
   {
      NavMap::const_iterator nmi;
      MessageMap::const_iterator smi;
         // loop over each PRN/SV
      for (nmi = groupedNav.begin(); nmi != groupedNav.end(); nmi++)
      {
            // count of unique messages
         size_t uniqueBits = nmi->second.size();
            // count of total messages
         size_t msgCount = 0;
            // store the vote winner here
         CNavFilterData *winner = NULL;
            // store the largest number of "votes" for a subframe here
         size_t voteCount = 0;
         for (smi = nmi->second.begin(); smi != nmi->second.end(); smi++)
         {
            size_t msgs = smi->second.size();
            msgCount += msgs;
               // minimum # of useful votes
            if ((msgs > voteCount) && (msgs >= 2))
            {
               voteCount = msgs;
               winner = smi->first;
            }
         }
         if (msgCount < 2)
            winner = NULL; // not enough messages to have a useful vote

            // If winner is NULL, i.e. there is no winner, all
            // messages will be rejected below.  Otherwise only the
            // winners will be accepted.
         for (smi = nmi->second.begin(); smi != nmi->second.end(); smi++)
         {
            if (smi->first == winner)
               accept(smi->second, msgBitsOut);
            else
               reject(smi->second);
         }
      }
   }
}
