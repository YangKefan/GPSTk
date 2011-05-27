#pragma ident "$Id$"

/// @file SP3EphemerisStore.hpp
/// Store a tabular list of position and clock bias (perhaps also velocity and clock
/// drift) data from SP3 file(s) for several satellites; access the tables to compute
/// values at any timetag, within the limits of the data, from this table via
/// interpolation.
/// An option allows assigning the clock store to RINEX clock files, with separate
/// timestep and interpolation algorithm.

#ifndef GPSTK_SP3_EPHEMERIS_STORE_INCLUDE
#define GPSTK_SP3_EPHEMERIS_STORE_INCLUDE

#include <map>
#include <iostream>

#include "Exception.hpp"
#include "SatID.hpp"
#include "CommonTime.hpp"
#include "XvtStore.hpp"

#include "FileStore.hpp"
#include "ClockSatStore.hpp"
#include "PositionSatStore.hpp"

#include "SP3Header.hpp"
#include "RinexClockHeader.hpp"

namespace gpstk
{

   /** @addtogroup ephemstore */
   //@{

   /// Store position and clock bias (and perhaps velocity and drift) data from SP3
   /// files, using (separate) stores based on TabularSatStore. An option allows the
   /// clock store to be taken from RINEX clock files instead.
   /// Keep a FileStore for the SP3 input files, and for the RINEX clock files, if
   /// they are used. Inherit XvtStore for the interface it defines.
   class SP3EphemerisStore : public XvtStore<SatID>
   {

   // member data
   private:
      /// PositionSatStore for SP3 ephemeris data
      PositionSatStore posStore;

      /// ClockSatStore for SP3 OR RINEX clock data
      ClockSatStore clkStore;

      /// FileStore for the SP3 input files
      FileStore<SP3Header> SP3Files;

      /// FileStore for the (optional) RINEX clock input files
      FileStore<RinexClockHeader> clkFiles;

      /// flag indicating whether the clock store contains data from SP3 (true,
      /// the default) or Rinex clock (false) files
      bool useSP3clock;

      /// Flag to reject clock data when there are bad positions, default true.
      /// NB this flag has no effect if the clock store comes from RINEX clock files.
      bool rejectBadPosFlag;

      /// Flag to reject position data when there are bad clocks, default true.
      /// NB this flag has no effect if the clock store comes from RINEX clock files.
      bool rejectBadClockFlag;

   // member functions
   public:

      /// Default constructor
      SP3EphemerisStore() throw() : useSP3clock(true),
                                       rejectBadPosFlag(true),
                                       rejectBadClockFlag(true)
      {
         // set default gap and interval checking
         setPosGapInterval(901.);
         setPosMaxInterval(8101.);
         setClockGapInterval(30.1);
         setClockMaxInterval(60.1);
      }

      /// Destructor
      virtual ~SP3EphemerisStore()
      {}

   // XvtStore interface:

      /// Returns the position and clock offset of the indicated
      /// object in ECEF coordinates (meters) at the indicated time.
      /// @param[in] sat the satellite of interest
      /// @param[in] ttag the time to look up
      /// @return the Xt of the object at the indicated time
      /// @throw InvalidRequest If the request can not be completed for any
      ///    reason, this is thrown. The text may have additional
      ///    information as to why the request failed.
      virtual Xt getXt(const SatID& sat, const CommonTime& ttag)
         const throw(InvalidRequest);

      /// Returns the position, velocity, and clock offset of the indicated
      /// object in ECEF coordinates (meters) at the indicated time.
      /// @param[in] sat the satellite of interest
      /// @param[in] ttag the time to look up
      /// @return the Xvt of the object at the indicated time
      /// @throw InvalidRequest If the request can not be completed for any
      ///    reason, this is thrown. The text may have additional
      ///    information as to why the request failed.
      virtual Xvt getXvt(const SatID& sat, const CommonTime& ttag)
         const throw(InvalidRequest);

