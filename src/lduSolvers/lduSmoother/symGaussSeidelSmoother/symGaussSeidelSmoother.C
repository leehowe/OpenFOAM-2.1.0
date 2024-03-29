/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2004-6 H. Jasak All rights reserved
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Class
    symGaussSeidelSmoother

Description
    Symmetric Gauss-Seidel smoother

Author
    Hrvoje Jasak, Wikki Ltd.  All rights reserved

\*---------------------------------------------------------------------------*/

#include "symGaussSeidelSmoother.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(symGaussSeidelSmoother, 0);

    lduSmoother::addsymMatrixConstructorToTable<symGaussSeidelSmoother>
        addGaussSeidelSmootherSymMatrixConstructorToTable_;

    lduSmoother::addasymMatrixConstructorToTable<symGaussSeidelSmoother>
        addGaussSeidelSmootherAsymMatrixConstructorToTable_;

// add by RXG: begin
    lduSmoother::addsymMatrixConstructorToTableAppend<symGaussSeidelSmoother>
        addGaussSeidelSmootherSymMatrixConstructorToTableAppend_;

    lduSmoother::addasymMatrixConstructorToTableAppend<symGaussSeidelSmoother>
        addGaussSeidelSmootherAsymMatrixConstructorToTableAppend_;

// add by RXG: end
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::symGaussSeidelSmoother::symGaussSeidelSmoother
(
    const lduMatrix& matrix,
    const FieldField<Field, scalar>& coupleBouCoeffs,
    const FieldField<Field, scalar>& coupleIntCoeffs,
    const lduInterfaceFieldPtrsList& interfaces
)
:
    lduSmoother
    (
        matrix,
        coupleBouCoeffs,
        coupleIntCoeffs,
        interfaces
    ),
    gs_
    (
        matrix,
        coupleBouCoeffs,
        coupleIntCoeffs,
        interfaces
    )
{}


// add by RXG: begin
Foam::symGaussSeidelSmoother::symGaussSeidelSmoother
(
    const word& fieldName,
    const lduMatrix& matrix,
    const FieldField<Field, scalar>& coupleBouCoeffs,
    const FieldField<Field, scalar>& coupleIntCoeffs,
    const lduInterfaceFieldPtrsList& interfaces
)       
:
    lduSmoother
    (
        fieldName,
        matrix,
        coupleBouCoeffs, 
        coupleIntCoeffs,
        interfaces
    ),
    gs_
    (
        matrix,
        coupleBouCoeffs,
        coupleIntCoeffs,
        interfaces
    )
{}  
        
// add by RXG: end

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::symGaussSeidelSmoother::smooth
(
    scalarField& x,
    const scalarField& b,
    const direction cmpt,
    const label nSweeps
) const
{
    for (label sweep = 0; sweep < nSweeps; sweep++)
    {
        gs_.precondition(x, b, cmpt);
    }
}



// ************************************************************************* //
