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

#include "DataEntry.H"

// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

template<class Type>
Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const CSV<Type>& tbl
)
{
    if (os.format() == IOstream::ASCII)
    {
        os  << static_cast<const DataEntry<Type>& >(tbl)
            << token::SPACE << tbl.headerLine_
            << token::SPACE << tbl.timeColumn_
            << token::SPACE << tbl.componentColumns_
            << token::SPACE << tbl.separator_
            << token::SPACE << tbl.fileName_;
    }
    else
    {
        os  << static_cast<const DataEntry<Type>& >(tbl);
    }

    // Check state of Ostream
    os.check
    (
        "Ostream& operator<<(Ostream&, const CSV<Type>&)"
    );

    return os;
}


template<class Type>
void Foam::CSV<Type>::writeData(Ostream& os) const
{
    DataEntry<Type>::writeData(os);

    os  << token::END_STATEMENT << nl;
    os  << indent << word(type() + "Coeffs") << nl;
    os  << indent << token::BEGIN_BLOCK << incrIndent << nl;
    os.writeKeyword("headerLine") << headerLine_ << token::END_STATEMENT << nl;
    os.writeKeyword("refColumn") << refColumn_ << token::END_STATEMENT << nl;
    os.writeKeyword("componentColumns") << componentColumns_
        << token::END_STATEMENT << nl;
    os.writeKeyword("separator") << string(separator_)
        << token::END_STATEMENT << nl;
    os.writeKeyword("fileName") << fName_ << token::END_STATEMENT << nl;
    os  << decrIndent << indent << token::END_BLOCK << endl;
}


// ************************************************************************* //