      /// Dump information about the store to an ostream.
      /// @param[in] os ostream to receive the output; defaults to std::cout
      /// @param[in] detail integer level of detail to provide; allowed values are
      ///    0: number of satellites, time step and time limits
      ///    1: above plus flags, gap and interval values, and number of data/sat
      ///    2: above plus all the data tables
      virtual void dump(std::ostream& os = std::cout, short detail = 0)
         const throw()
      {
         // may have to re-write this...
         SP3Files.dump(os, detail);
         posStore.dump(os, detail);
         if(!useSP3clock) clkFiles.dump(os, detail);
         clkStore.dump(os, detail);
      }

      /// Edit the dataset, removing data outside the indicated time interval
      /// @param[in] tmin defines the beginning of the time interval
      /// @param[in] tmax defines the end of the time interval
      virtual void edit(const CommonTime& tmin, 
                        const CommonTime& tmax = CommonTime::END_OF_TIME) throw()
      {
         posStore.edit(tmin, tmax);
         clkStore.edit(tmin, tmax);
      }

      /// Clear the dataset, meaning remove all data
      virtual void clear(void) throw()
         { clearPosition(); clearClock(); }

      /// Determine the earliest time for which this object can successfully 
      /// determine the Xvt for any object.
      /// @return the earliest time in the table
      /// @throw InvalidRequest if the object has no data.
      virtual CommonTime getInitialTime() const throw(InvalidRequest);

      /// Determine the latest time for which this object can successfully 
      /// determine the Xvt for any object.
      /// @return the latest time in the table
      /// @throw InvalidRequest if the object has no data.
      virtual CommonTime getFinalTime() const throw(InvalidRequest);

      /// Return true if IndexType=SatID is present in the data tables
      virtual bool isPresent(const SatID& sat) const throw()
         { return (posStore.isPresent(sat) && clkStore.isPresent(sat)); }

      /// Return true if velocity is present in the data tables
      virtual bool hasVelocity() const throw()
         {  return posStore.hasVelocity(); }

   // end of XvtStore interface

      /// Dump information about the position store to an ostream.
      /// @param[in] os ostream to receive the output; defaults to std::cout
      /// @param[in] detail integer level of detail to provide; allowed values are
      ///    0: number of satellites, time step and time limits
      ///    1: above plus flags, gap and interval values, and number of data/sat
      ///    2: above plus all the data tables
      void dumpPosition(std::ostream& os = std::cout, short detail = 0)
         const throw()
      {
         SP3Files.dump(os, detail);
         posStore.dump(os, detail);
      }

      /// Dump information about the clock store to an ostream.
      /// @param[in] os ostream to receive the output; defaults to std::cout
      /// @param[in] detail integer level of detail to provide; allowed values are
      ///    0: number of satellites, time step and time limits
      ///    1: above plus flags, gap and interval values, and number of data/sat
      ///    2: above plus all the data tables
      void dumpClock(std::ostream& os = std::cout, short detail = 0)
         const throw()
      {
         if(useSP3clock)
            SP3Files.dump(os, detail);
         else
            clkFiles.dump(os, detail);
         clkStore.dump(os, detail);
      }

      /// Return the position for the given satellite at the given time
      /// @param[in] sat the SatID of the satellite of interest
      /// @param[in] ttag the time (CommonTime) of interest
      /// @return Triple containing the position ECEF XYZ meters
      /// @throw InvalidRequest if result cannot be computed, for example because
      ///  a) the time t does not lie within the time limits of the data table
      ///  b) checkDataGap is true and there is a data gap
      ///  c) checkInterval is true and the interval is larger than maxInterval
      Triple getPosition(const SatID sat, const CommonTime ttag)
         const throw(InvalidRequest);

