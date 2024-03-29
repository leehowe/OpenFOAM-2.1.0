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

inline const Foam::mappedPatchBase::sampleMode&
Foam::mappedPatchBase::mode() const
{
    return mode_;
}


inline const Foam::word& Foam::mappedPatchBase::sampleRegion() const
{
    return sampleRegion_;
}


inline const Foam::word& Foam::mappedPatchBase::samplePatch() const
{
    return samplePatch_;
}


inline const Foam::vector& Foam::mappedPatchBase::offset() const
{
    return offset_;
}


inline const Foam::vectorField& Foam::mappedPatchBase::offsets() const
{
    return offsets_;
}


inline bool Foam::mappedPatchBase::sameRegion() const
{
    return sameRegion_;
}


inline const Foam::mapDistribute& Foam::mappedPatchBase::map() const
{
    if (mapPtr_.empty())
    {
        calcMapping();
    }

    return mapPtr_();
}


inline const Foam::AMIPatchToPatchInterpolation& Foam::mappedPatchBase::AMI
(
    bool forceUpdate
) const
{
    if (forceUpdate || AMIPtr_.empty())
    {
        calcAMI();
    }

    return AMIPtr_();
}


// ************************************************************************* //
