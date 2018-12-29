//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_INTERVAL_H__
#define __LOMSE_INTERVAL_H__

#include "lomse_internal_model.h"        //for EKeySignature enum

#include <string>
using namespace std;

///@cond INTERNALS
namespace lomse
{
///@endcond


//---------------------------------------------------------------------------------------
// Interval types
/** @ingroup enumerations

    This enum describes valid interval types.

    @#include <lomse_pitch.h>
*/
enum EIntervalType
{
    k_double_diminished = 0,    ///< Double diminished
    k_diminished,               ///< Diminished
    k_minor,                    ///< Minor
    k_major,                    ///< Major
    k_perfect,                  ///< Perfect
    k_augmented,                ///< Augmented
    k_double_augmented,         ///< Double augmented
};

//---------------------------------------------------------------------------------------
/** Class %FIntval represents an interval. Intervals are defined using the properties
    of FPitch representation: An interval is just the difference between the two FPitches.

    Intervals can be added and substracted. For example, a perfect fifth minus a major
    third is a minor third (p5-M3 = m3)
    @code
    FIntval(5, k_perfect) - FIntval(3, k_major) == FIntval(3, k_minor)
    @endcode

    Intervals greater than one octave can also be build directly. For example,
    a major 10th:
    @code
    FIntval(10, k_major);
    @endcode
    But can also be computed by adding octaves to simple intervals. For example,
    a major 10th is a major third plus one octave (M3+p8 = M10), thus:
    @code
    FIntval(10, k_major) == FIntval(3, k_major) + FIntval(8, k_perfect)
    @endcode

    Intervals have sign: positive intervals are ascending and negative ones are
    descending but, appart from the sign, all other properties do not change.
    For example:
    @code
    FIntval intv == FIntval(3, k_major) - FIntval(8, k_perfect);

    // intv is a negative interval (minor 6th, descending). Thus, the following
    // assertions are true:

    assert(intv < 0);
    assert(intv.is_descending() == true);

    // But all other properties do not change by being descending:
    assert( intv.get_number() == 3 );
    assert( intv.get_type() == k_major );
    assert( intv.get_num_semitones() == 5 );
    assert( intv.get_code() == "M3" );
    @endcode


    Macros for all the simple intervals are also defined, for convenience. They are
    named 'k_interval_xxxx', where 'xxxx' is the abbreviated name. See get_code().
    @code
    FIntval(9, k_augmented) == FIntval(2, k_augmented) + k_interval_p8;
    FIntval(9, k_augmented) == k_interval_a2 + k_interval_p8;
    @endcode
*/
class FIntval
{
protected:
    int m_interval;

public:
    /** Constructor for building an interval from its number and its type.

        @param number The interval number. Positive for ascending intervals or negative
            for descending intervals.

        @param type A value from enum EIntervalType.

        Examples:

        @code
        FInval(3, k_minor);         //a third minor, ascending
        FInval(9, k_augmented);     //an augmented ninth, ascending
        FInval(-2, k_major);        //a major second, descending
        @endcode
    */
    FIntval(int number, EIntervalType type);

    /** Constructor for building an interval from its code. See get_code().

        @warning This constructor is only valid for intervals up to one octave. The maximum
          allowed is "da8" (double augmented octave).

        Examples:

        @code
        FInval("m3");                   //a third minor, ascending
        FInval("a6");                   //an augmented sixth, ascending
        FIntval("m7", k_descending);     //a minor seventh, descending
        @endcode
    */
    FIntval(const string& code, bool fDescending=false);

    /** Contructor for building an interval from an integer representing the
        interval value.

        @warning The interval value is not the interval number but an internal code
            that is returned when casting an FIntval object to an int.
    */
    FIntval(int value) : m_interval(value) {}

    /** Default empty constructor. Builds an invalid, null interval.    */
    FIntval();

    /** Operator to cast to an int. Negative for descending intervals.     */
    operator int() { return m_interval; }

        //get interval attributes
    /** Return string representing the interval code (its abbreviated name). Examples:
        "m2", "p4", "M3".

        The code is the same for ascending and descending intervals. To check for a
        descending interval use is_descending() method.

        The codes are formed by concatenating one or two characters with the interval
        number. The valid characters are as follows:

            dd  - double diminished
            d   - diminished
            m   - minor
            M   - Major
            p   - perfect
            a   - augmented
            da  - double augmented

        Thus:

            'm2' is a minor 2nd
            'p4' is a perfect fourth
            'M3' is a major third
            'da9' is a double augmented ninth
            'p15' is a two octaves

        Macros for all the simple intervals are also defined, for convenience. They are
        named 'k_interval_xxxx', where 'xxxx' is the code of the interval. For
        example:
        @code
            k_interval_m2 == FIntval(2, k_minor)        //minor 2nd
            k_interval_p4 == FIntval(4, k_perfect)      //perfect fourth
            k_interval_M3 == FIntval(3, k_major)        //major third
            k_interval_da9 == FIntval(9, k_double_augmented)    //double augmented ninth
            k_interval_p8 == FIntval(8, k_perfect)      //perfect octave
        @endcode
   */
    string get_code();

    /** Returns the interval number: 1 for unison, 2 for second, 3 for third, ... ,
        8 for octave, 9 for ninth, ..., 15 for two octaves, and so on.

        The number is the same for ascending and descending intervals. To check for a
        descending interval use is_descending() method.
    */
    int get_number();

    /** Returns the interval type (diminished, major, etc.) as a value from enum
        EIntervalType.

        The type is the same for ascending and descending intervals. To check for a
        descending interval use is_descending() method.
    */
    EIntervalType get_type();

    /** Returns the number of semitones implied by this interval.

        The numebr of semitones is always positive. It doesn't matter if the interval
        is ascending or descending. To check for a
        descending interval use is_descending() method.
    */
    int get_num_semitones();

