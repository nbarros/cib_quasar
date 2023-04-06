
/*  © Copyright CERN, 2015. All rights not expressly granted are reserved.

    The stub of this file was generated by quasar (https://github.com/quasar-team/quasar/)

    Quasar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public Licence as published by
    the Free Software Foundation, either version 3 of the Licence.
    Quasar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public Licence for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Quasar.  If not, see <http://www.gnu.org/licenses/>.


 */


#ifndef __DIoLCIB__H__
#define __DIoLCIB__H__

#include <Base_DIoLCIB.h>

namespace Device
{

class
    DIoLCIB
    : public Base_DIoLCIB
{

public:
    /* sample constructor */
    explicit DIoLCIB (
        const Configuration::IoLCIB& config,
        Parent_DIoLCIB* parent
    ) ;
    /* sample dtr */
    ~DIoLCIB ();

    /* delegators for
    cachevariables and sourcevariables */


    /* delegators for methods */

private:
    /* Delete copy constructor and assignment operator */
    DIoLCIB( const DIoLCIB& other );
    DIoLCIB& operator=(const DIoLCIB& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:

    bool is_ready() {return m_is_ready;}

private:
    bool m_is_ready;


};

}

#endif // __DIoLCIB__H__