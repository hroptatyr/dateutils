---
title: dateutils changelog
layout: subpage
project: dateutils
logo: dateutils_logo_120.png
---

Changelog in reverse order
==========================

v0.4.4
------
Summary: v0.4.4 of dateutils

This is dateutils v0.4.4.

This is a bugfix release.

Incompatible changes:

- suffix `m` is no longer accepted as a synonym for `mo`
    with date-only input, issue #76

Bugfixes:

- expose BSD routines (fgetln()) to yuck
- for dates passed to dateround(1) that coincidentally
    match the roundspecs do read them as dates
- wrong timestamps read via -i %s signal error
- facilitate bmake build
- time rounding on date-only input keeps the date unchanged
- dateseq's short-cut iterator (2 date arguments) does not
    interfere with the 3-argument version
- adding 0 date or time units does not change the summand
- datezone on times (without date) will return times
- zones are singletons now, opened and closed only once

See info page examples and/or README.

v0.4.3
------
Summary: v0.4.3 of dateutils

This is dateutils v0.4.3.

This is a feature release.

Features:

- base expansion works for times now

Bugfixes:

- durations in months weeks and days are calculated
    like durations in months and days, consistency
- am and pm indicators in inputs are handled properly
- military midnights decay when not printed in full

See info page examples and/or README.

v0.4.2
------
Summary: v0.4.2 of dateutils

This is dateutils v0.4.2.

This is a feature release.

Features:

- allow `%-` specifiers to turn off padding (as GNU date does)
- support matlab day numbers, as `mdn` or `matlab`

Bugfixes:

- fix build issue on FBSD 11
- allow zones to transition at INT_MAX (mapped to `never`)

See info page examples and/or README.

v0.4.1
------
Summary: v0.4.1 of dateutils

This is dateutils v0.4.1.

This is a bugfix release.

Bugfixes:

- bug/50, keep end-of-term (ultimo) property in dateseq(1)
- allow today/now for ywd dates in 1 parameter dateseq(1)
- allow different calendars for start and end date in dateseq(1)
- round time in dateround(1) in zone then convert --from-zone
- allow SPC padded numerals in %d input format

See info page examples and/or README.

v0.4.0
------
Summary: v0.4.0 of dateutils

This is dateutils v0.4.0.

This is a bugfix release.

Bugfixes:

- document %g/%G format specifiers
- bug/42, accept NUL characters in input
- bug/45, correctly display Gregorian and ISO week dates in one line
- bug/46, adhere to ISO group's official formatting of week dates
- bug/47, allow rounding of Epoch based timestamps
- bug/48, avoid crash for void input to strptime(3)

Features:

- datetest supports `--isvalid` to conditionalise on date/time parsing
- all tools support `--from-locale` to parse localised input
- tools with output formatting support `--locale` to format output
  according to locale

See info page examples and/or README.


v0.3.5
------
Summary: v0.3.5 of dateutils

This is dateutils v0.3.5.

This is a bugfix release.

Bugfixes:

- bug/40, distinguish between numerals-only dates and durations
- tests will succeed independent of current date

See info page examples and/or README.


v0.3.4
------
Summary: v0.3.4 of dateutils

This is dateutils v0.3.4.

This is a bugfix release.

Bugfixes:

- bug/39, MacOSX endianness detection
- bug/38, ddiff day-only durations on date/times
- dadd +2m bug is fixed, regression dtadd.049.clit/dtadd.050.clit

See info page examples and/or README.


v0.3.3
------
Summary: v0.3.3 of dateutils

This is dateutils v0.3.3.

This is a feature release.

Features:

- to clarify purpose and avoid name clashes prefix binaries with date-
    This results in: dateadd dateconv datediff dategrep dateround
    dateseq datesort datetest and datezone
- provide compatibilty through configure switch --with-old-names
- provide single digit years through %_y
- allow rounding of ISO-week dates (ywd) to week numbers

