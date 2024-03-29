/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "Time.H"
#include "PstreamReduceOps.H"
#include "argList.H"

#include <sstream>

// * * * * * * * * * * * * * Static Member Data  * * * * * * * * * * * * * * //

defineTypeNameAndDebug(Foam::Time, 0);

namespace Foam
{
    template<>
    const char* Foam::NamedEnum
    <
        Foam::Time::stopAtControls,
        4
    >::names[] =
    {
        "endTime",
        "noWriteNow",
        "writeNow",
        "nextWrite"
    };

    template<>
    const char* Foam::NamedEnum
    <
        Foam::Time::writeControls,
        5
    >::names[] =
    {
        "timeStep",
        "runTime",
        "adjustableRunTime",
        "clockTime",
        "cpuTime"
    };
}

const Foam::NamedEnum<Foam::Time::stopAtControls, 4>
    Foam::Time::stopAtControlNames_;

const Foam::NamedEnum<Foam::Time::writeControls, 5>
    Foam::Time::writeControlNames_;

Foam::Time::fmtflags Foam::Time::format_(Foam::Time::general);
int Foam::Time::precision_(12);//changed by xiaow

Foam::word Foam::Time::controlDictName("controlDict");

//add by RXG: begin
Foam::CommProfiler Foam::Time::commProfiler_ = CommProfiler();
bool Foam::Time::isInSubTime_ = 0;
Foam::fileName Foam::Time::profilerPath_="";


//add by RXG: end

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::Time::adjustDeltaT()
{
    if (writeControl_ == wcAdjustableRunTime)
    {
        scalar interval = writeInterval_;
        if (secondaryWriteControl_ == wcAdjustableRunTime)
        {
            interval = min(interval, secondaryWriteInterval_);
        }


        scalar timeToNextWrite = max
        (
            0.0,
            (outputTimeIndex_ + 1)*interval - (value() - startTime_)
        );

        scalar nSteps = timeToNextWrite/deltaT_ - SMALL;

        // For tiny deltaT the label can overflow!
        if (nSteps < labelMax)
        {
            label nStepsToNextWrite = label(nSteps) + 1;

            scalar newDeltaT = timeToNextWrite/nStepsToNextWrite;

            // Control the increase of the time step to within a factor of 2
            // and the decrease within a factor of 5.
            if (newDeltaT >= deltaT_)
            {
                deltaT_ = min(newDeltaT, 2.0*deltaT_);
            }
            else
            {
                deltaT_ = max(newDeltaT, 0.2*deltaT_);
            }
        }
    }
}


