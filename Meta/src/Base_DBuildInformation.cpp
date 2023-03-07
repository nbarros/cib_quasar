
/*  © Copyright CERN, 2015. All rights not expressly granted are reserved.
    Authors(from Quasar team): Piotr Nikiel

    This file is part of Quasar.

    Quasar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public Licence as published by
    the Free Software Foundation, either version 3 of the Licence.
    Quasar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public Licence for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Quasar.  If not, see <http://www.gnu.org/licenses/>.

    This file was completely generated by Quasar (additional info: using transform designToDeviceBaseBody.xslt)
    on 2018-11-09T17:06:14.522+01:00
 */




#include <Configuration.hxx>


#include <Base_DBuildInformation.h>







namespace Device
{



void Base_DBuildInformation::linkAddressSpace( AddressSpace::ASBuildInformation *addressSpaceLink)
{
    if (m_addressSpaceLink != 0)
    {
        /* signalize nice error from here. */
        abort();
    }
    else
        m_addressSpaceLink = addressSpaceLink;
}

/* add/remove */


//! Disconnects AddressSpace part from the Device logic, and does the same for all children
//! Returns number of unlinked objects including self
unsigned int Base_DBuildInformation::unlinkAllChildren ()
{
    unsigned int objectCounter=1;  // 1 is for self
    m_addressSpaceLink = 0;
    /* Fill up: call unlinkAllChildren on all children */

    return objectCounter;

}


/* find methods for children */






/* Constructor */
Base_DBuildInformation::Base_DBuildInformation ()
:m_addressSpaceLink(0)
{}

Base_DBuildInformation::~Base_DBuildInformation ()
{
    /* remove children */

}



std::list<DBuildInformation*> Base_DBuildInformation::s_orphanedObjects;




}


