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

InClass
    decompositionMethod

\*---------------------------------------------------------------------------*/

#include "decompositionMethod.H"
#include "globalIndex.H"
#include "cyclicPolyPatch.H"
#include "syncTools.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(decompositionMethod, 0);
    defineRunTimeSelectionTable(decompositionMethod, dictionary);
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::autoPtr<Foam::decompositionMethod> Foam::decompositionMethod::New
(
    const dictionary& decompositionDict
)
{
    const word methodType(decompositionDict.lookup("method"));

    Info<< "Selecting decompositionMethod " << methodType << endl;

    dictionaryConstructorTable::iterator cstrIter =
        dictionaryConstructorTablePtr_->find(methodType);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        FatalErrorIn
        (
            "decompositionMethod::New"
            "(const dictionary& decompositionDict)"
        )   << "Unknown decompositionMethod "
            << methodType << nl << nl
            << "Valid decompositionMethods are : " << endl
            << dictionaryConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<decompositionMethod>(cstrIter()(decompositionDict));
}


Foam::labelList Foam::decompositionMethod::decompose
(
    const polyMesh& mesh,
    const pointField& points
)
{
    scalarField weights(points.size(), 1.0);

    return decompose(mesh, points, weights);
}


Foam::labelList Foam::decompositionMethod::decompose
(
    const polyMesh& mesh,
    const labelList& fineToCoarse,
    const pointField& coarsePoints,
    const scalarField& coarseWeights
)
{
    CompactListList<label> coarseCellCells;
    calcCellCells
    (
        mesh,
        fineToCoarse,
        coarsePoints.size(),
        coarseCellCells
    );

    // Decompose based on agglomerated points
    labelList coarseDistribution
    (
        decompose
        (
            coarseCellCells(),
            coarsePoints,
            coarseWeights
        )
    );

    // Rework back into decomposition for original mesh_
    labelList fineDistribution(fineToCoarse.size());

    forAll(fineDistribution, i)
    {
        fineDistribution[i] = coarseDistribution[fineToCoarse[i]];
    }

    return fineDistribution;
}


Foam::labelList Foam::decompositionMethod::decompose
(
    const polyMesh& mesh,
    const labelList& fineToCoarse,
    const pointField& coarsePoints
)
{
    scalarField cWeights(coarsePoints.size(), 1.0);

    return decompose
    (
        mesh,
        fineToCoarse,
        coarsePoints,
        cWeights
    );
}


Foam::labelList Foam::decompositionMethod::decompose
(
    const labelListList& globalCellCells,
    const pointField& cc
)
{
    scalarField cWeights(cc.size(), 1.0);

    return decompose(globalCellCells, cc, cWeights);
}


