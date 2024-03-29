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

#include "faceSource.H"
#include "surfaceFields.H"
#include "volFields.H"
#include "sampledSurface.H"

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

template<class Type>
bool Foam::fieldValues::faceSource::validField(const word& fieldName) const
{
    typedef GeometricField<Type, fvsPatchField, surfaceMesh> sf;
    typedef GeometricField<Type, fvPatchField, volMesh> vf;

    if (source_ != stSampledSurface && obr_.foundObject<sf>(fieldName))
    {
        return true;
    }
    else if (obr_.foundObject<vf>(fieldName))
    {
        return true;
    }

    return false;
}


template<class Type>
Foam::tmp<Foam::Field<Type> > Foam::fieldValues::faceSource::getFieldValues
(
    const word& fieldName,
    const bool mustGet
) const
{
    typedef GeometricField<Type, fvsPatchField, surfaceMesh> sf;
    typedef GeometricField<Type, fvPatchField, volMesh> vf;

    if (source_ != stSampledSurface && obr_.foundObject<sf>(fieldName))
    {
        return filterField(obr_.lookupObject<sf>(fieldName), true);
    }
    else if (obr_.foundObject<vf>(fieldName))
    {
        if (surfacePtr_.valid())
        {
            return surfacePtr_().sample(obr_.lookupObject<vf>(fieldName));
        }
        else
        {
            return filterField(obr_.lookupObject<vf>(fieldName), true);
        }
    }

    if (mustGet)
    {
        FatalErrorIn
        (
            "Foam::tmp<Foam::Field<Type> > "
            "Foam::fieldValues::faceSource::getFieldValues"
            "("
                "const word&, "
                "const bool"
            ") const"
        )   << "Field " << fieldName << " not found in database"
            << abort(FatalError);
    }

    return tmp<Field<Type> >(new Field<Type>(0));
}


template<class Type>
Type Foam::fieldValues::faceSource::processValues
(
    const Field<Type>& values,
    const scalarField& magSf,
    const scalarField& weightField
) const
{
    Type result = pTraits<Type>::zero;
    switch (operation_)
    {
        case opSum:
        {
            result = sum(values);
            break;
        }
        case opAreaAverage:
        {
            result = sum(values*magSf)/sum(magSf);
            break;
        }
        case opAreaIntegrate:
        {
            result = sum(values*magSf);
            break;
        }
        case opWeightedAverage:
        {
            result = sum(values*weightField)/sum(weightField);
            break;
        }
        case opMin:
        {
            result = min(values);
            break;
        }
        case opMax:
        {
            result = max(values);
            break;
        }
        case opCoV:
        {
            Type meanValue = sum(values*magSf)/sum(magSf);

            const label nComp = pTraits<Type>::nComponents;

            for (direction d=0; d<nComp; ++d)
            {
                scalarField vals(values.component(d));
                scalar mean = component(meanValue, d);
                scalar& res = setComponent(result, d);

                res =
                    sqrt(sum(magSf*sqr(vals - mean))/(magSf.size()*sum(magSf)))
                   /mean;
            }

            break;
        }
        default:
        {
            // Do nothing
        }
    }

    return result;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
bool Foam::fieldValues::faceSource::writeValues(const word& fieldName)
{
    const bool ok = validField<Type>(fieldName);

    if (ok)
    {
        Field<Type> values(getFieldValues<Type>(fieldName));
        scalarField weightField;

        if (operation_ == opWeightedAverage)
        {
            weightField = getFieldValues<scalar>(weightFieldName_, true);
        }

        scalarField magSf;

        if (surfacePtr_.valid())
        {
            // Get unoriented magSf
            magSf = surfacePtr_().magSf();
        }
        else
        {
            // Get unoriented magSf
            magSf = filterField(mesh().magSf(), false);
        }

        // Combine onto master
        combineFields(values);
        combineFields(magSf);
        combineFields(weightField);


        if (Pstream::master())
        {
            Type result = processValues(values, magSf, weightField);

            if (valueOutput_)
            {
                IOField<Type>
                (
                    IOobject
                    (
                        fieldName + "_" + sourceTypeNames_[source_] + "-"
                            + sourceName_,
                        obr_.time().timeName(),
                        obr_,
                        IOobject::NO_READ,
                        IOobject::NO_WRITE
                    ),
                    values
                ).write();
            }

            outputFilePtr_()<< tab << result;

            if (log_)
            {
                Info<< "    " << operationTypeNames_[operation_]
                    << "(" << sourceName_ << ") for " << fieldName
                    <<  " = " << result << endl;
            }
        }
    }

    return ok;
}


template<class Type>
Foam::tmp<Foam::Field<Type> > Foam::fieldValues::faceSource::filterField
(
    const GeometricField<Type, fvPatchField, volMesh>& field,
    const bool applyOrientation
) const
{
    tmp<Field<Type> > tvalues(new Field<Type>(faceId_.size()));
    Field<Type>& values = tvalues();

    forAll(values, i)
    {
        label faceI = faceId_[i];
        label patchI = facePatchId_[i];
        if (patchI >= 0)
        {
            values[i] = field.boundaryField()[patchI][faceI];
        }
        else
        {
            FatalErrorIn
            (
                "fieldValues::faceSource::filterField"
                "("
                    "const GeometricField<Type, fvPatchField, volMesh>&"
                ") const"
            )   << type() << " " << name_ << ": "
                << sourceTypeNames_[source_] << "(" << sourceName_ << "):"
                << nl
                << "    Unable to process internal faces for volume field "
                << field.name() << nl << abort(FatalError);
        }
    }

    if (applyOrientation)
    {
        forAll(values, i)
        {
            values[i] *= faceSign_[i];
        }
    }

    return tvalues;
}


template<class Type>
Foam::tmp<Foam::Field<Type> > Foam::fieldValues::faceSource::filterField
(
    const GeometricField<Type, fvsPatchField, surfaceMesh>& field,
    const bool applyOrientation
) const
{
    tmp<Field<Type> > tvalues(new Field<Type>(faceId_.size()));
    Field<Type>& values = tvalues();

    forAll(values, i)
    {
        label faceI = faceId_[i];
        label patchI = facePatchId_[i];
        if (patchI >= 0)
        {
            values[i] = field.boundaryField()[patchI][faceI];
        }
        else
        {
            values[i] = field[faceI];
        }
    }

    if (applyOrientation)
    {
        forAll(values, i)
        {
            values[i] *= faceSign_[i];
        }
    }

    return tvalues;
}


// ************************************************************************* //

