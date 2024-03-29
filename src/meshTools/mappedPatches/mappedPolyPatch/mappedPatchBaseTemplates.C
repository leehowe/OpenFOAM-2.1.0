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

template<class Type>
void Foam::mappedPatchBase::distribute(List<Type>& lst) const
{
    switch (mode_)
    {
        case NEARESTPATCHFACEAMI:
        {
            lst = AMI().interpolateToSource(Field<Type>(lst.xfer()));
            break;
        }
        default:
        {
            map().distribute(lst);
        }
    }
}


template<class Type, class BinaryOp>
void Foam::mappedPatchBase::distribute
(
    List<Type>& lst,
    const BinaryOp& bop
) const
{
    switch (mode_)
    {
        case NEARESTPATCHFACEAMI:
        {
            lst = AMI().interpolateToSource
                (
                    Field<Type>(lst.xfer()),
                    bop
                );
            break;
        }
        default:
        {
            map().distribute
            (
                Pstream::defaultCommsType,
                map().schedule(),
                map().constructSize(),
                map().subMap(),
                map().constructMap(),
                lst,
                bop,
                pTraits<Type>::zero
            );
        }
    }
}


template<class Type>
void Foam::mappedPatchBase::reverseDistribute(List<Type>& lst) const
{
    switch (mode_)
    {
        case NEARESTPATCHFACEAMI:
        {
            lst = AMI().interpolateToTarget(Field<Type>(lst.xfer()));
            break;
        }
        default:
        {
            label cSize = patch_.size();
            map().reverseDistribute(cSize, lst);
        }
    }
}


template<class Type, class BinaryOp>
void Foam::mappedPatchBase::reverseDistribute
(
    List<Type>& lst,
    const BinaryOp& bop
) const
{
    switch (mode_)
    {
        case NEARESTPATCHFACEAMI:
        {
            lst = AMI().interpolateToTarget
                (
                    Field<Type>(lst.xfer()),
                    bop
                );
            break;
        }
        default:
        {
            label cSize = patch_.size();
            map().distribute
            (
                Pstream::defaultCommsType,
                map().schedule(),
                cSize,
                map().constructMap(),
                map().subMap(),
                lst,
                bop,
                pTraits<Type>::zero
            );
        }
    }
}


// ************************************************************************* //