void Foam::Time::setControls()
{
    // default is to resume calculation from "latestTime"
    const word startFrom = controlDict_.lookupOrDefault<word>
    (
        "startFrom",
        "latestTime"
    );

    if (startFrom == "startTime")
    {
        controlDict_.lookup("startTime") >> startTime_;
    }
    else
    {
        // Search directory for valid time directories
        instantList timeDirs = findTimes(path());

        if (startFrom == "firstTime")
        {
            if (timeDirs.size())
            {
                if (timeDirs[0].name() == constant() && timeDirs.size() >= 2)
                {
                    startTime_ = timeDirs[1].value();
                }
                else
                {
                    startTime_ = timeDirs[0].value();
                }
            }
        }
        else if (startFrom == "latestTime")
        {
            if (timeDirs.size())
            {
                startTime_ = timeDirs.last().value();
            }
        }
        else
        {
            FatalIOErrorIn("Time::setControls()", controlDict_)
                << "expected startTime, firstTime or latestTime"
                << " found '" << startFrom << "'"
                << exit(FatalIOError);
        }
    }

    setTime(startTime_, 0);

    readDict();
    deltaTSave_ = deltaT_;
    deltaT0_ = deltaT_;

    if (Pstream::parRun())
    {
        scalar sumStartTime = startTime_;
        reduce(sumStartTime, sumOp<scalar>());
        if
        (
            mag(Pstream::nProcs()*startTime_ - sumStartTime)
          > Pstream::nProcs()*deltaT_/10.0
        )
        {
            FatalIOErrorIn("Time::setControls()", controlDict_)
                << "Start time is not the same for all processors" << nl
                << "processor " << Pstream::myProcNo() << " has startTime "
                << startTime_ << exit(FatalIOError);
        }
    }

    IOdictionary timeDict
    (
        IOobject
        (
            "time",
            timeName(),
            "uniform",
            *this,
            IOobject::READ_IF_PRESENT,
            IOobject::NO_WRITE,
            false
        )
    );

    if (timeDict.readIfPresent("deltaT", deltaT_))
    {
        deltaTSave_ = deltaT_;
        deltaT0_ = deltaT_;
    }

    timeDict.readIfPresent("deltaT0", deltaT0_);

    if (timeDict.readIfPresent("index", startTimeIndex_))
    {
        timeIndex_ = startTimeIndex_;
    }

    scalar timeValue;
    if (timeDict.readIfPresent("value", timeValue))
    {
        word storedTimeName(timeName(timeValue));

        if (storedTimeName != timeName())
        {
            IOWarningIn("Time::setControls()", timeDict)
                << "Time read from time dictionary " << storedTimeName
                << " differs from actual time " << timeName() << '.' << nl
                << "    This may cause unexpected database behaviour."
                << " If you are not interested" << nl
                << "    in preserving time state delete the time dictionary."
                << endl;
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::Time::Time
(
    const word& controlDictName,
    const fileName& rootPath,
    const fileName& caseName,
    const word& systemName,
    const word& constantName,
    const bool enableFunctionObjects
)
:
    TimePaths
    (
        rootPath,
        caseName,
        systemName,
        constantName
    ),

    objectRegistry(*this),

    libs_(),

    controlDict_
    (
        IOobject
        (
            controlDictName,
            system(),
            *this,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE,
            false
        )
    ),

    startTimeIndex_(0),
    startTime_(0),
    endTime_(0),

    stopAt_(saEndTime),
    writeControl_(wcTimeStep),
    writeInterval_(GREAT),
    secondaryWriteControl_(wcTimeStep),
    secondaryWriteInterval_(labelMax/10.0), // bit less to allow calculations
    purgeWrite_(0),
    writeOnce_(false),
    subCycling_(false),
    sigWriteNow_(true, *this),
    sigStopAtWriteNow_(true, *this),

    writeFormat_(IOstream::ASCII),
    writeVersion_(IOstream::currentVersion),
    writeCompression_(IOstream::UNCOMPRESSED),
    graphFormat_("raw"),
    runTimeModifiable_(true),

    functionObjects_(*this, enableFunctionObjects)
{
    libs_.open(controlDict_, "libs");

	//add by RXG: begin
    profilerPath_=path()/"CommProfiling";
	mkDir(profilerPath_);
	//add by RXG: end

    // Explicitly set read flags on objectRegistry so anything constructed
    // from it reads as well (e.g. fvSolution).
    readOpt() = IOobject::MUST_READ_IF_MODIFIED;

    setControls();

    // Time objects not registered so do like objectRegistry::checkIn ourselves.
    if (runTimeModifiable_)
    {
        monitorPtr_.reset
        (
            new fileMonitor
            (
                regIOobject::fileModificationChecking == inotify
             || regIOobject::fileModificationChecking == inotifyMaster
            )
        );

        // File might not exist yet.
        fileName f(controlDict_.filePath());

        if (!f.size())
        {
            // We don't have this file but would like to re-read it.
            // Possibly if in master-only reading mode. Use a non-existing
            // file to keep fileMonitor synced.
            f = controlDict_.objectPath();
        }

        controlDict_.watchIndex() = addWatch(f);
    }
}


Foam::Time::Time
(
    const word& controlDictName,
    const argList& args,
    const word& systemName,
    const word& constantName
)
:
    TimePaths
    (
        args.rootPath(),
        args.caseName(),
        systemName,
        constantName
    ),

    objectRegistry(*this),

    libs_(),

    controlDict_
    (
        IOobject
        (
            controlDictName,
            system(),
            *this,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE,
            false
        )
    ),

    startTimeIndex_(0),
    startTime_(0),
    endTime_(0),

    stopAt_(saEndTime),
    writeControl_(wcTimeStep),
    writeInterval_(GREAT),
    secondaryWriteControl_(wcTimeStep),
    secondaryWriteInterval_(labelMax/10.0),
    purgeWrite_(0),
    writeOnce_(false),
    subCycling_(false),
    sigWriteNow_(true, *this),
    sigStopAtWriteNow_(true, *this),

    writeFormat_(IOstream::ASCII),
    writeVersion_(IOstream::currentVersion),
    writeCompression_(IOstream::UNCOMPRESSED),
    graphFormat_("raw"),
    runTimeModifiable_(true),

    functionObjects_(*this, !args.optionFound("noFunctionObjects"))
{
    libs_.open(controlDict_, "libs");
    //add by RXG: begin
    profilerPath_=path()/"CommProfiling";
	mkDir(profilerPath_);
	//add by RXG: end

    // Explicitly set read flags on objectRegistry so anything constructed
    // from it reads as well (e.g. fvSolution).
    readOpt() = IOobject::MUST_READ_IF_MODIFIED;

    setControls();

    // Time objects not registered so do like objectRegistry::checkIn ourselves.
    if (runTimeModifiable_)
    {
        monitorPtr_.reset
        (
            new fileMonitor
            (
                regIOobject::fileModificationChecking == inotify
             || regIOobject::fileModificationChecking == inotifyMaster
            )
        );

        // File might not exist yet.
        fileName f(controlDict_.filePath());

        if (!f.size())
        {
            // We don't have this file but would like to re-read it.
            // Possibly if in master-only reading mode. Use a non-existing
            // file to keep fileMonitor synced.
            f = controlDict_.objectPath();
        }

        controlDict_.watchIndex() = addWatch(f);
    }
}


Foam::Time::Time
(
    const dictionary& dict,
    const fileName& rootPath,
    const fileName& caseName,
    const word& systemName,
    const word& constantName,
    const bool enableFunctionObjects
)
:
    TimePaths
    (
        rootPath,
        caseName,
        systemName,
        constantName
    ),

    objectRegistry(*this),

    libs_(),

    controlDict_
    (
        IOobject
        (
            controlDictName,
            system(),
            *this,
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        ),
        dict
    ),

    startTimeIndex_(0),
    startTime_(0),
    endTime_(0),

    stopAt_(saEndTime),
    writeControl_(wcTimeStep),
    writeInterval_(GREAT),
    secondaryWriteControl_(wcTimeStep),
    secondaryWriteInterval_(labelMax/10.0),
    purgeWrite_(0),
    writeOnce_(false),
    subCycling_(false),
    sigWriteNow_(true, *this),
    sigStopAtWriteNow_(true, *this),

    writeFormat_(IOstream::ASCII),
    writeVersion_(IOstream::currentVersion),
    writeCompression_(IOstream::UNCOMPRESSED),
    graphFormat_("raw"),
    runTimeModifiable_(true),

    functionObjects_(*this, enableFunctionObjects)
{
    libs_.open(controlDict_, "libs");

	//add by RXG: begin
    profilerPath_=path()/"CommProfiling";
	mkDir(profilerPath_);
	//add by RXG: end

    // Explicitly set read flags on objectRegistry so anything constructed
    // from it reads as well (e.g. fvSolution).
    readOpt() = IOobject::MUST_READ_IF_MODIFIED;

    // Since could not construct regIOobject with setting:
    controlDict_.readOpt() = IOobject::MUST_READ_IF_MODIFIED;

    setControls();

    // Time objects not registered so do like objectRegistry::checkIn ourselves.
    if (runTimeModifiable_)
    {
        monitorPtr_.reset
        (
            new fileMonitor
            (
                regIOobject::fileModificationChecking == inotify
             || regIOobject::fileModificationChecking == inotifyMaster
            )
        );

        // File might not exist yet.
        fileName f(controlDict_.filePath());

        if (!f.size())
        {
            // We don't have this file but would like to re-read it.
            // Possibly if in master-only reading mode. Use a non-existing
            // file to keep fileMonitor synced.
            f = controlDict_.objectPath();
        }

        controlDict_.watchIndex() = addWatch(f);
    }
}


Foam::Time::Time
(
    const fileName& rootPath,
    const fileName& caseName,
    const word& systemName,
    const word& constantName,
    const bool enableFunctionObjects
)
:
    TimePaths
    (
        rootPath,
        caseName,
        systemName,
        constantName
    ),

    objectRegistry(*this),

    libs_(),

    controlDict_
    (
        IOobject
        (
            controlDictName,
            system(),
            *this,
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        )
    ),

    startTimeIndex_(0),
    startTime_(0),
    endTime_(0),

    stopAt_(saEndTime),
    writeControl_(wcTimeStep),
    writeInterval_(GREAT),
    secondaryWriteControl_(wcTimeStep),
    secondaryWriteInterval_(labelMax/10.0),
    purgeWrite_(0),
    writeOnce_(false),
    subCycling_(false),

    writeFormat_(IOstream::ASCII),
    writeVersion_(IOstream::currentVersion),
    writeCompression_(IOstream::UNCOMPRESSED),
    graphFormat_("raw"),
    runTimeModifiable_(true),

    functionObjects_(*this, enableFunctionObjects)
{
    libs_.open(controlDict_, "libs");
	//add by RXG: begin
    profilerPath_=path()/"CommProfiling";
	mkDir(profilerPath_);
	//add by RXG: end
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::Time::~Time()
{
    if (controlDict_.watchIndex() != -1)
    {
        removeWatch(controlDict_.watchIndex());
    }

    // destroy function objects first
    functionObjects_.clear();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::label Foam::Time::addWatch(const fileName& fName) const
{
    return monitorPtr_().addWatch(fName);
}


bool Foam::Time::removeWatch(const label watchIndex) const
{
    return monitorPtr_().removeWatch(watchIndex);
}

const Foam::fileName& Foam::Time::getFile(const label watchIndex) const
{
    return monitorPtr_().getFile(watchIndex);
}


Foam::fileMonitor::fileState Foam::Time::getState
(
    const label watchFd
) const
{
    return monitorPtr_().getState(watchFd);
}


void Foam::Time::setUnmodified(const label watchFd) const
{
    monitorPtr_().setUnmodified(watchFd);
}


Foam::word Foam::Time::timeName(const scalar t)
{
    std::ostringstream buf;
    buf.setf(ios_base::fmtflags(format_), ios_base::floatfield);
    buf.precision(precision_);
    buf << t;
    return buf.str();
}


Foam::word Foam::Time::timeName() const
{
    return dimensionedScalar::name();
}


// Search the construction path for times
Foam::instantList Foam::Time::times() const
{
    return findTimes(path());
}


Foam::word Foam::Time::findInstancePath(const instant& t) const
{
    instantList timeDirs = findTimes(path());

    forAllReverse(timeDirs, timeI)
    {
        if (timeDirs[timeI] == t)
        {
            return timeDirs[timeI].name();
        }
    }

    return word::null;
}


Foam::instant Foam::Time::findClosestTime(const scalar t) const
{
    instantList timeDirs = findTimes(path());

    // there is only one time (likely "constant") so return it
    if (timeDirs.size() == 1)
    {
        return timeDirs[0];
    }

    if (t < timeDirs[1].value())
    {
        return timeDirs[1];
    }
    else if (t > timeDirs.last().value())
    {
        return timeDirs.last();
    }

    label nearestIndex = -1;
    scalar deltaT = GREAT;

    for (label timeI=1; timeI < timeDirs.size(); ++timeI)
    {
        scalar diff = mag(timeDirs[timeI].value() - t);
        if (diff < deltaT)
        {
            deltaT = diff;
            nearestIndex = timeI;
        }
    }

    return timeDirs[nearestIndex];
}


// This should work too,
// if we don't worry about checking "constant" explicitly
//
// Foam::instant Foam::Time::findClosestTime(const scalar t) const
// {
//     instantList timeDirs = findTimes(path());
//     label timeIndex = min(findClosestTimeIndex(timeDirs, t), 0);
//     return timeDirs[timeIndex];
// }

Foam::label Foam::Time::findClosestTimeIndex
(
    const instantList& timeDirs,
    const scalar t
)
{
    label nearestIndex = -1;
    scalar deltaT = GREAT;

    forAll(timeDirs, timeI)
    {
        if (timeDirs[timeI].name() == "constant") continue;

        scalar diff = mag(timeDirs[timeI].value() - t);
        if (diff < deltaT)
        {
            deltaT = diff;
            nearestIndex = timeI;
        }
    }

    return nearestIndex;
}


Foam::label Foam::Time::startTimeIndex() const
{
    return startTimeIndex_;
}


Foam::dimensionedScalar Foam::Time::startTime() const
{
    return dimensionedScalar("startTime", dimTime, startTime_);
}


Foam::dimensionedScalar Foam::Time::endTime() const
{
    return dimensionedScalar("endTime", dimTime, endTime_);
}


bool Foam::Time::run() const
{
    bool running = value() < (endTime_ - 0.5*deltaT_);

    if (!subCycling_)
    {
        // only execute when the condition is no longer true
        // ie, when exiting the control loop
        if (!running && timeIndex_ != startTimeIndex_)
        {
            // Note, end() also calls an indirect start() as required
            functionObjects_.end();
        }
    }

    if (running)
    {
        if (!subCycling_)
        {
            const_cast<Time&>(*this).readModifiedObjects();

            if (timeIndex_ == startTimeIndex_)
            {
                functionObjects_.start();
            }
            else
            {
                functionObjects_.execute();
            }
        }

        // Update the "running" status following the
        // possible side-effects from functionObjects
        running = value() < (endTime_ - 0.5*deltaT_);
    }

    return running;
}


bool Foam::Time::loop()
{
    bool running = run();

    if (running)
    {
        operator++();
    }

    return running;
}


bool Foam::Time::end() const
{
    return value() > (endTime_ + 0.5*deltaT_);
}


bool Foam::Time::stopAt(const stopAtControls sa) const
{
    const bool changed = (stopAt_ != sa);
    stopAt_ = sa;

    // adjust endTime
    if (sa == saEndTime)
    {
        controlDict_.lookup("endTime") >> endTime_;
    }
    else
    {
        endTime_ = GREAT;
    }
    return changed;
}


void Foam::Time::setTime(const Time& t)
{
    value() = t.value();
    dimensionedScalar::name() = t.dimensionedScalar::name();
    timeIndex_ = t.timeIndex_;
}


void Foam::Time::setTime(const instant& inst, const label newIndex)
{
    value() = inst.value();
    dimensionedScalar::name() = inst.name();
    timeIndex_ = newIndex;

    IOdictionary timeDict
    (
        IOobject
        (
            "time",
            timeName(),
            "uniform",
            *this,
            IOobject::READ_IF_PRESENT,
            IOobject::NO_WRITE,
            false
        )
    );

    timeDict.readIfPresent("deltaT", deltaT_);
    timeDict.readIfPresent("deltaT0", deltaT0_);
    timeDict.readIfPresent("index", timeIndex_);
}


void Foam::Time::setTime(const dimensionedScalar& newTime, const label newIndex)
{
    setTime(newTime.value(), newIndex);
}


void Foam::Time::setTime(const scalar newTime, const label newIndex)
{
    value() = newTime;
    dimensionedScalar::name() = timeName(timeToUserTime(newTime));
    timeIndex_ = newIndex;
}


void Foam::Time::setEndTime(const dimensionedScalar& endTime)
{
    setEndTime(endTime.value());
}


void Foam::Time::setEndTime(const scalar endTime)
{
    endTime_ = endTime;
}


void Foam::Time::setDeltaT(const dimensionedScalar& deltaT)
{
    setDeltaT(deltaT.value());
}


void Foam::Time::setDeltaT(const scalar deltaT)
{
    deltaT_ = deltaT;
    deltaTchanged_ = true;
    adjustDeltaT();
}


Foam::TimeState Foam::Time::subCycle(const label nSubCycles)
{
    subCycling_ = true;
    prevTimeState_.set(new TimeState(*this));

    setTime(*this - deltaT(), (timeIndex() - 1)*nSubCycles);
    deltaT_ /= nSubCycles;
    deltaT0_ /= nSubCycles;
    deltaTSave_ = deltaT0_;

    return prevTimeState();
}


void Foam::Time::endSubCycle()
{
    if (subCycling_)
    {
        subCycling_ = false;
        TimeState::operator=(prevTimeState());
        prevTimeState_.clear();
    }
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

Foam::Time& Foam::Time::operator+=(const dimensionedScalar& deltaT)
{
    return operator+=(deltaT.value());
}


Foam::Time& Foam::Time::operator+=(const scalar deltaT)
{
    setDeltaT(deltaT);
    return operator++();
}


Foam::Time& Foam::Time::operator++()
{
    deltaT0_ = deltaTSave_;
    deltaTSave_ = deltaT_;

    // Save old time name
    const word oldTimeName = dimensionedScalar::name();

    setTime(value() + deltaT_, timeIndex_ + 1);

    if (!subCycling_)
    {
        // If the time is very close to zero reset to zero
        if (mag(value()) < 10*SMALL*deltaT_)
        {
            setTime(0.0, timeIndex_);
        }
    }


    // Check that new time representation differs from old one
    if (dimensionedScalar::name() == oldTimeName)
    {
        int oldPrecision = precision_;
        do
        {
            precision_++;
            setTime(value(), timeIndex());
        }
        while (precision_ < 100 && dimensionedScalar::name() == oldTimeName);

        WarningIn("Time::operator++()")
            << "Increased the timePrecision from " << oldPrecision
            << " to " << precision_
            << " to distinguish between timeNames at time " << value()
            << endl;
    }


    if (!subCycling_)
    {
        if (sigStopAtWriteNow_.active() || sigWriteNow_.active())
        {
            // A signal might have been sent on one processor only
            // Reduce so all decide the same.

            label flag = 0;
            if (sigStopAtWriteNow_.active() && stopAt_ == saWriteNow)
            {
                flag += 1;
            }
            if (sigWriteNow_.active() && writeOnce_)
            {
                flag += 2;
            }
            reduce(flag, maxOp<label>());

            if (flag & 1)
            {
                stopAt_ = saWriteNow;
            }
            if (flag & 2)
            {
                writeOnce_ = true;
            }
        }


        outputTime_ = false;

        switch (writeControl_)
        {
            case wcTimeStep:
                outputTime_ = !(timeIndex_ % label(writeInterval_));
            break;

            case wcRunTime:
            case wcAdjustableRunTime:
            {
                label outputIndex = label
                (
                    ((value() - startTime_) + 0.5*deltaT_)
                  / writeInterval_
                );

                if (outputIndex > outputTimeIndex_)
                {
                    outputTime_ = true;
                    outputTimeIndex_ = outputIndex;
                }
            }
            break;

            case wcCpuTime:
            {
                label outputIndex = label
                (
                    returnReduce(elapsedCpuTime(), maxOp<double>())
                  / writeInterval_
                );
                if (outputIndex > outputTimeIndex_)
                {
                    outputTime_ = true;
                    outputTimeIndex_ = outputIndex;
                }
            }
            break;

            case wcClockTime:
            {
                label outputIndex = label
                (
                    returnReduce(label(elapsedClockTime()), maxOp<label>())
                  / writeInterval_
                );
                if (outputIndex > outputTimeIndex_)
                {
                    outputTime_ = true;
                    outputTimeIndex_ = outputIndex;
                }
            }
            break;
        }


        // Adapt for secondaryWrite controls
        switch (secondaryWriteControl_)
        {
            case wcTimeStep:
                outputTime_ =
                    outputTime_
                || !(timeIndex_ % label(secondaryWriteInterval_));
            break;

            case wcRunTime:
            case wcAdjustableRunTime:
            {
                label outputIndex = label
                (
                    ((value() - startTime_) + 0.5*deltaT_)
                  / secondaryWriteInterval_
                );

                if (outputIndex > outputTimeIndex_)
                {
                    outputTime_ = true;
                    outputTimeIndex_ = outputIndex;
                }
            }
            break;

            case wcCpuTime:
            {
                label outputIndex = label
                (
                    returnReduce(elapsedCpuTime(), maxOp<double>())
                  / secondaryWriteInterval_
                );
                if (outputIndex > outputTimeIndex_)
                {
                    outputTime_ = true;
                    outputTimeIndex_ = outputIndex;
                }
            }
            break;

            case wcClockTime:
            {
                label outputIndex = label
                (
                    returnReduce(label(elapsedClockTime()), maxOp<label>())
                  / secondaryWriteInterval_
                );
                if (outputIndex > outputTimeIndex_)
                {
                    outputTime_ = true;
                    outputTimeIndex_ = outputIndex;
                }
            }
            break;
        }


        // see if endTime needs adjustment to stop at the next run()/end() check
        if (!end())
        {
            if (stopAt_ == saNoWriteNow)
            {
                endTime_ = value();
            }
            else if (stopAt_ == saWriteNow)
            {
                endTime_ = value();
                outputTime_ = true;
            }
            else if (stopAt_ == saNextWrite && outputTime_ == true)
            {
                endTime_ = value();
            }
        }

        // Override outputTime if one-shot writing
        if (writeOnce_)
        {
            outputTime_ = true;
            writeOnce_ = false;
        }

    }
	//add by RXG: begin
    if(isInSubTime_==0)
    {
		Foam::Time::enterTimeStep(timeName());
		//Pout<< timeName()<<"******************************************************************"<<endl;
    }
	//add by RXG: end
	
    return *this;
}




Foam::Time& Foam::Time::operator++(int)
{
    return operator++();
}


// ************************************************************************* //
