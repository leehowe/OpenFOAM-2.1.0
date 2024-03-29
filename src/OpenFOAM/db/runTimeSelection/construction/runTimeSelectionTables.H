/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2004-2011 OpenCFD Ltd.
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

Description
    Macros to ease declaration of run-time selection tables.

    declareRunTimeSelectionTable is used to create a run-time selection table
    for a base-class which holds constructor pointers on the table.

    declareRunTimeNewSelectionTable is used to create a run-time selection
    table for a derived-class which holds "New" pointers on the table.

\*---------------------------------------------------------------------------*/

#include "token.H"

#ifndef runTimeSelectionTables_H
#define runTimeSelectionTables_H

#include "autoPtr.H"
#include "HashTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// external use:
// ~~~~~~~~~~~~~
// declare a run-time selection:
#define declareRunTimeSelectionTable\
(autoPtr,baseType,argNames,argList,parList)                                   \
                                                                              \
    /* Construct from argList function pointer type */                        \
    typedef autoPtr< baseType > (*argNames##ConstructorPtr)argList;           \
                                                                              \
    /* Construct from argList function table type */                          \
    typedef HashTable< argNames##ConstructorPtr, word, string::hash >         \
        argNames##ConstructorTable;                                           \
                                                                              \
    /* Construct from argList function pointer table pointer */               \
    static argNames##ConstructorTable* argNames##ConstructorTablePtr_;        \
                                                                              \
    /* Table constructor called from the table add function */                \
    static void construct##argNames##ConstructorTables();                     \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    static void destroy##argNames##ConstructorTables();                       \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    template< class baseType##Type >                                          \
    class add##argNames##ConstructorToTable                                   \
    {                                                                         \
    public:                                                                   \
                                                                              \
        static autoPtr< baseType > New argList                                \
        {                                                                     \
            return autoPtr< baseType >(new baseType##Type parList);           \
        }                                                                     \
        add##argNames##ConstructorToTable                                     \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->insert(lookup, New);              \
        }                                                                     \
                                                                              \
        ~add##argNames##ConstructorToTable()                                  \
        {                                                                     \
            destroy##argNames##ConstructorTables();                           \
        }                                                                     \
    };                                                                        \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    /* Remove only the entry (not the table) upon destruction */              \
    template< class baseType##Type >                                          \
    class addRemovable##argNames##ConstructorToTable                          \
    {                                                                         \
        /* retain lookup name for later removal */                            \
        const word& lookup_;                                                  \
                                                                              \
    public:                                                                   \
                                                                              \
        static autoPtr< baseType > New argList                                \
        {                                                                     \
            return autoPtr< baseType >(new baseType##Type parList);           \
        }                                                                     \
                                                                              \
        addRemovable##argNames##ConstructorToTable                            \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        :                                                                     \
            lookup_(lookup)                                                   \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->set(lookup, New);                 \
        }                                                                     \
                                                                              \
        ~addRemovable##argNames##ConstructorToTable()                         \
        {                                                                     \
            if (argNames##ConstructorTablePtr_)                               \
            {                                                                 \
                argNames##ConstructorTablePtr_->erase(lookup_);               \
            }                                                                 \
        }                                                                     \
    };


//add by xiaow:begin, argList1 for interface1, argList2 for interface2
#define declareRunTimeSelectionTable2\
(autoPtr,baseType,argNames,argList1,argList2, parList1,parList2)                        \
                                                                              \
    /* Construct from argList function pointer type */                        \
    typedef autoPtr< baseType > (*argNames##ConstructorPtr)argList2;          \
                                                                              \
    typedef autoPtr< baseType > (*argNames##ConstructorPtr1)argList1;         \
                                                                              \
    /* Construct from argList function table type */                          \
    typedef HashTable< argNames##ConstructorPtr, word, string::hash >         \
        argNames##ConstructorTable;                                           \
                                                                              \
    /* Construct from argList function table type */                          \
    typedef HashTable< argNames##ConstructorPtr1, word, string::hash >        \
        argNames##ConstructorTable1;                                          \
                                                                              \
    /* Construct from argList function pointer table pointer */               \
    static argNames##ConstructorTable1* argNames##ConstructorTablePtr_1;      \
                                                                              \
    /* Construct from argList function pointer table pointer */               \
    static argNames##ConstructorTable* argNames##ConstructorTablePtr_;        \
                                                                              \
    /* Table constructor called from the table add function */                \
    static void construct##argNames##ConstructorTables();                     \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    static void destroy##argNames##ConstructorTables();                       \
                                                                              \
    /* Table constructor called from the table add function */                \
    static void construct##argNames##ConstructorTables1();                    \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    static void destroy##argNames##ConstructorTables1();                      \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    template< class baseType##Type >                                          \
    class add##argNames##ConstructorToTable                                   \
    {                                                                         \
    public:                                                                   \
                                                                              \
                                                                              \
        static autoPtr< baseType > New argList2                               \
        {                                                                     \
            return autoPtr< baseType >(new baseType##Type parList2);           \
        }                                                                     \
                                                                              \
        add##argNames##ConstructorToTable                                     \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->insert(lookup, New);              \
        }                                                                     \
                                                                              \
        ~add##argNames##ConstructorToTable()                                  \
        {                                                                     \
            destroy##argNames##ConstructorTables();                           \
        }                                                                     \
    };                                                                        \
                                                                              \
       /* Class to add constructor from argList to table */                   \
    template< class baseType##Type >                                          \
    class add##argNames##ConstructorToTableAppend                             \
    {                                                                         \
    public:                                                                   \
                                                                              \
        static autoPtr< baseType > New argList1                               \
        {                                                                     \
            return autoPtr< baseType >(new baseType##Type parList1);          \
        }                                                                     \
                                                                              \
        add##argNames##ConstructorToTableAppend                               \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        {                                                                     \
            construct##argNames##ConstructorTables1();                        \
            argNames##ConstructorTablePtr_1->insert(lookup, New);             \
        }                                                                     \
                                                                              \
        ~add##argNames##ConstructorToTableAppend()                        \
        {                                                                     \
            destroy##argNames##ConstructorTables1();                          \
        }                                                                     \
    };                                                                        \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    /* Remove only the entry (not the table) upon destruction */              \
    template< class baseType##Type >                                          \
    class addRemovable##argNames##ConstructorToTable                          \
    {                                                                         \
        /* retain lookup name for later removal */                            \
        const word& lookup_;                                                  \
                                                                              \
    public:                                                                   \
                                                                              \
                                                                              \
        static autoPtr< baseType > New argList2                               \
        {                                                                     \
            return autoPtr< baseType >(new baseType##Type parList2);           \
        }                                                                     \
                                                                              \
        addRemovable##argNames##ConstructorToTable                            \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        :                                                                     \
            lookup_(lookup)                                                   \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->set(lookup, New);                 \
        }                                                                     \
                                                                              \
        ~addRemovable##argNames##ConstructorToTable()                         \
        {                                                                     \
            if (argNames##ConstructorTablePtr_)                               \
            {                                                                 \
                argNames##ConstructorTablePtr_->erase(lookup_);               \
            }                                                                 \
        }                                                                     \
    };
//add by xiaow:end



// external use:
// ~~~~~~~~~~~~~
// declare a run-time selection for derived classes:
#define declareRunTimeNewSelectionTable\
(autoPtr,baseType,argNames,argList,parList)                                   \
                                                                              \
    /* Construct from argList function pointer type */                        \
    typedef autoPtr< baseType > (*argNames##ConstructorPtr)argList;           \
                                                                              \
    /* Construct from argList function table type */                          \
    typedef HashTable< argNames##ConstructorPtr, word, string::hash >         \
        argNames##ConstructorTable;                                           \
                                                                              \
    /* Construct from argList function pointer table pointer */               \
    static argNames##ConstructorTable* argNames##ConstructorTablePtr_;        \
                                                                              \
    /* Table constructor called from the table add function */                \
    static void construct##argNames##ConstructorTables();                     \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    static void destroy##argNames##ConstructorTables();                       \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    template< class baseType##Type >                                          \
    class add##argNames##ConstructorToTable                                   \
    {                                                                         \
    public:                                                                   \
                                                                              \
        static autoPtr< baseType > New##baseType argList                      \
        {                                                                     \
            return autoPtr< baseType >(baseType##Type::New parList.ptr());    \
        }                                                                     \
                                                                              \
        add##argNames##ConstructorToTable                                     \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->insert                            \
            (                                                                 \
                lookup,                                                       \
                New##baseType                                                 \
            );                                                                \
        }                                                                     \
                                                                              \
        ~add##argNames##ConstructorToTable()                                  \
        {                                                                     \
            destroy##argNames##ConstructorTables();                           \
        }                                                                     \
    };                                                                        \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    template< class baseType##Type >                                          \
    class addRemovable##argNames##ConstructorToTable                          \
    {                                                                         \
        /* retain lookup name for later removal */                            \
        const word& lookup_;                                                  \
                                                                              \
    public:                                                                   \
                                                                              \
        static autoPtr< baseType > New##baseType argList                      \
        {                                                                     \
            return autoPtr< baseType >(baseType##Type::New parList.ptr());    \
        }                                                                     \
                                                                              \
        addRemovable##argNames##ConstructorToTable                            \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        :                                                                     \
            lookup_(lookup)                                                   \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->set                               \
            (                                                                 \
                lookup,                                                       \
                New##baseType                                                 \
            );                                                                \
        }                                                                     \
                                                                              \
        ~addRemovable##argNames##ConstructorToTable()                         \
        {                                                                     \
            if (argNames##ConstructorTablePtr_)                               \
            {                                                                 \
                argNames##ConstructorTablePtr_->erase(lookup_);               \
            }                                                                 \
        }                                                                     \
    };

//add by xiaow:begin
// internal use:
// constructor aid
#define defineRunTimeSelectionTableConstructor2\
(baseType,argNames)                                                           \
                                                                              \
    /* Table constructor called from the table add function */                \
    void baseType::construct##argNames##ConstructorTables()                   \
    {                                                                         \
        static bool constructed = false;                                      \
        if (!constructed)                                                     \
        {                                                                     \
            constructed = true;                                               \
            baseType::argNames##ConstructorTablePtr_                          \
                = new baseType::argNames##ConstructorTable;                   \
        }                                                                     \
    }                                                                         \
    void baseType::construct##argNames##ConstructorTables1()                   \
    {                                                                         \
        static bool constructed = false;                                      \
        if (!constructed)                                                     \
        {                                                                     \
            constructed = true;                                               \
            baseType::argNames##ConstructorTablePtr_1                          \
                = new baseType::argNames##ConstructorTable1;                   \
        }                                                                     \
    }   
//add by xiaow:end


#define defineRunTimeSelectionTableConstructor\
(baseType,argNames)                                                           \
                                                                              \
    /* Table constructor called from the table add function */                \
    void baseType::construct##argNames##ConstructorTables()                   \
    {                                                                         \
        static bool constructed = false;                                      \
        if (!constructed)                                                     \
        {                                                                     \
            constructed = true;                                               \
            baseType::argNames##ConstructorTablePtr_                          \
                = new baseType::argNames##ConstructorTable;                   \
        }                                                                     \
    }                                                                         



//add by xiaow:begin
// internal use:
// destructor aid
#define defineRunTimeSelectionTableDestructor2\
(baseType,argNames)                                                           \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    void baseType::destroy##argNames##ConstructorTables()                     \
    {                                                                         \
        if (baseType::argNames##ConstructorTablePtr_)                         \
        {                                                                     \
            delete baseType::argNames##ConstructorTablePtr_;                  \
            baseType::argNames##ConstructorTablePtr_ = NULL;                  \
        }                                                                     \
    }                                                                         \
    void baseType::destroy##argNames##ConstructorTables1()                    \
    {                                                                         \
        if (baseType::argNames##ConstructorTablePtr_1)                        \
        {                                                                     \
            delete baseType::argNames##ConstructorTablePtr_1;                 \
            baseType::argNames##ConstructorTablePtr_1 = NULL;                 \
        }                                                                     \
    }
//add by xiaow:end


#define defineRunTimeSelectionTableDestructor\
(baseType,argNames)                                                           \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    void baseType::destroy##argNames##ConstructorTables()                     \
    {                                                                         \
        if (baseType::argNames##ConstructorTablePtr_)                         \
        {                                                                     \
            delete baseType::argNames##ConstructorTablePtr_;                  \
            baseType::argNames##ConstructorTablePtr_ = NULL;                  \
        }                                                                     \
    }                                                                         


//add by xiaow:begin
// internal use:
// create pointer to hash-table of functions
#define defineRunTimeSelectionTablePtr2\
(baseType,argNames)                                                           \
                                                                              \
    /* Define the constructor function table */                               \
    baseType::argNames##ConstructorTable*                                     \
        baseType::argNames##ConstructorTablePtr_ = NULL;                       \
       /* Define the constructor function table */                            \
    baseType::argNames##ConstructorTable1*                                     \
        baseType::argNames##ConstructorTablePtr_1 = NULL
//add by xiaow:end

#define defineRunTimeSelectionTablePtr\
(baseType,argNames)                                                           \
                                                                              \
    /* Define the constructor function table */                               \
    baseType::argNames##ConstructorTable*                                     \
        baseType::argNames##ConstructorTablePtr_ = NULL                       

// not much in use:
#define defineTemplateRunTimeSelectionTablePtr(baseType,argNames)             \
                                                                              \
    /* Define the constructor function table */                               \
    typename baseType::argNames##ConstructorTable*                            \
        baseType::argNames##ConstructorTablePtr_ = NULL


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// external use:
// ~~~~~~~~~~~~~
// define run-time selection table
#define defineRunTimeSelectionTable\
(baseType,argNames)                                                           \
                                                                              \
    defineRunTimeSelectionTablePtr(baseType,argNames);                        \
    defineRunTimeSelectionTableConstructor(baseType,argNames);                \
    defineRunTimeSelectionTableDestructor(baseType,argNames)

//add by xiaow:begin
#define defineRunTimeSelectionTable2\
(baseType,argNames)                                                           \
                                                                              \
    defineRunTimeSelectionTablePtr2(baseType,argNames);                       \
    defineRunTimeSelectionTableConstructor2(baseType,argNames);               \
    defineRunTimeSelectionTableDestructor2(baseType,argNames)
//add by xiaow:end


// external use:
// ~~~~~~~~~~~~~
// define run-time selection table for template classes
// use when baseType doesn't need a template argument (eg, is a typedef)
#define defineTemplateRunTimeSelectionTable\
(baseType,argNames)                                                           \
                                                                              \
    template<>                                                                \
    defineRunTimeSelectionTablePtr(baseType,argNames);                        \
    template<>                                                                \
    defineRunTimeSelectionTableConstructor(baseType,argNames);                \
    template<>                                                                \
    defineRunTimeSelectionTableDestructor(baseType,argNames)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// internal use:
// constructor aid
// use when baseType requires the Targ template argument
#define defineTemplatedRunTimeSelectionTableConstructor\
(baseType,argNames,Targ)                                                      \
                                                                              \
    /* Table constructor called from the table add function */                \
    void baseType< Targ >::construct##argNames##ConstructorTables()           \
    {                                                                         \
        static bool constructed = false;                                      \
        if (!constructed)                                                     \
        {                                                                     \
            constructed = true;                                               \
            baseType< Targ >::argNames##ConstructorTablePtr_                  \
                = new baseType< Targ >::argNames##ConstructorTable;           \
        }                                                                     \
    }                                                                         

// internal use:
// destructor aid
// use when baseType requires the Targ template argument
#define defineTemplatedRunTimeSelectionTableDestructor\
(baseType,argNames,Targ)                                                      \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    void baseType< Targ >::destroy##argNames##ConstructorTables()             \
    {                                                                         \
        if (baseType< Targ >::argNames##ConstructorTablePtr_)                 \
        {                                                                     \
            delete baseType< Targ >::argNames##ConstructorTablePtr_;          \
            baseType< Targ >::argNames##ConstructorTablePtr_ = NULL;          \
        }                                                                     \
    }                                                                         


// internal use:
// create pointer to hash-table of functions
// use when baseType requires the Targ template argument
#define defineTemplatedRunTimeSelectionTablePtr\
(baseType,argNames,Targ)                                                      \
                                                                              \
    /* Define the constructor function table */                               \
    baseType< Targ >::argNames##ConstructorTable*                             \
        baseType< Targ >::argNames##ConstructorTablePtr_ = NULL               


// external use:
// ~~~~~~~~~~~~~
// define run-time selection table for template classes
// use when baseType requires the Targ template argument
#define defineTemplatedRunTimeSelectionTable\
(baseType,argNames,Targ)                                                      \
                                                                              \
    template<>                                                                \
    defineTemplatedRunTimeSelectionTablePtr(baseType,argNames,Targ);          \
    template<>                                                                \
    defineTemplatedRunTimeSelectionTableConstructor(baseType,argNames,Targ);  \
    template<>                                                                \
    defineTemplatedRunTimeSelectionTableDestructor(baseType,argNames,Targ)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
