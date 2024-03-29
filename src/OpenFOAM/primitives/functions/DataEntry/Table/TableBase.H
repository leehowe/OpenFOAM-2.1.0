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
    Foam::TableBase

Description
    Base class for table with bounds handling, interpolation and integration

SourceFiles
    TableBase.C

\*---------------------------------------------------------------------------*/

#ifndef TableBase_H
#define TableBase_H

#include "DataEntry.H"
#include "Tuple2.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template<class Type>
class TableBase;

template<class Type>
Ostream& operator<<
(
    Ostream&,
    const TableBase<Type>&
);

/*---------------------------------------------------------------------------*\
                        Class TableBase Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class TableBase
{
public:

    // Public data types

        //- Enumeration for handling out-of-bound values
        enum boundsHandling
        {
            ERROR,          /*!< Exit with a FatalError */
            WARN,           /*!< Issue warning and clamp value (default) */
            CLAMP,          /*!< Clamp value to the start/end value */
            REPEAT          /*!< Treat as a repeating list */
        };


protected:

    // Protected data

        //- Table name
        word name_;

        //- Enumeration for handling out-of-bound values
        boundsHandling boundsHandling_;

        //- Table data
        List<Tuple2<scalar, Type> > table_;


    // Protected Member Functions

        //- Disallow default bitwise assignment
        void operator=(const TableBase<Type>&);


public:

    // Constructors

        //- Construct from dictionary - note table is not populated
        TableBase(const word& name, const dictionary& dict);

        //- Copy constructor
        TableBase(const TableBase<Type>& tbl);


    //- Destructor
    virtual ~TableBase();


    // Member Functions

        //- Return the out-of-bounds handling as a word
        word boundsHandlingToWord(const boundsHandling& bound) const;

        //- Return the out-of-bounds handling as an enumeration
        boundsHandling wordToBoundsHandling(const word& bound) const;

        //- Set the out-of-bounds handling from enum, return previous setting
        boundsHandling outOfBounds(const boundsHandling& bound);

        //- Check the table for size and consistency
        void check() const;

        //- Check minimum table bounds
        bool checkMinBounds(const scalar x, scalar& xDash) const;

        //- Check maximum table bounds
        bool checkMaxBounds(const scalar x, scalar& xDash) const;

        //- Return Table value
        virtual Type value(const scalar x) const;

        //- Integrate between two (scalar) values
        virtual Type integrate(const scalar x1, const scalar x2) const;


    // I/O

        //- Ostream Operator
        friend Ostream& operator<< <Type>
        (
            Ostream& os,
            const TableBase<Type>& tbl
        );

        //- Write in dictionary format
        virtual void writeData(Ostream& os) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "TableBase.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
