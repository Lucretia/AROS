/*
    Copyright � 1995-2019, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Country data for Nihon (Japan)
*/

#include "country_locale.h"
#include <libraries/locale.h>

struct IntCountryPrefs japanPrefs =
{
    {
        /* Reserved */
        { 0, 0, 0, 0 },

        /* Country code (licence plate number), telephone code, measuring system */
        MAKE_ID('J', 0 , 0 , 0 ), 81, MS_ISO,

        /* Date time format, date format, time format */
        "%Yǯ%m\xB7\xEE%e\xC6\xFC %H\xBB\xFE%Mʬ%S\xC9\xC3",
        "%Yǯ%m\xB7\xEE%e\xC6\xFC",
        "%H\xBB\xFE%Mʬ%S\xC9\xC3",

        /* Short datetime, short date, short time formats */
        "%Yǯ%m\xB7\xEE%e\xC6\xFC %H\xBB\xFE%Mʬ%S\xC9\xC3",
        "%Yǯ%m\xB7\xEE%e\xC6\xFC",
        "%H\xBB\xFE%Mʬ",

        /* Decimal point, group separator, frac group separator */
        ".", ",", "",

        /* For grouping rules, see <libraries/locale.h> */

        /* Grouping, Frac Grouping */
        { 3 }, { 0 },

        /* Mon dec pt, mon group sep, mon frac group sep */
        ",", ".", "",

        /* Mon Grouping, Mon frac grouping */
        { 3 }, { 0 },

        /* Mon Frac digits, Mon IntFrac digits, then number of digits in
           the fractional part of the money value. Most countries that
           use dollars and cents, would have 2 for this value

           (As would many of those you don't).
        */
        2, 2,

        /* Currency symbol, Small currency symbol */
        "\xA5", "",

        /* Int CS, this is the ISO 4217 symbol, followed by the character to
           separate that symbol from the rest of the money. (\x00 for none).
        */
        "JPY",

        /* Mon +ve sign, +ve space sep, +ve sign pos, +ve cs pos */
        "", SS_NOSPACE, SP_PREC_ALL, CSP_SUCCEEDS,

        /* Mon -ve sign, -ve space sep, -ve sign pos, -ve cs pos */
        "-", SS_NOSPACE, SP_PREC_ALL, CSP_SUCCEEDS,

        /* Calendar type */
        CT_7SUN
    },
    "$VER: japan.country 44.0 (02.04.2019)",
    "Nihon",
    "Countries/Japan"
};