      /// Return the velocity for the given satellite at the given time
      /// @param[in] sat the SatID of the satellite of interest
      /// @param[in] ttag the time (CommonTime) of interest
      /// @return Triple containing the velocity ECEF XYZ meters/second
      /// @throw InvalidRequest if result cannot be computed, for example because
      ///  a) the time t does not lie within the time limits of the data table
      ///  b) checkDataGap is true and there is a data gap
      ///  c) checkInterval is true and the interval is larger than maxInterval
      Triple getVelocity(const SatID sat, const CommonTime ttag)
         const throw(InvalidRequest);

      /// Return the acceleration for the given satellite at the given time
      /// @param[in] sat the SatID of the satellite of interest
      /// @param[in] ttag the time (CommonTime) of interest
      /// @return Triple containing the acceleration ECEF XYZ meters/second/second
      /// @throw InvalidRequest if result cannot be computed, for example because
      ///  a) the time t does not lie within the time limits of the data table
      ///  b) checkDataGap is true and there is a data gap
      ///  c) checkInterval is true and the interval is larger than maxInterval
      Triple getAcceleration(const SatID sat, const CommonTime ttag)
         const throw(InvalidRequest)
         { return posStore.getAcceleration(sat,ttag); }


      /// Clear the position dataset only, meaning remove all data from the tables.
      virtual void clearPosition(void) throw()
         { posStore.clear(); }

      /// Clear the clock dataset only, meaning remove all data from the tables.
      virtual void clearClock(void) throw()
         { clkStore.clear(); }

   
      /// Choose to load the clock data tables from RINEX clock files. This will
      /// clear the clock store; loadFile() or loadRinexClockFile() should be called
      /// after this, to load data into the clock store from RINEX clock files.
      /// This routine has no effect if the clock store is already set to RINEX clock.
      /// NB will be called by loadRinexClockFile() if clock store is set to SP3.
      void useRinexClockData(void) throw()
      {
         if(!useSP3clock) return;
         useSP3clock = false;
         clearClock();
      }

      /// Choose to load the clock data tables from SP3 files (this is the default).
      /// This will clear the clock store; if the position store has already been
      /// loaded it should also be cleared. The routines loadFile() or loadSP3File()
      /// should be called after this, to load data into the clock store from SP3
      /// files; NB this will also load position data into the position store.
      /// This routine has no effect if the clock store is already set to SP3.
      void useSP3ClockData(void) throw()
      {
         if(useSP3clock) return;
         useSP3clock = true;
         clearClock();
      }

      /// Get the earliest time of data in the position store.
      /// @return CommonTime the first time
      /// @throw InvalidRequest if there is no data
      CommonTime getPositionInitialTime(void) const throw(InvalidRequest)
         { return posStore.getInitialTime(); }

      /// Get the latest time of data in the position store.
      /// @return CommonTime the latest time
      /// @throw InvalidRequest if there is no data
      CommonTime getPositionFinalTime(void) const throw(InvalidRequest)
         { return posStore.getFinalTime(); }

      /// Get the earliest time of data in the clock store.
      /// @return CommonTime the first time
      /// @throw InvalidRequest if there is no data
      CommonTime getClockInitialTime(void) const throw(InvalidRequest)
         { return clkStore.getInitialTime(); }

      /// Get the latest time of data in the clock store.
      /// @return CommonTime the latest time
      /// @throw InvalidRequest if there is no data
      CommonTime getClockFinalTime(void) const throw(InvalidRequest)
         { return clkStore.getFinalTime(); }

      /// Get the earliest time of data in the position store for the given satellite.
      /// @return CommonTime the first time
      /// @throw InvalidRequest if there is no data
      CommonTime getPositionInitialTime(const SatID& sat) const throw(InvalidRequest)
         { return posStore.getInitialTime(sat); }

      /// Get the latest time of data in the position store for the given satellite.
      /// @return CommonTime the latest time
      /// @throw InvalidRequest if there is no data
      CommonTime getPositionFinalTime(const SatID& sat) const throw(InvalidRequest)
         { return posStore.getFinalTime(sat); }

