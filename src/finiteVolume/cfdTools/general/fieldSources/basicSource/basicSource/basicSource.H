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
    Foam::basicSource

Description
    Field source abtract base class.  Provides a base set of controls, e.g.

        type            scalarExplicitSource    // source type
        active          on;                     // on/off switch
        timeStart       0.0;                    // start time
        duration        1000.0;                 // duration
        selectionMode   cellSet;                // cellSet // points //cellZone

    On evaluation, source expects to be added to the rhs of the equation


SourceFiles
    basicSource.C
    basicSourceIO.C

\*---------------------------------------------------------------------------*/

#ifndef basicSource_H
#define basicSource_H

#include "fvMatricesFwd.H"
#include "cellSet.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class fvMesh;

/*---------------------------------------------------------------------------*\
                         Class basicSource Declaration
\*---------------------------------------------------------------------------*/

class basicSource
{
public:

    // Public data

        //- Enumeration for selection mode types
        enum selectionModeType
        {
            smPoints,
            smCellSet,
            smCellZone,
            smAll
        };

        //- Word list of selection mode type names
        static const NamedEnum<selectionModeType, 4> selectionModeTypeNames_;


protected:

    // Protected data

        //- Source name
        word name_;

        //- Reference to the mesh database
        const fvMesh& mesh_;

        //- Top level source dictionary
        dictionary dict_;

        //- Dictionary containing source coefficients
        dictionary coeffs_;

        //- Source active flag
        bool active_;

        //- Time start
        scalar timeStart_;

        //- Duration
        scalar duration_;

        //- Cell selection mode
        selectionModeType selectionMode_;

        //- Name of cell set for "cellSet" and "cellZone" selectionMode
        word cellSetName_;

        //- List of points for "points" selectionMode
        List<point> points_;

        //- Set of cells to apply source to
        labelList cells_;

        //- Sum of cell volumes
        scalar V_;

        //- Field names to apply source to - populated by derived models
        wordList fieldNames_;

        //- Applied flag list - corresponds to each fieldNames_ entry
        List<bool> applied_;


    // Protected functions

        //- Set the cellSet or points selection
        void setSelection(const dictionary& dict);

        //- Set the cell set based on the user input selection mode
        void setCellSet();


public:

    //- Runtime type information
    TypeName("basicSource");


     // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            basicSource,
            dictionary,
            (
                const word& name,
                const word& modelType,
                const dictionary& dict,
                const fvMesh& mesh
            ),
            (name, modelType, dict, mesh)
        );


    // Constructors

        //- Construct from components
        basicSource
        (
            const word& name,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );

        //- Return clone
        autoPtr<basicSource> clone() const
        {
            notImplemented("autoPtr<basicSource> clone() const");
            return autoPtr<basicSource>(NULL);
        }

        //- Return pointer to new basicSource object created
        //  on the freestore from an Istream
        class iNew
        {
            //- Reference to the mesh database
            const fvMesh& mesh_;
            const word& name_;

        public:

            iNew
            (
                const fvMesh& mesh,
                const word& name
            )
            :
                mesh_(mesh),
                name_(name)
            {}

            autoPtr<basicSource> operator()(Istream& is) const
            {
                //const word name(is);
                const dictionary dict(is);

                return autoPtr<basicSource>
                (
                    basicSource::New
                    (
                        name_,
                        dict,
                        mesh_
                    )
                );
            }
        };


    // Selectors

        //- Return a reference to the selected basicSource model
        static autoPtr<basicSource> New
        (
            const word& name,
            const dictionary& dict,
            const fvMesh& mesh
        );


    //- Destructor
    virtual ~basicSource()
    {}


    // Member Functions

        // Access

            //- Return const access to the source name
            inline const word& name() const;

            //- Return const access to the mesh database
            inline const fvMesh& mesh() const;

            //- Return dictionary
            inline const dictionary& coeffs() const;

            //- Return const access to the source active flag
            inline bool active() const;

            //- Return const access to the time start
            inline scalar timeStart() const;

            //- Return const access to the duration
            inline scalar duration() const;

            //- Return const access to the time end
            inline scalar timeEnd() const;

            //- Return const access to the cell selection mode
            inline const selectionModeType& selectionMode() const;

            //- Return const access to the name of cell set for "cellSet"
            //  selectionMode
            inline const word& cellSetName() const;

            //- Return const access to the total cell volume
            inline scalar V() const;

            //- Return const access to the cell set
            inline const labelList& cells() const;

            //- Set the applied flag to true for field index fieldI
            inline void setApplied(const label fieldI);


        // Edit

            //- Return access to the source active flag
            inline bool& active();

            //- Return access to the time start
            inline scalar& timeStart();

            //- Return access to the duration
            inline scalar& duration();


        // Checks

            //- Is the source active?
            virtual bool isActive();

            //- Return index of field name if found in fieldNames list
            virtual label applyToField(const word& fieldName) const;

            //- Check that the source has been applied
            virtual void checkApplied() const;


        // Evaluation

            // Add explicit and implicit contributions

                //- Scalar
                virtual void addSup
                (
                    fvMatrix<scalar>& eqn,
                    const label fieldI
                );

                //- Vector
                virtual void addSup
                (
                    fvMatrix<vector>& eqn,
                    const label fieldI
                );

                //- Spherical tensor
                virtual void addSup
                (
                    fvMatrix<symmTensor>& eqn,
                    const label fieldI
                );

                //- Symmetric tensor
                virtual void addSup
                (
                    fvMatrix<sphericalTensor>& eqn,
                    const label fieldI
                );

                //- Tensor
                virtual void addSup
                (
                    fvMatrix<tensor>& eqn,
                    const label fieldI
                );


            // Set values directly

                //- Scalar
                virtual void setValue
                (
                    fvMatrix<scalar>& eqn,
                    const label fieldI
                );

                //- Vector
                virtual void setValue
                (
                    fvMatrix<vector>& eqn,
                    const label fieldI
                );

                //- Spherical tensor
                virtual void setValue
                (
                    fvMatrix<sphericalTensor>& eqn,
                    const label fieldI
                );

                //- Symmetric tensor
                virtual void setValue
                (
                    fvMatrix<symmTensor>& eqn,
                    const label fieldI
                );

                //- Tensor
                virtual void setValue
                (
                    fvMatrix<tensor>& eqn,
                    const label fieldI
                );


        // I-O

            //- Write the source header information
            virtual void writeHeader(Ostream&) const;

            //- Write the source footer information
            virtual void writeFooter(Ostream&) const;

            //- Write the source properties
            virtual void writeData(Ostream&) const;

            //- Read source dictionary
            virtual bool read(const dictionary& dict);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "basicSourceI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
