/*+--------------------------------------------------------------------------*\
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
    Exchange data.

\*---------------------------------------------------------------------------*/

#include "Pstream.H"
#include "contiguous.H"
#include "PstreamCombineReduceOps.H"
#include "UPstream.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

//template <template<class> class ListType, class T>
template <class Container, class T>
void Pstream::exchange
(
    const List<Container>& sendBufs,
    List<Container>& recvBufs,
    labelListList& sizes,
    const int tag,
    const bool block
)
{
    if (!contiguous<T>())
    {
        FatalErrorIn
        (
            "Pstream::exchange(..)"
        )   << "Continuous data only." << Foam::abort(FatalError);
    }

    if (sendBufs.size() != UPstream::nProcs())
    {
        FatalErrorIn
        (
            "Pstream::exchange(..)"
        )   << "Size of list:" << sendBufs.size()
            << " does not equal the number of processors:"
            << UPstream::nProcs()
            << Foam::abort(FatalError);
    }

    sizes.setSize(UPstream::nProcs());
    labelList& nsTransPs = sizes[UPstream::myProcNo()];
    nsTransPs.setSize(UPstream::nProcs());

    forAll(sendBufs, procI)
    {
        nsTransPs[procI] = sendBufs[procI].size();
    }

    // Send sizes across. Note: blocks.
    combineReduce(sizes, UPstream::listEq(), tag);

    if (Pstream::parRun())
    {
        label startOfRequests = Pstream::nRequests();

        // Set up receives
        // ~~~~~~~~~~~~~~~

        recvBufs.setSize(sendBufs.size());
        forAll(sizes, procI)
        {
            label nRecv = sizes[procI][UPstream::myProcNo()];

            if (procI != Pstream::myProcNo() && nRecv > 0)
            {
                recvBufs[procI].setSize(nRecv);
                UIPstream::read
                (
                    UPstream::nonBlocking,
                    procI,
                    reinterpret_cast<char*>(recvBufs[procI].begin()),
                    nRecv*sizeof(T),
                    tag
                );
            }
        }


        // Set up sends
        // ~~~~~~~~~~~~

        forAll(sendBufs, procI)
        {
            if (procI != Pstream::myProcNo() && sendBufs[procI].size() > 0)
            {
                if
                (
                   !UOPstream::write
                    (
                        UPstream::nonBlocking,
                        procI,
                        reinterpret_cast<const char*>(sendBufs[procI].begin()),
                        sendBufs[procI].size()*sizeof(T),
                        tag
                    )
                )
                {
                    FatalErrorIn("Pstream::exchange(..)")
                        << "Cannot send outgoing message. "
                        << "to:" << procI << " nBytes:"
                        << label(sendBufs[procI].size()*sizeof(T))
                        << Foam::abort(FatalError);
                }
            }
        }


        // Wait for all to finish
        // ~~~~~~~~~~~~~~~~~~~~~~

        if (block)
        {
            Pstream::waitRequests(startOfRequests);
        }
    }

    // Do myself
    recvBufs[Pstream::myProcNo()] = sendBufs[Pstream::myProcNo()];
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