      /// Get the earliest time of data in the clock store for the given satellite.
      /// @return CommonTime the first time
      /// @throw InvalidRequest if there is no data
      CommonTime getClockInitialTime(const SatID& sat) const throw(InvalidRequest)
         { return clkStore.getInitialTime(sat); }

      /// Get the latest time of data in the clock store for the given satellite.
      /// @return CommonTime the latest time
      /// @throw InvalidRequest if there is no data
      CommonTime getClockFinalTime(const SatID& sat) const throw(InvalidRequest)
         { return clkStore.getFinalTime(sat); }

      /// Get the earliest time of both clock and position data in the store for the
      /// given satellite.
      /// @return CommonTime the first time
      /// @throw InvalidRequest if there is no data
      CommonTime getInitialTime(const SatID& sat) const throw(InvalidRequest);

      /// Get the latest time of both clock and position data in the store for the
      /// given satellite.
      /// @return the latest time
      /// @throw InvalidRequest if there is no data
      CommonTime getFinalTime(const SatID& sat) const throw(InvalidRequest);


      /// Get the nominal time step in seconds for the (position) data.
      /// NB. if the clock store uses RINEX clock data, use getPositionTimeStep()
      /// and getClockTimeStep() instead.
      double getTimeStep(void) const throw()
         { return posStore.getTimeStep(); }

      /// Set the nominal time step in seconds. NB. if the clock store uses RINEX
      /// clock data, use setPositionTimeStep() and setClockTimeStep() instead.
      void setTimeStep(double dt) throw()
         { posStore.setTimeStep(dt); clkStore.setTimeStep(dt); }

      /// Get the nominal time step in seconds for the position data.
      double getPositionTimeStep(void) const throw()
         { return posStore.getTimeStep(); }

      /// Set the nominal time step in seconds for the position data.
      /// NB. if the clock store uses SP3 data, setTimeStep() may be used to set the
      /// time step for both position and clock.
      void setPositionTimeStep(double dt) throw()
         { posStore.setTimeStep(dt); }

      /// Get the nominal time step in seconds for the clock data.
      double getClockTimeStep(void) const throw()
         { return clkStore.getTimeStep(); }

      /// Set the nominal time step in seconds for the clock data.
      /// NB. if the clock store uses SP3 data, setTimeStep() may be used to set the
      /// time step for both position and clock.
      void setClockTimeStep(double dt) throw()
         { clkStore.setTimeStep(dt); }


      /// Get current interpolation order for the position table
      unsigned int getPositionInterpOrder(void) throw()
         { return posStore.getInterpolationOrder(); }

      /// Set the interpolation order for the position table; it is forced to be even.
      void setPositionInterpOrder(unsigned int order) throw()
         { posStore.setInterpolationOrder(order); }

      /// Get current interpolation order for the clock data (meaningless if the
      /// interpolation type is linear).
      unsigned int getClockInterpOrder(void) throw()
         { return clkStore.getInterpolationOrder(); }

      /// Set the interpolation order for the clock table; it is forced to be even.
      /// This is ignored if the clock interpolation type is linear.
      void setClockInterpOrder(unsigned int order) throw()
         { clkStore.setInterpolationOrder(order); }

      /// Set the type of clock interpolation to Lagrange (the default); set the order
      /// of the interpolation with setClockInterpolationOrder(order);
      void setClockLagrangeInterp(void) throw()
         { clkStore.setLagrangeInterp(); }

      /// Set the type of clock interpolation to linear
      /// (interpolation order is ignored).
      void setClockLinearInterp(void) throw()
         { clkStore.setLinearInterp(); }


      /// Get a list (std::vector) of SatIDs present in the store
      std::vector<SatID> getSatList(void) const throw()
      {
         std::vector<SatID> posList(posStore.getSatList());
         std::vector<SatID> clkList(clkStore.getSatList());
         std::vector<SatID> retList;
         for(int i=0; i<posList.size(); i++)
            if(find(clkList.begin(),clkList.end(),posList[i]) != clkList.end())
               retList.push_back(posList[i]);
         return retList;
      }


