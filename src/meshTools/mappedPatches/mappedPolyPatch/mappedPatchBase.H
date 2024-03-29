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

Class
    Foam::mappedPatchBase

Description
    Determines a mapping between patch face centres and mesh cell or face
    centres and processors they're on.

    If constructed from dictionary:
        // Region to sample (default is region0)
        sampleRegion region0;

        // What to sample:
        // - nearestCell         : sample nearest cell
        // - nearestPatchFace    : nearest face on selected patch
        // - nearestPatchFaceAMI : nearest face on selected patch
                                   - patches need not conform
                                   - uses AMI interpolation
        // - nearestFace         : nearest boundary face on any patch
        sampleMode nearestCell;

        // If sampleMod is nearestPatchFace : patch to find faces of
        samplePatch movingWall;

        // How to supply offset (w.r.t. my patch face centres):
        // - uniform : single offset vector
        // - nonuniform : per-face offset vector
        // - normal : using supplied distance and face normal
        offsetMode uniform;

        // According to offsetMode (see above) supply one of
        // offset, offsets or distance
        offset  (1 0 0);

    Note: if offsetMode is 'normal' it uses outwards pointing normals. So
    supply a negative distance if sampling inside the domain.


Note
    Storage is not optimal. It temporary collects all (patch)face centres
    on all processors to keep the addressing calculation simple.

SourceFiles
    mappedPatchBase.C

\*---------------------------------------------------------------------------*/

#ifndef mappedPatchBase_H
#define mappedPatchBase_H

#include "pointField.H"
#include "Tuple2.H"
#include "pointIndexHit.H"
#include "AMIPatchToPatchInterpolation.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class polyPatch;
class polyMesh;
class mapDistribute;

/*---------------------------------------------------------------------------*\
                       Class mappedPatchBase Declaration
\*---------------------------------------------------------------------------*/

class mappedPatchBase
{

public:

    // Type enumerations

        //- Mesh items to sample
        enum sampleMode
        {
            NEARESTCELL,         // nearest cell
            NEARESTPATCHFACE,    // faces on selected patch
            NEARESTPATCHFACEAMI, // nearest patch face + AMI interpolation
            NEARESTFACE          // nearest face
        };

        //- How to project face centres
        enum offsetMode
        {
            UNIFORM,            // single offset vector
            NONUNIFORM,         // per-face offset vector
            NORMAL              // use face normal + distance
        };

        static const NamedEnum<sampleMode, 4> sampleModeNames_;

        static const NamedEnum<offsetMode, 3> offsetModeNames_;


    //- Helper class for finding nearest
    // Nearest:
    //  - point+local index
    //  - sqr(distance)
    //  - processor
    typedef Tuple2<pointIndexHit, Tuple2<scalar, label> > nearInfo;

    class nearestEqOp
    {

    public:

        void operator()(nearInfo& x, const nearInfo& y) const
        {
            if (y.first().hit())
            {
                if (!x.first().hit())
                {
                    x = y;
                }
                else if (y.second().first() < x.second().first())
                {
                    x = y;
                }
            }
        }
    };


protected:

    // Protected data

        //- Patch to sample
        const polyPatch& patch_;

        //- Region to sample
        const word sampleRegion_;

        //- What to sample
        const sampleMode mode_;

        //- Patch (only if NEARESTPATCHFACE)
        const word samplePatch_;

        //- How to obtain samples
        offsetMode offsetMode_;

        //- Offset vector (uniform)
        vector offset_;

        //- Offset vector (nonuniform)
        vectorField offsets_;

        //- Offset distance (normal)
        scalar distance_;

        //- Same region
        const bool sameRegion_;


        // Derived information

            //- Communication schedule:
            //  - Cells/faces to sample per processor
            //  - Patch faces to receive per processor
            //  - schedule
            mutable autoPtr<mapDistribute> mapPtr_;


        // AMI interpolator

            //- Pointer to AMI interpolator
            mutable autoPtr<AMIPatchToPatchInterpolation> AMIPtr_;

