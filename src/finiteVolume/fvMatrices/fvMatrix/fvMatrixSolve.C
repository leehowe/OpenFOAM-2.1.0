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

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void Foam::fvMatrix<Type>::setComponentReference
(
    const label patchi,
    const label facei,
    const direction cmpt,
    const scalar value
)
{
    if (psi_.needReference())
    {
        if (Pstream::master())
        {
            internalCoeffs_[patchi][facei].component(cmpt) +=
                diag()[psi_.mesh().boundary()[patchi].faceCells()[facei]];

            boundaryCoeffs_[patchi][facei].component(cmpt) +=
                diag()[psi_.mesh().boundary()[patchi].faceCells()[facei]]
               *value;
        }
    }
}


template<class Type>
Foam::lduMatrix::solverPerformance Foam::fvMatrix<Type>::solve
(
    const dictionary& solverControls
)
{
    if (debug)
    {
        Info<< "fvMatrix<Type>::solve(const dictionary& solverControls) : "
               "solving fvMatrix<Type>"
            << endl;
    }

    GeometricField<Type, fvPatchField, volMesh>& psi =
       const_cast<GeometricField<Type, fvPatchField, volMesh>&>(psi_);

    lduMatrix::solverPerformance solverPerfVec
    (
        "fvMatrix<Type>::solve",
        psi.name()
    );

    scalarField saveDiag(diag());

    Field<Type> source(source_);

    // At this point include the boundary source from the coupled boundaries.
    // This is corrected for the implict part by updateMatrixInterfaces within
    // the component loop.
    addBoundarySource(source);

    typename Type::labelType validComponents
    (
        pow
        (
            psi.mesh().solutionD(),
            pTraits<typename powProduct<Vector<label>, Type::rank>::type>::zero
        )
    );

    for (direction cmpt=0; cmpt<Type::nComponents; cmpt++)
    {
        if (validComponents[cmpt] == -1) continue;

        // copy field and source

        scalarField psiCmpt(psi.internalField().component(cmpt));
        addBoundaryDiag(diag(), cmpt);

        scalarField sourceCmpt(source.component(cmpt));

        FieldField<Field, scalar> bouCoeffsCmpt
        (
            boundaryCoeffs_.component(cmpt)
        );

        FieldField<Field, scalar> intCoeffsCmpt
        (
            internalCoeffs_.component(cmpt)
        );

        lduInterfaceFieldPtrsList interfaces =
            psi.boundaryField().interfaces();

        // Use the initMatrixInterfaces and updateMatrixInterfaces to correct
        // bouCoeffsCmpt for the explicit part of the coupled boundary
        // conditions
        initMatrixInterfaces
        (
            bouCoeffsCmpt,
            interfaces,
            psiCmpt,
            sourceCmpt,
            cmpt
        );

        updateMatrixInterfaces
        (
            bouCoeffsCmpt,
            interfaces,
            psiCmpt,
            sourceCmpt,
            cmpt
        );

        lduMatrix::solverPerformance solverPerf;

        // Solver call
        solverPerf = lduMatrix::solver::New
        (
            psi.name() + pTraits<Type>::componentNames[cmpt],
            *this,
            bouCoeffsCmpt,
            intCoeffsCmpt,
            interfaces,
            solverControls
        )->solve(psiCmpt, sourceCmpt, cmpt);


        solverPerf.print();

        solverPerfVec = max(solverPerfVec, solverPerf);
        solverPerfVec.solverName() = solverPerf.solverName();

        psi.internalField().replace(cmpt, psiCmpt);
        diag() = saveDiag;
    }

    psi.correctBoundaryConditions();

    psi.mesh().setSolverPerformance(psi.name(), solverPerfVec);

    return solverPerfVec;
}


template<class Type>
Foam::autoPtr<typename Foam::fvMatrix<Type>::fvSolver>
Foam::fvMatrix<Type>::solver()
{
    return solver
    (
        psi_.mesh().solverDict
        (
            psi_.select
            (
                psi_.mesh().data::template lookupOrDefault<bool>
                ("finalIteration", false)
            )
        )
    );
}


template<class Type>
Foam::lduMatrix::solverPerformance Foam::fvMatrix<Type>::fvSolver::solve()
{
    return solve
    (
        fvMat_.psi_.mesh().solverDict
        (
            fvMat_.psi_.select
            (
                fvMat_.psi_.mesh().data::template lookupOrDefault<bool>
                ("finalIteration", false)
            )
        )
    );
}


template<class Type>
Foam::lduMatrix::solverPerformance Foam::fvMatrix<Type>::solve()
{
    return solve
    (
        psi_.mesh().solverDict
        (
            psi_.select
            (
                psi_.mesh().data::template lookupOrDefault<bool>
                ("finalIteration", false)
            )
        )
    );
}


template<class Type>
Foam::tmp<Foam::Field<Type> > Foam::fvMatrix<Type>::residual() const
{
    tmp<Field<Type> > tres(new Field<Type>(source_));
    Field<Type>& res = tres();

    addBoundarySource(res);

    // Loop over field components
    for (direction cmpt=0; cmpt<Type::nComponents; cmpt++)
    {
        scalarField psiCmpt(psi_.internalField().component(cmpt));

        scalarField boundaryDiagCmpt(psi_.size(), 0.0);
        addBoundaryDiag(boundaryDiagCmpt, cmpt);

        FieldField<Field, scalar> bouCoeffsCmpt
        (
            boundaryCoeffs_.component(cmpt)
        );

        res.replace
        (
            cmpt,
            lduMatrix::residual
            (
                psiCmpt,
                res.component(cmpt) - boundaryDiagCmpt*psiCmpt,
                bouCoeffsCmpt,
                psi_.boundaryField().interfaces(),
                cmpt
            )
        );
    }

    return tres;
}


// ************************************************************************* //