      /// Get the total number of (position) data records in the store
      inline int ndata(void) const throw()
         { return posStore.ndata(); }

      /// Get the number of (position) data records for the given sat
      inline int ndata(const SatID& sat) const throw()
         { return posStore.ndata(sat); }

      /// Get the total number of position data records in the store
      inline int ndataPosition(void) const throw()
         { return posStore.ndata(); }

      /// Get the number of position data records for the given sat
      inline int ndataPosition(const SatID& sat) const throw()
         { return posStore.ndata(sat); }

      /// Get the total number of clock data records in the store
      inline int ndataClock(void) const throw()
         { return clkStore.ndata(); }

      /// Get the number of position data records for the given sat
      inline int ndataClock(const SatID& sat) const throw()
         { return clkStore.ndata(sat); }

      /// same as ndataPosition()
      inline int size(void) const throw() { return ndataPosition(); }


      /// Load an SP3 ephemeris file; if the clock store uses RINEX clock files,
      /// this routine will also accept that file type and load the data into the
      /// clock store. This routine will may set the velocity, acceleration, bias
      /// or drift 'have' flags.
      void loadFile(const std::string& filename) throw(Exception);

      /// Load an SP3 ephemeris file; may set the velocity and acceleration flags.
      /// If the clock store uses RINEX clock data, this will ignore the clock data.
      void loadSP3File(const std::string& filename) throw(Exception);

      /// Load a RINEX clock file; may set the 'have' bias and drift flags.
      /// If clock store is set to use SP3 data, this will call useRinexClockData()
      void loadRinexClockFile(const std::string& filename) throw(Exception);


      /// Add a complete PositionRecord to the store; this is the preferred method
      /// of adding data to the tables.
      /// NB. If these addXXX() routines are used more than once for the same record
      /// (sat,ttag), be aware that since ttag is used as they key in a std::map,
      /// the value used must be EXACTLY the same in all calls; (numerical noise could
      /// cause the std::map to consider two "equal" ttags as different).
      void addPositionRecord(const SatID& sat, const CommonTime& ttag,
                        const PositionRecord& data) throw()
      { posStore.addPositionRecord(sat,ttag,data); }

      /// Add position data to the store
      void addPositionData(const SatID& sat, const CommonTime& ttag,
                        const Triple& Pos, const Triple& sig) throw()
      { posStore.addPositionData(sat,ttag,Pos,sig); }

      /// Add velocity data to the store
      void addVelocityData(const SatID& sat, const CommonTime& ttag,
                        const Triple& Vel, const Triple& sig) throw()
      { posStore.addVelocityData(sat,ttag,Vel,sig); }

      /// Add a complete ClockRecord to the store; this is the preferred method
      /// of adding data to the tables.
      /// NB. If these addXXX() routines are used more than once for the same record
      /// (sat,ttag), be aware that since ttag is used as they key in a std::map,
      /// the value used must be EXACTLY the same in all calls; (numerical noise could
      /// cause the std::map to consider two "equal" ttags as different).
      void addClockRecord(const SatID& sat, const CommonTime& ttag,
                          const ClockRecord& rec) throw()
      { clkStore.addClockRecord(sat,ttag,rec); }

      /// Add clock bias data (only) to the store
      void addClockBias(const SatID& sat, const CommonTime& ttag,
                        const double& bias, const double& sig=0.0) throw()
      { clkStore.addClockBias(sat,ttag,bias,sig); }

      /// Add clock drift data (only) to the store
      void addClockDrift(const SatID& sat, const CommonTime& ttag,
                        const double& drift, const double& sig=0.0) throw()
      { clkStore.addClockDrift(sat,ttag,drift,sig); }

      /// Add clock acceleration data (only) to the store
      void addClockAcceleration(const SatID& sat, const CommonTime& ttag,
                        const double& accel, const double& sig=0.0) throw()
      { clkStore.addClockAcceleration(sat,ttag,accel,sig); }