void Foam::decompositionMethod::calcCellCells
(
    const polyMesh& mesh,
    const labelList& agglom,
    const label nCoarse,
    CompactListList<label>& cellCells
)
{
    const labelList& faceOwner = mesh.faceOwner();
    const labelList& faceNeighbour = mesh.faceNeighbour();
    const polyBoundaryMesh& patches = mesh.boundaryMesh();


    // Create global cell numbers
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~

    globalIndex globalAgglom(nCoarse);


    // Get agglomerate owner on other side of coupled faces
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    labelList globalNeighbour(mesh.nFaces()-mesh.nInternalFaces());

    forAll(patches, patchI)
    {
        const polyPatch& pp = patches[patchI];

        if (pp.coupled())
        {
            label faceI = pp.start();
            label bFaceI = pp.start() - mesh.nInternalFaces();

            forAll(pp, i)
            {
                globalNeighbour[bFaceI] = globalAgglom.toGlobal
                (
                    agglom[faceOwner[faceI]]
                );

                bFaceI++;
                faceI++;
            }
        }
    }

    // Get the cell on the other side of coupled patches
    syncTools::swapBoundaryFaceList(mesh, globalNeighbour);



    //// Determine the cellCells (in agglomeration numbering) on coupled faces
    //// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //
    //labelListList globalCellCells(mesh.nFaces()-mesh.nInternalFaces());
    //
    //// Current set of face neighbours for the current cell
    //labelHashSet cCells;
    //
    //forAll(patches, patchI)
    //{
    //    const polyPatch& pp = patches[patchI];
    //
    //    if (pp.coupled())
    //    {
    //        label faceI = pp.start();
    //        label bFaceI = pp.start() - mesh.nInternalFaces();
    //
    //        forAll(pp, i)
    //        {
    //            label cellI = faceOwner[faceI];
    //            label globalCellI = globalAgglom.toGlobal(agglom[cellI]);
    //
    //            // First check if agglomerated across coupled patches at all
    //            // so we don't use memory if not needed
    //            if (globalNeighbour[bFaceI] == globalCellI)
    //            {
    //                cCells.clear();
    //
    //                const cell& cFaces = mesh.cells()[cellI];
    //
    //                forAll(cFaces, i)
    //                {
    //                    if (mesh.isInternalFace(cFaces[i]))
    //                    {
    //                        label otherCellI = faceOwner[cFaces[i]];
    //                        if (otherCellI == cellI)
    //                        {
    //                            otherCellI = faceNeighbour[cFaces[i]];
    //                        }
    //
    //                        cCells.insert
    //                        (
    //                            globalAgglom.toGlobal
    //                            (
    //                                agglom[otherCellI]
    //                            )
    //                        );
    //                    }
    //                }
    //                globalCellCells[bFaceI] = cCells.toc();
    //            }
    //
    //            bFaceI++;
    //            faceI++;
    //        }
    //    }
    //}
    //
    //// Get the cell on the other side of coupled patches
    //syncTools::syncBoundaryFaceList
    //(
    //    mesh,
    //    globalCellCells,
    //    eqOp<labelList>(),
    //    dummyTransform()
    //);




    // Count number of faces (internal + coupled)
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Number of faces per coarse cell
    labelList nFacesPerCell(nCoarse, 0);

    for (label faceI = 0; faceI < mesh.nInternalFaces(); faceI++)
    {
        label own = agglom[faceOwner[faceI]];
        label nei = agglom[faceNeighbour[faceI]];

        nFacesPerCell[own]++;
        nFacesPerCell[nei]++;
    }

    forAll(patches, patchI)
    {
        const polyPatch& pp = patches[patchI];

        if (pp.coupled())
        {
            label faceI = pp.start();
            label bFaceI = pp.start()-mesh.nInternalFaces();

            forAll(pp, i)
            {
                label own = agglom[faceOwner[faceI]];

                //const labelList& cCells = globalCellCells[bFaceI];
                //
                //forAll(cCells, i)
                //{
                //    label globalNei = cCells[i];
                //
                //    // Allow only processor-local agglomeration
                //    if (globalAgglom.isLocal(globalNei))
                //    {
                //        nFacesPerCell[own]++;
                //    }
                //}
                label globalNei = globalNeighbour[bFaceI];
                if
                (
                    globalAgglom.isLocal(globalNei)
                 && globalAgglom.toLocal(globalNei) != own
                )
                {
                    nFacesPerCell[own]++;
                }

                faceI++;
                bFaceI++;
            }
        }
    }


    // Fill in offset and data
    // ~~~~~~~~~~~~~~~~~~~~~~~

    cellCells.setSize(nFacesPerCell);

    nFacesPerCell = 0;

    labelList& m = cellCells.m();
    const labelList& offsets = cellCells.offsets();

    // For internal faces is just offsetted owner and neighbour
    for (label faceI = 0; faceI < mesh.nInternalFaces(); faceI++)
    {
        label own = agglom[faceOwner[faceI]];
        label nei = agglom[faceNeighbour[faceI]];

        m[offsets[own] + nFacesPerCell[own]++] = globalAgglom.toGlobal(nei);
        m[offsets[nei] + nFacesPerCell[nei]++] = globalAgglom.toGlobal(own);
    }

    // For boundary faces is offsetted coupled neighbour
    forAll(patches, patchI)
    {
        const polyPatch& pp = patches[patchI];

        if (pp.coupled())
        {
            label faceI = pp.start();
            label bFaceI = pp.start()-mesh.nInternalFaces();

            forAll(pp, i)
            {
                label own = agglom[faceOwner[faceI]];

                //const labelList& cCells = globalCellCells[bFaceI];
                //
                //forAll(cCells, i)
                //{
                //    label globalNei = cCells[i];
                //
                //    // Allow only processor-local agglomeration
                //    if (globalAgglom.isLocal(globalNei))
                //    {
                //        m[offsets[own] + nFacesPerCell[own]++] = globalNei;
                //    }
                //}
                label globalNei = globalNeighbour[bFaceI];
                if
                (
                    globalAgglom.isLocal(globalNei)
                 && globalAgglom.toLocal(globalNei) != own
                )
                {
                    m[offsets[own] + nFacesPerCell[own]++] = globalNei;
                }

                faceI++;
                bFaceI++;
            }
        }
    }


    // Check for duplicates connections between cells
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Done as postprocessing step since we now have cellCells.
    label startIndex = 0;
    label newIndex = 0;
    labelHashSet nbrCells;
    forAll(cellCells, cellI)
    {
        nbrCells.clear();
        nbrCells.insert(cellI);

        label& endIndex = cellCells.offsets()[cellI+1];

        for (label i = startIndex; i < endIndex; i++)
        {
            if (nbrCells.insert(cellCells.m()[i]))
            {
                cellCells.m()[newIndex++] = cellCells.m()[i];
            }
        }

        startIndex = endIndex;
        endIndex = newIndex;
    }

    cellCells.m().setSize(newIndex);


    //forAll(cellCells, cellI)
    //{
    //    const labelUList cCells = cellCells[cellI];
    //    Pout<< "Compacted: Coarse cell " << cellI << endl;
    //    forAll(cCells, i)
    //    {
    //        Pout<< "    nbr:" << cCells[i] << endl;
    //    }
    //}
}


// ************************************************************************* //