Bugfixes:

- dashes behind a date do not count as tz indicator
- UTC/TAI/GPS special coordinated zones work on systems without
    leap second support

See info page examples and/or README.

v0.3.2
------
Summary: v0.3.2 of dateutils

This is dateutils v0.3.2.

This is a bugfix release.

Bugfixes:

- out of range minutes will be discarded
- bug 30 (malicious input crashes dconv) has been fixed

Features:

- military midnight stamps are supported (T24:00:00)
- 8601 ordinal dates (year + doy) are recognised directly (`-f yd`)
- strptime(1) can behave in a locale-dependent way

See info page examples and/or README.

v0.3.1
------
Summary: v0.3.1 of dateutils

This is dateutils v0.3.1.

This is a bugfix release.

Bugfixes:

- octave/matlab code is distributed fully
- negative durations with refined units are minus-signed only once
- ddiff is entirely anticommutative now
- tests don't fail if zones don't exist on the build system
- dseq with empty ranges will no longer produce output (just as seq(1))
- arbitrary integers are not interpreted as time anymore
- when converting from zone info properly clear zone difference for %Z
- dseq(1) will automatically resort to +1mo and +1y iterators for
    wildcarded ymd dates
- dadd(1)'ing ywd dates with output as ymd works properly now

Features:

- ddiff can output nanosecond diffs
- automatic fix-up of dates is documented now
- parser errors and fix ups are reported through return code 2
- dseq with no `-f|--format` stays in the calendric system of the start
    value instead of converting all output to ymd

See info page examples and/or README.

v0.3.0
------
Summary: v0.3.0 of dateutils

This is dateutils v0.3.0.

This is a feature release.

Features:

- dgrep supports time zones both for the expression and the input
- timezones can be specified by alternative codes and [tzmap files][1]
- new tool dzone to inspect date/times in multiple timezones in bulk
- new tool dsort to sort input chronologically
- gengetopt and help2man maintainer dependencies removed
- lilian/julian inputs via `-i ldn` and `-i jdn`
- ymcw dates now follow ISO 8601 in using 07 to denote Sunday

Bugfixes:

- ddiff takes differences between a unix epoch stamp and a date/time
- zone converter assigns correct sign to zone difference when using %Z
- weekdays are properly calculated from epoch stamps (issue 24)

See info page examples and/or README.

  [1]: http://www.fresse.org/dateutils/tzmaps.html

v0.2.7
------
Summary: v0.2.7 of dateutils

This is dateutils v0.2.7.

This is a feature release.

Features:

- dgrep supports `-v|--invert-match` like grep
- output specifier %G is supported for compatibility with POSIX
- ddiff calculates year-day differences
- ddiff calculates ISO-week date differences
- ddiff output can be zero and space padded through 0 and SPC modifier
- zoneinfo database on AIX >= 6.1 is taken into account

Bug fixes:

- ddiff can calculate full year differences, [issue 21][1] fixed
- dseq now accepts %W, %V output formats, [issue 22][2] fixed
- builds with clang >= 3.3 work again, [clang bug 18028][3]

See info page examples and/or README.

  [1]: https://github.com/hroptatyr/dateutils/issues/21
  [2]: https://github.com/hroptatyr/dateutils/issues/22
  [3]: http://llvm.org/bugs/show_bug.cgi?id=18028

v0.2.6
------
Summary: v0.2.6 of dateutils

This is dateutils v0.2.6.

This is a bug fix release.

Bug fixes:

- issue 19, -q|--quiet no longer sends some commands into an inf-loop
- netbsd test failures are fixed (due to missing leap seconds)
- AIX builds are supported (getopt_long() is part of the code now)
- internally the test harness is migrated to the cli-testing tool
    this fixes an issue when tests are run in directories with odd names
    (spaces, dollar signs, etc. in the path name)

See info page examples and/or README.

v0.2.5
------
Summary: v0.2.5 of dateutils