      /// Get number of files (all types) in FileStore.
      int nfiles(void) throw()
         { return (SP3Files.size() + (useSP3clock ? 0 : clkFiles.size())); }

      /// Get number of SP3 files in FileStore.
      int nSP3files(void) throw()
         { return SP3Files.size(); }

      /// Get number of clock files in FileStore.
      int nClockfiles(void) throw()
         { return (useSP3clock ? SP3Files.size() : clkFiles.size()); }


      /// Return true if there is drift data in the tables
      virtual bool hasClockDrift() const throw()
         {  return clkStore.hasClockDrift(); }


      /// Set the flag; if true then bad position values are rejected when
      /// adding data to the store.
      void rejectBadPositions(const bool flag)
         { rejectBadPosFlag = flag; }

      /// Set the flag; if true then bad position values are rejected when
      /// adding data to the store.
      void rejectBadClocks(const bool flag)
         { rejectBadClockFlag = flag; }


      /// Disable checking of data gaps in both position and clock.
      void disableDataGapCheck(void) throw()
         { posStore.disableDataGapCheck(); clkStore.disableDataGapCheck(); }

      /// Disable checking of data gaps in position store
      void disablePosDataGapCheck(void) throw()
         { posStore.disableDataGapCheck(); }

      /// Disable checking of data gaps in clock store
      void disableClockDataGapCheck(void) throw()
         { clkStore.disableDataGapCheck(); }

      /// Get current gap interval in the position store.
      double getPosGapInterval(void) throw()
         { return posStore.getGapInterval(); }

      /// Get current gap interval in the clock store.
      double getClockGapInterval(void) throw()
         { return clkStore.getGapInterval(); }

      /// Set gap interval and turn on gap checking in the position store;
      /// default is 901 sec.
      void setPosGapInterval(double interval) throw()
         { posStore.setGapInterval(interval); }

      /// Set gap interval and turn on gap checking in the clock store; default is 30
      void setClockGapInterval(double interval) throw()
         { clkStore.setGapInterval(interval); }

      /// Disable checking of maximum interval in both position and clock.
      void disableIntervalCheck(void) throw()
         { posStore.disableIntervalCheck(); clkStore.disableIntervalCheck(); }

      /// Disable checking of maximum interval in position store
      void disablePosIntervalCheck(void) throw()
         { posStore.disableIntervalCheck(); }

      /// Disable checking of maximum interval in clock store
      void disableClockIntervalCheck(void) throw()
         { clkStore.disableIntervalCheck(); }

      /// Get current maximum interval in the position store
      double getPosMaxInterval(void) throw()
         { return posStore.getMaxInterval(); }

      /// Get current maximum interval in the clock store
      double getClockMaxInterval(void) throw()
         { return clkStore.getMaxInterval(); }

      /// Set maximum interval and turn on interval checking in the position store
      /// Default is 8101 sec.
      void setPosMaxInterval(double interval) throw()
         { posStore.setMaxInterval(interval); }

      /// Set maximum interval and turn on interval checking in the clock store
      /// Default is 60 sec.
      void setClockMaxInterval(double interval) throw()
         { clkStore.setMaxInterval(interval); }


      /// \deprecated
      virtual bool velocityIsPresent() const throw()
         {  return posStore.hasVelocity(); }

      /// \deprecated
      virtual bool clockIsPresent() const throw()
         {  return true; }

      /// \deprecated
      /// Get current (position) interpolation order
      unsigned int getInterpolationOrder(void) throw()
         { return getPositionInterpOrder(); }

      /// \deprecated
      /// Set the (position) interpolation order for the position table;
      /// it is forced to be even.
      void setInterpolationOrder(unsigned int order) throw()
         { setPositionInterpOrder(order); }

   }; // end class SP3EphemerisStore

      //@}

}  // End of namespace gpstk

#endif // GPSTK_SP3_EPHEMERIS_STORE_INCLUDE
