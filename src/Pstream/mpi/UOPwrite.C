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

Description
    Write primitive and binary block from OPstream

\*---------------------------------------------------------------------------*/

#include "mpi.h"

#include "UOPstream.H"
#include "PstreamGlobals.H"

#include "cpuTime.H" // added by Howe

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::UOPstream::write
(
    const commsTypes commsType,
    const int toProcNo,
    const char* buf,
    const std::streamsize bufSize,
    const int tag
)
{
    if (debug)
    {
        Pout<< "UOPstream::write : starting write to:" << toProcNo
            << " tag:" << tag << " size:" << label(bufSize)
            << " commsType:" << UPstream::commsTypeNames[commsType]
            << Foam::endl;
    }

    bool transferFailed = true;
	cpuTime	timer; // added by Howe
	double tmpT=0; // added by Howe
//add by RXG: begin
	baseCommInfo* tempRecord = Foam::Time::commProfiler_.commRecord(myProcNo(),procID(toProcNo),bufSize, commsType);
//add by RXG: end
//	Foam::Time::commProfiler_.testTT(); // added by Howe
    if (commsType == blocking)
    {
	tmpT=timer.cpuTimeIncrement(); // added by Howe
	
        transferFailed = MPI_Bsend
        (
            const_cast<char*>(buf),
            bufSize,
            MPI_PACKED,
            procID(toProcNo),
            tag,
            MPI_COMM_WORLD
        );
	tmpT=timer.cpuTimeIncrement(); // added by Howe
	Foam::Time::commProfiler_.sendRecord(0,tmpT); // added by Howe

        if (debug)
        {
            Pout<< "UOPstream::write : finished write to:" << toProcNo
                << " tag:" << tag << " size:" << label(bufSize)
                << " commsType:" << UPstream::commsTypeNames[commsType]
                << Foam::endl;
        }
    }
    else if (commsType == scheduled)
    {
	tmpT=timer.cpuTimeIncrement(); // added by Howe
	
	transferFailed = MPI_Send
        (
            const_cast<char*>(buf),
            bufSize,
            MPI_PACKED,
            procID(toProcNo),
            tag,
            MPI_COMM_WORLD
        );
	
	tmpT=timer.cpuTimeIncrement(); // added by Howe
	Foam::Time::commProfiler_.sendRecord(1,tmpT); // added by Howe
	
	if (debug)
        {
            Pout<< "UOPstream::write : finished write to:" << toProcNo
                << " tag:" << tag << " size:" << label(bufSize)
                << " commsType:" << UPstream::commsTypeNames[commsType]
                << Foam::endl;
        }
    }
    else if (commsType == nonBlocking)
    {
        MPI_Request request;
	
	tmpT=timer.cpuTimeIncrement(); // added by Howe
        transferFailed = MPI_Isend
        (
            const_cast<char*>(buf),
            bufSize,
            MPI_PACKED,
            procID(toProcNo),
            tag,
            MPI_COMM_WORLD,
            &request
        );

	tmpT=timer.cpuTimeIncrement(); // added by Howe
	Foam::Time::commProfiler_.sendRecord(2,tmpT); // added by Howe	

        if (debug)
        {
            Pout<< "UOPstream::write : started write to:" << toProcNo
                << " tag:" << tag << " size:" << label(bufSize)
                << " commsType:" << UPstream::commsTypeNames[commsType]
                << " request:" << PstreamGlobals::outstandingRequests_.size()
                << Foam::endl;
        }

        PstreamGlobals::outstandingRequests_.append(request);
    }
    else
    {
        FatalErrorIn
        (
            "UOPstream::write"
            "(const int fromProcNo, char* buf, std::streamsize bufSize"
            ", const int)"
        )   << "Unsupported communications type "
            << UPstream::commsTypeNames[commsType]
            << Foam::abort(FatalError);
    }

//add by Xiaow:begin
	if(tempRecord!=NULL)
		tempRecord->setEndTime();
//add by Xiaow:end


    return !transferFailed;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// ************************************************************************* //
