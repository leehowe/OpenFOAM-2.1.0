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

#include "polynomial.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(polynomial, 0);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::polynomial::polynomial(const word& entryName, const dictionary& dict)
:
    DataEntry<scalar>(entryName),
    coeffs_(),
    canIntegrate_(true)
{
    Istream& is(dict.lookup(entryName));
    word entryType(is);

    is  >> coeffs_;

    if (!coeffs_.size())
    {
        FatalErrorIn("Foam::polynomial::polynomial(const word&, Istream&)")
            << "polynomial coefficients for entry " << this->name_
            << " are invalid (empty)" << nl << exit(FatalError);
    }

    forAll(coeffs_, i)
    {
        if (mag(coeffs_[i].second() + 1) < ROOTVSMALL)
        {
            canIntegrate_ = false;
            break;
        }
    }

    if (debug)
    {
        if (!canIntegrate_)
        {
            WarningIn("Foam::polynomial::polynomial(const word&, Istream&)")
                << "Polynomial " << this->name_ << " cannot be integrated"
                << endl;
        }
    }
}


Foam::polynomial::polynomial(const polynomial& poly)
:
    DataEntry<scalar>(poly),
    coeffs_(poly.coeffs_),
    canIntegrate_(poly.canIntegrate_)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::polynomial::~polynomial()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::scalar Foam::polynomial::value(const scalar x) const
{
    scalar y = 0.0;
    forAll(coeffs_, i)
    {
        y += coeffs_[i].first()*pow(x, coeffs_[i].second());
    }

    return y;
}


Foam::scalar Foam::polynomial::integrate(const scalar x1, const scalar x2) const
{
    scalar intx = 0.0;

    if (canIntegrate_)
    {
        forAll(coeffs_, i)
        {
            intx +=
                coeffs_[i].first()/(coeffs_[i].second() + 1)
               *(
                    pow(x2, coeffs_[i].second() + 1)
                  - pow(x1, coeffs_[i].second() + 1)
                );
        }
    }

    return intx;
}


// ************************************************************************* //