    /** Returns @true if the interval is descending and @false when it is unison or
        ascending.
    */
    inline bool is_descending() { return m_interval < 0; }

    /** Transforms the interval in a descending interval. If it is already descending
        this method does nothing.
    */
    inline void make_descending() { if (m_interval > 0) m_interval = -m_interval; }

    /** Returns @true if the interval is ascending or unison and @false when it is
        descending.
    */
    inline bool is_ascending() { return m_interval >= 0; }

    /** Transforms the interval in an ascending interval. If it is already ascending
        this method does nothing.
    */
    inline void make_ascending() { m_interval = abs(m_interval); }

    ///@{
    /// Comparison operators
    bool operator ==(FIntval intv) { return m_interval == int(intv); }
    bool operator !=(FIntval intv) { return m_interval != int(intv); }
    bool operator < (FIntval intv) { return m_interval < int(intv); }
    bool operator > (FIntval intv) { return m_interval > int(intv); }
    bool operator <= (FIntval intv) { return m_interval <= int(intv); }
    bool operator >= (FIntval intv) { return m_interval >= int(intv); }
    ///@}

    ///@{
    /// Add/substract operators
    FIntval operator -(FIntval intv) { return FIntval(m_interval - int(intv)); }
    FIntval operator +(FIntval intv) { return FIntval(m_interval + int(intv)); }
    FIntval operator -=(FIntval intv) {
        m_interval -= int(intv);
        return FIntval(m_interval);
    }
    FIntval operator +=(FIntval intv) {
        m_interval += int(intv);
        return FIntval(m_interval);
    }
    ///@}

protected:

};

#define k_descending    true
#define k_ascending     false



    //unison
#define k_interval_p1   FIntval(0)
#define k_interval_a1   FIntval(1)
#define k_interval_da1  FIntval(2)
    //second
#define k_interval_dd2  FIntval(3)
#define k_interval_d2   FIntval(4)
#define k_interval_m2   FIntval(5)
#define k_interval_M2   FIntval(6)
#define k_interval_a2   FIntval(7)
#define k_interval_da2  FIntval(8)
    //third
#define k_interval_dd3  FIntval(9)
#define k_interval_d3   FIntval(10)
#define k_interval_m3   FIntval(11)
#define k_interval_M3   FIntval(12)
#define k_interval_a3   FIntval(13)
#define k_interval_da3  FIntval(14)
    //fourth
#define k_interval_dd4  FIntval(15)
#define k_interval_d4   FIntval(16)
#define k_interval_p4   FIntval(17)
#define k_interval_a4   FIntval(18)
#define k_interval_da4  FIntval(19)
    //fifth
#define k_interval_dd5  FIntval(21)
#define k_interval_d5   FIntval(22)
#define k_interval_p5   FIntval(23)
#define k_interval_a5   FIntval(24)
#define k_interval_da5  FIntval(25)
    //sixth
#define k_interval_dd6  FIntval(26)
#define k_interval_d6   FIntval(27)
#define k_interval_m6   FIntval(28)
#define k_interval_M6   FIntval(29)
#define k_interval_a6   FIntval(30)
#define k_interval_da6  FIntval(31)
    //seventh
#define k_interval_dd7  FIntval(32)
#define k_interval_d7   FIntval(33)
#define k_interval_m7   FIntval(34)
#define k_interval_M7   FIntval(35)
#define k_interval_a7   FIntval(36)
#define k_interval_da7  FIntval(37)
    //octave
#define k_interval_dd8  FIntval(38)
#define k_interval_d8   FIntval(39)
#define k_interval_p8   FIntval(40)
#define k_interval_a8   FIntval(41)
#define k_interval_da8  FIntval(42)
    //9th
#define k_interval_dd9  FIntval(43)
#define k_interval_d9   FIntval(44)
#define k_interval_m9   FIntval(45)
#define k_interval_M9   FIntval(46)
#define k_interval_a9   FIntval(47)
#define k_interval_da9  FIntval(48)
    //10th
#define k_interval_dd10 FIntval(49)
#define k_interval_d10  FIntval(50)
#define k_interval_m10  FIntval(51)
#define k_interval_M10  FIntval(52)
#define k_interval_a10  FIntval(53)
#define k_interval_da10 FIntval(54)
    //11h
#define k_interval_dd11 FIntval(55)
#define k_interval_d11  FIntval(56)
#define k_interval_p11  FIntval(57)
#define k_interval_a11  FIntval(58)
#define k_interval_da11 FIntval(59)
    //12h
#define k_interval_dd12 FIntval(61)
#define k_interval_d12  FIntval(62)
#define k_interval_p12  FIntval(63)
#define k_interval_a12  FIntval(64)
#define k_interval_da12 FIntval(65)
    //13h
#define k_interval_dd13 FIntval(66)
#define k_interval_d13  FIntval(67)
#define k_interval_m13  FIntval(68)
#define k_interval_M13  FIntval(69)
#define k_interval_a13  FIntval(70)
#define k_interval_da13 FIntval(71)
    //14h
#define k_interval_dd14 FIntval(72)
#define k_interval_d14  FIntval(73)
#define k_interval_m14  FIntval(74)
#define k_interval_M14  FIntval(75)
#define k_interval_a14  FIntval(76)
#define k_interval_da14 FIntval(77)
    //two octaves
#define k_interval_dd15 FIntval(78)
#define k_interval_d15  FIntval(79)
#define k_interval_p15  FIntval(80)

// Define a 'null interval'. It is usefull to signal special situations such as
// 'no interval defined', 'end of list', etc.
#define k_interval_null  FIntval(-15151)



} //namespace lomse

#endif    //__LOMSE_INTERVAL_H__