This is dateutils v0.2.5.

This is a bug fix release.

Bug fixes:

- issue 18, long inputs to a short specifier string will yield an error
- consume zone specs (a la +1200) in the input via %Z specifier
- ddiff's stdin stamps will undergo conversion according to --from-zone
- clean up dist for inclusion in debian

See info page examples and/or README.

v0.2.4
------
Summary: v0.2.4 of dateutils

This is dateutils v0.2.4.

This is a bug fix release.

Features:

- added special output format `jdn` and `ldn` for julian/lilian day number
- multiple occurrences of date/times within one line are now all processed
    rather than only the first occurrence
- zone difference specifier (%Z) is supported for parsing and printing
- matlab zone converter tzconv has been added

Bug fixes:

- building with icc 13 works now
- many gcc warnings are fixed

See info page examples and/or README.

v0.2.3
------
Summary: v0.2.3 of dateutils

This is dateutils v0.2.3.

This is a bug fix and feature release.

Features:

- ISO 8601 week dates are now first class objects (of type DT_YWD)
- introduce %rY specifier to denote years in calendars that deviate from
    the Gregorian year
- dgrep accepts short-hand inputs (today, now, etc.) and also inputs as
    specified by -i

Bug fixes:

- dadd'ing months and years to YMCW dates works now
- zoneinfo files with only transitions in the past are handled properly
    (bug #10)
- dseq with just 1 argument is working properly (story #36051287)

See info page examples and/or README.

v0.2.2
------
Summary: v0.2.2 of dateutils

This is dateutils v0.2.2.

This is a bug fix and feature release.

Features:

- Olson's zoneinfo database files are checked for at configure time
- leap-aware calculations use shipped leapseconds file
- ddiff and dadd can take leap-second transitions into account

Bug fixes:

- issue 7: ddiff without arguments does not segfault
- issue 8: dadd copes with huge summands
- issue 9: dadd stumbles on ymcw dates
- bug 33104651: bday negative difference A > B ddiff A B -f %db is wrong

See info page examples and/or README.

v0.2.1
------
Summary: v0.2.1 of dateutils

This is dateutils v0.2.1.

This is a bug fix and feature release.

The dadd tool now supports mass-adding durations (from stdin).
The ddiff tool is now time zone aware.
A new tool dround is added to round dates or times or date-times to the
next occurrence of what's given as round-spec.

Bug fixes:

- issue 7: ddiff without arguments does not segfault
- issue 8: dadd copes with huge summands

See info page examples and/or README.

v0.2.0
------
Summary: v0.2.0 of dateutils

This is dateutils v0.2.0.

This is a feature release.

The distinction between binaries for date, time and date-time processing
is cleared up by a unified set of tools, prefixed with `d`.

Thus:
dadd + tadd -> dadd
dconv + tconv + dtconv -> dconv
ddiff + tdiff -> ddiff
dgrep + tgrep -> dgrep
dseq + tseq -> dseq
dtest + ttest -> dtest

Furthermore, all tools now fully cope with dates, times and date-times.
Virtual timezones have been added (use `GPS` or `TAI`).

See info page examples and/or README.

v0.1.10
-------
Summary: v0.1.10 of dateutils

This is dateutils v0.1.10.

This is a bug fix release.

- account for big-endian machines
- GNUisms (mempcpy() and getline()) are removed
- inf-loop in tseq is fixed (bug #6)
- nanoseconds are preserved upon time zone conversion

See info page examples and/or README.

v0.1.9
------
Summary: v0.1.9 of dateutils

This is dateutils v0.1.9.

This is a bug fix release.

The code for date addition is refactored, with it a new duration type is
introduced, DT_MD, to capture larger month and day summands.

See info page examples and/or README.

v0.1.8
------
Summary: v0.1.8 of dateutils

This is dateutils v0.1.8.

This is a bug fix release.

A bit fiddling bug gave erroneous results in `dconv now`.

Furtherly, date expressions (for dgrep et al.) can now be arbitrarily
joined with conjunctions (&&) and disjunctions (||) as well as negations
(!).

See info page examples and/or README.

v0.1.7
------
Summary: v0.1.7 of dateutils

This is dateutils v0.1.7.

This is a bug fix release.

Most notably, sloppy date arithmetics have been replaced by correct
ones, e.g. 2100 is not longer a leap year and the 31st of Feb is
instantly corrected to 28/29 Feb.

Furtherly, the unmaintainable idea of dedicated duration types has been
replaced with overloaded dt_d_s types with the side-effect that
adding days or business days to dates now works and ymcw dates can be
properly compared.

See info page examples and/or README.

v0.1.6
------
Summary: v0.1.6 of dateutils

This is dateutils v0.1.6.

This is a feature release.

The dcal and tcal binaries are renamed to dconv and tconv respectively,
there has been a naming conflict with the tcal binary from the gcal
package.  Thanks to ulm (https://github.com/ulm) for pointing this out.

Changes in behaviour:
Furthermore, many numerical specifiers now cope with the `th` flag to
denote ordinals: `%dth %b %Y` applied to 2011-10-03 will yield
`3rd Oct 2011`.

business days can be denoted by suffixing them with `b` both in the
input and the specs, the `b` modifer like the `th` modifier are
suffixes and serve formatting and notation purposes.

Also, many gnu-isms are removed to facilitate \*BSD builds.

See info page examples and/or README.

v0.1.5
------
Summary: v0.1.5 of dateutils

This is dateutils v0.1.5.

This is a feature release.
Grep-like utilities have been added.

+ dgrep  like grep for date values
+ tgrep  like grep for time values
+ ttest  like test for time values
+ tcal   like dcal for time values

Changed behaviour:
Utilities in sed-mode will repeat the whole line if nothing on the line
matches instead of writing a warning.

See info page examples and/or README.

v0.1.4
------
Summary: v0.1.4 of dateutils

This is dateutils v0.1.4.

This is a feature release.
Utilities to deal with times have been added.

+ tseq  like dseq for time values
+ tadd  like dadd for time values
+ tdiff like ddiff for time values

See info page examples and/or README.

Man pages have an author now and most of the format specs are
documented, at least the specs we do not plan to change.

v0.1.3
------
Summary: release v0.1.3 of dateutils

This is dateutils v0.1.3.

This is a feature and convenience release.
Most importantly, the project is now called `dateutils`.
All tools are documented now (to some degree) and have their own tests.

v0.1.2
------
Summary: release v0.1.2 of datetools

This is datetools v0.1.2.

This is a feature release.  The dseq tool is now entirely in the hand of
our internal date library.  Furthermore, dseq now accepts negative
increments and dates where FIRST > LAST.  Hereby the increment argument
has an overloaded meaning:
If INCREMENT is negative but FIRST < LAST then compute the beginning
instead of the end.

    dseq 2000-01-01 -34 2000-03-31
    =>
      2000-01-23
      2000-02-26
      2000-03-31

as opposed to

    dseq 2000-01-01 34 2000-03-31
    =>
      2000-01-01
      2000-02-04
      2000-03-09

Likewise, FIRST can be newer than LAST and with a negative increment,
the end is variable whereas a positive increment leaves the beginning
variable.

v0.1.1
------
Summary: release v0.1.1 of datetools

This is datetools v0.1.1.

This is a clean up release with the long-term aim to outsource all
functionality into a library.  Two new command line tools have undergone
this transition, dcal and dtest.

The overall goal is to have all tools using a common set of command line
options, -i or --input-format to specify one or more input formats to be
tried and -f or --format to specify an output format if applicable.

v0.1.0
------
Summary: release v0.1.0 of datetools

This is datetools v0.1.0.

This is the first working version of datetools comprising two command
line tools, dseq and strptime.