            //- Flag to indicate that slave patch should be reversed for AMI
            const bool AMIReverse_;

            //- Pointer to projection surface employed by AMI interpolator
            mutable autoPtr<searchableSurface> surfPtr_;

            //- Dictionary storing projection surface description
            dictionary surfDict_;


    // Protected Member Functions

        //- Collect single list of samples and originating processor+face.
        void collectSamples
        (
            pointField&,
            labelList& patchFaceProcs,
            labelList& patchFaces,
            pointField& patchFc
        ) const;

        //- Find cells/faces containing samples
        void findSamples
        (
            const pointField&,
            labelList& sampleProcs,     // processor containing sample
            labelList& sampleIndices,   // local index of cell/face
            pointField& sampleLocations // actual representative location
        ) const;

        //- Calculate mapping
        void calcMapping() const;

        //- Calculate AMI interpolator
        void calcAMI() const;


public:

    //- Runtime type information
    TypeName("mappedPatchBase");


    // Constructors

        //- Construct from patch
        mappedPatchBase(const polyPatch&);

        //- Construct with offsetMode=non-uniform
        mappedPatchBase
        (
            const polyPatch& pp,
            const word& sampleRegion,
            const sampleMode sampleMode,
            const word& samplePatch,
            const vectorField& offsets
        );

        //- Construct from offsetMode=uniform
        mappedPatchBase
        (
            const polyPatch& pp,
            const word& sampleRegion,
            const sampleMode sampleMode,
            const word& samplePatch,
            const vector& offset
        );

        //- Construct from offsetMode=normal and distance
        mappedPatchBase
        (
            const polyPatch& pp,
            const word& sampleRegion,
            const sampleMode sampleMode,
            const word& samplePatch,
            const scalar distance
        );

        //- Construct from dictionary
        mappedPatchBase(const polyPatch&, const dictionary&);

        //- Construct as copy, resetting patch
        mappedPatchBase(const polyPatch&, const mappedPatchBase&);

        //- Construct as copy, resetting patch, map original data
        mappedPatchBase
        (
            const polyPatch&,
            const mappedPatchBase&,
            const labelUList& mapAddressing
        );


    //- Destructor
    virtual ~mappedPatchBase();


    // Member functions

        void clearOut();

        // Access

            //- What to sample
            inline const sampleMode& mode() const;

            //- Region to sample
            inline const word& sampleRegion() const;

            //- Patch (only if NEARESTPATCHFACE)
            inline const word& samplePatch() const;

            //- Offset vector (from patch faces to destination mesh objects)
            inline const vector& offset() const;

            //- Offset vector (from patch faces to destination mesh objects)
            inline const vectorField& offsets() const;

            //- Cached sampleRegion != mesh.name()
            inline bool sameRegion() const;

            //- Return reference to the parallel distribution map
            inline const mapDistribute& map() const;

            //- Return reference to the AMI interpolator
            inline const AMIPatchToPatchInterpolation& AMI
            (
                const bool forceUpdate = false
            ) const;

            //- Return a pointer to the AMI projection surface
            const autoPtr<Foam::searchableSurface>& surfPtr() const;

            //- Get the region mesh
            const polyMesh& sampleMesh() const;

            //- Get the patch on the region
            const polyPatch& samplePolyPatch() const;

            //- Get the sample points
            tmp<pointField> samplePoints() const;


        // Distribute

            //- Wrapper around map/interpolate data distribution
            template<class Type>
            void distribute(List<Type>& lst) const;

            //- Wrapper around map/interpolate data distribution with operation
            template<class Type, class BinaryOp>
            void distribute(List<Type>& lst, const BinaryOp& bop) const;

            //- Wrapper around map/interpolate data distribution
            template<class Type>
            void reverseDistribute(List<Type>& lst) const;

            //- Wrapper around map/interpolate data distribution with operation
            template<class Type, class BinaryOp>
            void reverseDistribute(List<Type>& lst, const BinaryOp& bop) const;


        // I/O

            //- Write as a dictionary
            virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "mappedPatchBaseI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "mappedPatchBaseTemplates.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
