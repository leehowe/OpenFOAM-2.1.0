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

#include "regionModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

inline const Foam::fvMesh&
Foam::regionModels::regionModel::primaryMesh() const
{
    return primaryMesh_;
}


inline const Foam::Time& Foam::regionModels::regionModel::time() const
{
    return time_;
}


inline const Foam::Switch& Foam::regionModels::regionModel::active() const
{
    return active_;
}


inline const Foam::Switch& Foam::regionModels::regionModel::infoOutput() const
{
    return infoOutput_;
}


inline const Foam::word& Foam::regionModels::regionModel::modelName() const
{
    return modelName_;
}


inline const Foam::fvMesh& Foam::regionModels::regionModel::regionMesh() const
{
    if (!regionMeshPtr_.valid())
    {
        FatalErrorIn
        (
            "inline const Foam::fvMesh&"
            "Foam::regionModels::regionModel::regionMesh() const"
        )<< "Region mesh not available" << abort(FatalError);
    }

    return regionMeshPtr_();
}


inline Foam::fvMesh& Foam::regionModels::regionModel::regionMesh()
{
    if (!regionMeshPtr_.valid())
    {
        FatalErrorIn
        (
            "inline Foam::fvMesh&"
            "Foam::regionModels::regionModel::regionMesh()"
        )<< "Region mesh not available" << abort(FatalError);
    }

    return regionMeshPtr_();
}


inline const Foam::dictionary& Foam::regionModels::regionModel::coeffs() const
{
    return coeffs_;
}


inline const Foam::dictionary&
Foam::regionModels::regionModel::solution() const
{
    return regionMesh().solutionDict();
}


inline bool Foam::regionModels::regionModel::isRegionPatch
(
    const label patchI
) const
{
    forAll(primaryPatchIDs_, i)
    {
        if (primaryPatchIDs_[i] == patchI)
        {
            return true;
        }
    }

    return false;
}


inline const Foam::labelList&
Foam::regionModels::regionModel::primaryPatchIDs() const
{
    return primaryPatchIDs_;
}


inline const Foam::labelList&
Foam::regionModels::regionModel::intCoupledPatchIDs() const
{
    return intCoupledPatchIDs_;
}


inline Foam::label Foam::regionModels::regionModel::regionPatchID
(
    const label primaryPatchID
) const
{
    forAll(primaryPatchIDs_, i)
    {
        if (primaryPatchIDs_[i] == primaryPatchID)
        {
            return intCoupledPatchIDs_[i];
        }
    }

    return -1;
}


// ************************************************************************* //
