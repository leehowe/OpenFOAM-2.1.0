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

#include "thermoBaffleModel.H"
#include "fvMesh.H"
#include "mappedVariableThicknessWallPolyPatch.H"
#include "wedgePolyPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace thermoBaffleModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(thermoBaffleModel, 0);
defineRunTimeSelectionTable(thermoBaffleModel, mesh);
defineRunTimeSelectionTable(thermoBaffleModel, dictionary);


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool thermoBaffleModel::read()
{
    regionModel1D::read();
    return true;
}


void thermoBaffleModel::init()
{
    if (active_)
    {
        const polyBoundaryMesh& rbm = regionMesh().boundaryMesh();

        // Check if region mesh in 1-D
        label nTotalEdges = 0;
        const label patchi = intCoupledPatchIDs_[0];
        nTotalEdges = 2*nLayers_*rbm[patchi].nInternalEdges();
        nTotalEdges +=
            nLayers_*(rbm[patchi].nEdges() - rbm[patchi].nInternalEdges());

        reduce(nTotalEdges, sumOp<label>());

        label nFaces = 0;
        forAll (rbm, patchi)
        {
            if (
                   rbm[patchi].size()
                &&
                   (
                       isA<wedgePolyPatch>(rbm[patchi])
                    || isA<emptyPolyPatch>(rbm[patchi])
                   )
                )
            {
                nFaces += rbm[patchi].size();
            }
        }
        reduce(nFaces, sumOp<label>());

        if (nTotalEdges == nFaces)
        {
            oneD_ = true;
            Info << "\nThe thermal baffle is 1D \n" << endl;
        }
        else
        {
            Info << "\nThe thermal baffle is 3D \n" << endl;
        }

        forAll(intCoupledPatchIDs_, i)
        {
            const label patchI = intCoupledPatchIDs_[i];
            const polyPatch& pp = rbm[patchI];

            if  (
                    !isA<mappedVariableThicknessWallPolyPatch>(pp)
                 && oneD_
                 && !constantThickness_
                )
            {
                FatalErrorIn
                (
                    "thermoBaffleModel::thermoBaffleModel"
                    "("
                    "   const word&,"
                    "   const fvMesh&"
                    ")"
                )   << "\n    patch type '" << pp.type()
                    << "' not type '"
                    << mappedVariableThicknessWallPolyPatch::typeName
                    << "'. This is necessary for 1D solution "
                    << " and variable thickness"
                    << "\n    for patch. " << pp.name()
                    << exit(FatalError);
            }
            else if (!isA<mappedWallPolyPatch>(pp))
            {
                FatalErrorIn
                (
                    "thermoBaffleModel::thermoBaffleModel"
                    "("
                    "   const word&,"
                    "   const fvMesh&"
                    ")"
                )   << "\n    patch type '" << pp.type()
                    << "' not type '"
                    << mappedWallPolyPatch::typeName
                    << "'. This is necessary for 3D solution"
                    << "\n    for patch. " << pp.name()
                    << exit(FatalError);
            }
        }

        if (oneD_ && !constantThickness_)
        {
            const label patchI = intCoupledPatchIDs_[0];
            const polyPatch& pp = rbm[patchI];
            const mappedVariableThicknessWallPolyPatch& ppCoupled =
                refCast
                <
                    const mappedVariableThicknessWallPolyPatch
                >(pp);

            thickness_ = ppCoupled.thickness();

            // Check that thickness has the right size
            if (thickness_.size() != pp.size())
            {
                FatalErrorIn
                (
                    "thermoBaffleModel::thermoBaffleModel"
                    "("
                    "   const word&,"
                    "   const fvMesh&"
                    ")"
                )   << " coupled patches in thermoBaffle are " << nl
                    << " different sizes from list thickness" << nl
                    << exit(FatalError);
            }

            // Calculate thickness of the baffle on the first face only.
            if (delta_.value() == 0.0)
            {
                forAll (ppCoupled, localFaceI)
                {
                    label faceI = ppCoupled.start() + localFaceI;

                    label faceO =
                        boundaryFaceOppositeFace_[localFaceI];

                    delta_.value() = mag
                    (
                        regionMesh().faceCentres()[faceI]
                      - regionMesh().faceCentres()[faceO]
                    );
                    break;
                }
            }
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

thermoBaffleModel::thermoBaffleModel(const fvMesh& mesh)
:
    regionModel1D(mesh),
    thickness_(),
    delta_("delta", dimLength, 0.0),
    oneD_(false),
    constantThickness_(true)
{}


thermoBaffleModel::thermoBaffleModel
(
    const word& modelType,
    const fvMesh& mesh,
    const dictionary& dict

)
:
    regionModel1D(mesh, "thermoBaffle", modelType, dict, true),
    thickness_(),
    delta_("delta", dimLength, 0.0),
    oneD_(false),
    constantThickness_(dict.lookupOrDefault<bool>("constantThickness", true))
{
    init();
}


thermoBaffleModel::thermoBaffleModel(const word& modelType, const fvMesh& mesh)
:
    regionModel1D(mesh, "thermoBaffle", modelType),
    thickness_(),
    delta_("delta", dimLength, 0.0),
    oneD_(false),
    constantThickness_(lookupOrDefault<bool>("constantThickness", true))
{
    init();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

thermoBaffleModel::~thermoBaffleModel()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void thermoBaffleModel::preEvolveRegion()
{}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace thermoBaffleModels
} // End namespace regionModels
} // End namespace Foam

// ************************************************************************* //
