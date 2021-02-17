---
title: dateutils
layout: project
logo: dateutils_logo_120.png
project: dateutils
latest: dateutils-0.4.8.tar.xz
snap: https://hroptatyr.gitlab.io/dateutils/dateutils-latest.tar.xz
description: dateutils, command-line date calculation and conversion tools
---

Dateutils
=========

  <div class="sidebar-stack">
    <script type="text/javascript">
      var uvOptions = {};
      (function() {
        var uv = document.createElement('script');
        uv.type = 'text/javascript';
        uv.async = true;
        uv.src = ('https:' == document.location.protocol ? 'https://' : 'http://') + 'widget.uservoice.com/dvXNMbpRLq3837hCrt5W2Q.js';
        var s = document.getElementsByTagName('script')[0];
        s.parentNode.insertBefore(uv, s);
      })();
    </script>
  </div>

Dateutils are a bunch of tools that revolve around fiddling with dates
and times in the command line with a strong focus on use cases that
arise when dealing with large amounts of financial data.

Dateutils are hosted primarily on github:

+ project homepage: <http://www.fresse.org/dateutils/>
+ github page: <https://github.com/hroptatyr/dateutils>
+ downloads: <https://bitbucket.org/hroptatyr/dateutils/downloads>

Below is a short list of examples that demonstrate what dateutils can
do, for full specs refer to the info and man pages.  For installation
instructions refer to the INSTALL file.

Dateutils commands are prefixed with a `date` but otherwise resemble
known unix commands for reasons of intuition.  The only exception being
`strptime` which is analogous to the libc function of the same name.

+ [`strptime`](#strptime)     Command line version of the C function
+ [`dateadd`](#dateadd)       Add durations to dates or times
+ [`dateconv`](#dateconv)     Convert dates or times between calendars
+ [`datediff`](#datediff)     Compute durations between dates or times
+ [`dategrep`](#dategrep)     Grep dates or times in input streams
+ [`dateround`](#dateround)   Round dates or times to "fuller" values
+ [`dateseq`](#dateseq)       Generate sequences of dates or times
+ [`datesort`](#datesort)     Sort chronologically.
+ [`datetest`](#datetest)     Compare dates or times
+ [`datezone`](#datezone)     Convert date/times to timezones in bulk


Distributions
=============

Following Linux distros and BSD flavours provide native packages
(in alphabetical order):

+ Debian <http://packages.debian.org/sid/dateutils>
+ DragonFly BSD <http://gitweb.dragonflybsd.org/dports.git/tree/HEAD:/sysutils/dateutils>
+ Fedora <https://admin.fedoraproject.org/pkgdb/package/dateutils/>
+ FreeBSD <http://svnweb.freebsd.org/ports/head/sysutils/dateutils/>
+ Gentoo <https://packages.gentoo.org/package/app-misc/dateutils>
+ NetBSD <http://pkgsrc.se/time/dateutils>
+ OpenSuSE <http://software.opensuse.org/download.html?project=utilities&package=dateutils>
+ OS X Homebrew <http://brewformulas.org/Dateutils>
+ Slackware <http://slackbuilds.org/repository/14.1/system/dateutils/>
+ Ubuntu <http://packages.ubuntu.com/xenial/dateutils>


[Changelog][1]
==============

See dedicated [changelog page][1].

  [1]: /dateutils/changelog.html


Examples
========

I love everything explained by example to get a first impression.
So here it comes.

dateseq
-------
  A tool mimicking seq(1) but whose inputs are from the domain of dates
  rather than integers.  Typically scripts use something like

    $ for i in $(seq 0 9); do
        date -d "2010-01-01 +${i} days" "+%F"
      done

  which now can be shortened to

    $ dateseq 2010-01-01 2010-01-10

  with the additional benefit that the end date can be given directly
  instead of being computed from the start date and an interval in
  days.  Also, it provides date specific features that would be a PITA
  to implement using the above seq(1)/date(1) approach, like skipping
  certain weekdays:

    $ dateseq 2010-01-01 2010-01-10 --skip sat,sun
    =>  
      2010-01-01
      2010-01-04
      2010-01-05
      2010-01-06
      2010-01-07
      2010-01-08

  dateseq also works on times:

    $ dateseq 12:00:00 5m 12:17:00
    =>
      12:00:00
      12:05:00
      12:10:00
      12:15:00

  and also date-times:

    $ dateseq --compute-from-last 2012-01-02T12:00:00 5m 2012-01-02T12:17:00
    =>
      2012-01-02T12:02:00
      2012-01-02T12:07:00
      2012-01-02T12:12:00
      2012-01-02T12:17:00

dateconv
--------
  A tool to convert dates between different calendric systems and/or
  time zones.  While other such tools usually focus on converting
  Gregorian dates to, say, the Chinese calendar, dconv aims at
  supporting calendric systems which are essential in financial
  contexts.

  To convert a (Gregorian) date into the so called ymcw representation:

    $ dateconv 2012-03-04 -f "%Y-%m-%c-%w"
    =>
      2012-03-01-00

  and vice versa:

    $ dateconv 2012-03-01-Sun -i "%Y-%m-%c-%a" -f '%F'
    =>
      2012-03-04

  where the ymcw representation means, the `%c`-th `%w` of the month in
  a given year.  This is useful if dates are specified like, the third
  Thursday in May for instance.

  dateconv can also be used to convert occurrences of dates, times or
  date-times in an input stream on the fly

    $ dateconv -S -i '%b/%d %Y at %I:%M %P' <<EOF
    Remember we meet on Mar/03 2012 at 02:30 pm
    EOF
    =>
      Remember we meet on 2012-03-03T14:30:00

  and most prominently to convert between time zones:

    $ dateconv --from-zone "America/Chicago" --zone "Asia/Tokyo" 2012-01-04T09:33:00
    =>
      2012-01-05T00:33:00

    $ dateconv --zone "America/Chicago" now -f "%d %b %Y %T"
    =>
      05 Apr 2012 11:11:57

datetest
--------
  A tool to perform date comparison in the shell, it's modelled after
  `test(1)` but with proper command line options.

    $ if datetest today --gt 2010-01-01; then
        echo "yes"
      fi
    =>
      yes

dateadd
-------
  A tool to perform date arithmetic (date maths) in the shell.  Given
  a date and a list of durations this will compute new dates.  Given a
  duration and a list of dates this will compute new dates.

    $ dateadd 2010-02-02 +4d
    =>
      2010-02-06

    $ dateadd 2010-02-02 +1w
    =>
      2010-02-09

    $ dateadd -1d <<EOF
    2001-01-05
    2001-01-01
    EOF
    =>
      2001-01-04
      2000-12-31

  Adding durations to times:

    $ dateadd 12:05:00 +10m
    =>
      12:15:00

  and even date-times:

    $ dateadd 2012-03-12T12:05:00 -1d4h
    =>
      2012-03-11T08:05:00

  If supported by the system's zoneinfo database leap-second adjusted
  calculations are possible.  Use the unit `rs` to denote "real" seconds:

    $ dateadd '2012-06-30 23:59:30' +30rs
    =>
      2012-06-30T23:59:60

  as opposed to:

    $ dateadd '2012-06-30 23:59:30' +30s
    =>
      2012-07-01T00:00:00

datediff
--------
  A tool to calculate the difference between two (or more) dates.  This
  is somewhat the converse of dadd.  Outputs will be durations that,
  when added to the first date, give the second date.

  Get the number of days between two dates:

    $ datediff 2001-02-08 2001-03-02
    =>
      22

  The duration format can be controlled through the `-f` switch:

    $ datediff 2001-02-08 2001-03-09 -f "%m month and %d day"
    =>
      1 month and 1 day

  datediff also accepts time stamps as input:

    $ datediff 2012-03-01T12:17:00 2012-03-02T14:00:00
    =>
      92580s

  The `-f` switch does the right thing:

    $ datediff 2012-03-01T12:17:00 2012-03-02T14:00:00 -f '%dd %Ss'
    =>
      1d 6180s

  compare to:

    $ datediff 2012-03-01T12:17:00 2012-03-02T14:00:00 -f '%dd %Hh %Ss'
    =>
      1d 1h 2580s

  If supported by the system's zoneinfo database leap-second adjusted
  calculations can be made.  Use the format specifier `%rS` to get the
  elapsed time in "real" seconds:

    datediff '2012-06-30 23:59:30' '2012-07-01 00:00:30' -f '%rS'
    =>
      61

dategrep
--------
  A tool to extract lines from an input stream that match certain
  criteria, showing either the line or the match:

    $ dategrep '<2012-03-01' <<EOF
    Feb	2012-02-28
    Feb	2012-02-29	leap day
    Mar	2012-03-01
    Mar	2012-03-02
    EOF
    =>
      Feb	2012-02-28
      Feb	2012-02-29	leap day

dateround
---------
  A tool to "round" dates or time stamps to a recurring point in time,
  like the next/previous January or the next/previous Thursday.

  Round (backwards) to the first of the current month:

    $ dateround '2011-08-22' -1
    =>
      2011-08-01

  Find the next Monday from the current date (today is 2016-01-08):

    $ dateround today Mon
    =>
      2015-01-11

  Go back to last September, then round to the end of the month:

    $ dateround today -- -Sep +31d
    =>
      2015-09-30

  Round a stream of dates strictly to the next month's first:

    $ dateround -S -n 1 <<EOF
    pay cable	2012-02-28
    pay gas	2012-02-29
    pay rent	2012-03-01
    redeem loan	2012-03-02
    EOF
    =>
      pay cable	2012-03-01
      pay gas	2012-03-01
      pay rent	2012-04-01
      redeem loan	2012-04-01

  Round a timeseries to the next minute (i.e. the seconds part is 00)
  and then to the next half-past time (and convert to ISO):

    $ dateround -S 0s30m -i '%d/%m/%Y %T' -f '%F %T' <<EOF
    06/03/2012 14:27:12	eventA
    06/03/2012 14:29:59	eventA
    06/03/2012 14:30:00	eventB
    06/03/2012 14:30:01	eventB
    EOF
    =>
      2012-03-06 14:30:00	eventA
      2012-03-06 14:30:00	eventA
      2012-03-06 14:30:00	eventB
      2012-03-06 15:30:00	eventB

  Alternatively, if you divide the day into half-hours you can round to
  one of those using the co-class notation:

    $ dateround -S /30m -i '%d/%m/%Y %T' -f '%F %T' <<EOF
    06/03/2012 14:27:12	eventA
    06/03/2012 14:29:59	eventA
    06/03/2012 14:30:00	eventB
    06/03/2012 14:30:01	eventB
    EOF
    =>
      2012-03-06 14:30:00	eventA
      2012-03-06 14:30:00	eventA
      2012-03-06 14:30:00	eventB
      2012-03-06 15:00:00	eventB

  This is largely identical to the previous example except, that a full
  hour (being an even multiple of half-hours) is a possible rounding
  target.


datesort
-----
  A tool to bring the lines of a file into chronological order.

  At the moment the `datesort` tool depends on `sort(1)` with support
  for fields, in particular `-t` to select a separator and `-k` to sort
  by a particular field.

    $ datesort <<EOF
    2009-06-03 caev="DVCA" secu="VOD" exch="XLON" xdte="2009-06-03" nett/GBX="5.2"
    2011-11-16 caev="DVCA" secu="VOD" exch="XLON" xdte="2011-11-16" nett/GBX="3.05"
    2013-11-20 caev="DVCA" secu="VOD" exch="XLON" xdte="2013-11-20" nett/GBX="3.53"
    2012-06-06 caev="DVCA" secu="VOD" exch="XLON" xdte="2012-06-06" nett/GBX="6.47"
    2013-06-12 caev="DVCA" secu="VOD" exch="XLON" xdte="2013-06-12" nett/GBX="6.92"
    2010-11-17 caev="DVCA" secu="VOD" exch="XLON" xdte="2010-11-17" nett/GBX="2.85"
    EOF
    =>
      2009-06-03 caev="DVCA" secu="VOD" exch="XLON" xdte="2009-06-03" nett/GBX="5.2"
      2010-11-17 caev="DVCA" secu="VOD" exch="XLON" xdte="2010-11-17" nett/GBX="2.85"
      2011-11-16 caev="DVCA" secu="VOD" exch="XLON" xdte="2011-11-16" nett/GBX="3.05"
      2012-06-06 caev="DVCA" secu="VOD" exch="XLON" xdte="2012-06-06" nett/GBX="6.47"
      2013-06-12 caev="DVCA" secu="VOD" exch="XLON" xdte="2013-06-12" nett/GBX="6.92"
      2013-11-20 caev="DVCA" secu="VOD" exch="XLON" xdte="2013-11-20" nett/GBX="3.53"


datezone
--------
  A tool to quickly inspect date/time values in different timezones.
  The result will be a matrix that shows every date-time value in every
  timezone:

    $ datezone Europe/Berlin Australia/Sydney now 2014-06-30T05:00:00
    =>
      2014-01-30T17:37:13+01:00	Europe/Berlin
      2014-01-31T03:37:13+11:00	Australia/Sydney
      2014-06-30T07:00:00+02:00	Europe/Berlin
      2014-06-30T15:00:00+10:00	Australia/Sydney

  The `datezone` tool can also be used to obtain the next or previous DST
  transition relative to a given date/time:

    $ datezone --next Europe/Berlin Australia/Sydney 2013-02-19
    =>
      2013-03-31T02:00:00+01:00 -> 2013-03-31T03:00:00+02:00	Europe/Berlin
      2013-04-07T03:00:00+11:00 -> 2013-04-07T02:00:00+10:00	Australia/Sydney

  where the left time stamp denotes the current zone offset and the
  right side is the zone offset after the transition.  The date/time
  indicates the exact moment when the transition is about to take
  place.

  In essence `datezone` is a better [`zdump(8)`][5].

  [5]: http://linux.die.net/man/8/zdump


strptime
--------
  A tool that brings the flexibility of [`strptime(3)`][2] to the
  command line.  While (at least GNU) [`date(1)`][3] has support for
  output formats, it lacks any kind of support to read arbitrary input
  from the domain of dates, in particular when the input format is
  specifically known beforehand and only matching dates/times shall be
  considered.

  With the `strptime` tool reading weird dates like `Mon, May-01/2000`
  becomes a matter of

    strptime -i "%a, %b-%d/%Y" "Mon, May-01/2000"
    =>
      2000-05-01

  just as you would have done in C.

  Note that `strptime` actually uses the system libc's strptime routine,
  and for output the system's strftime routine.  Input and output
  modifiers will therefore vary between systems.

  For a portable parser/printer combination use `dateconv` as described
  above.  Its input and output format specifiers are independent of the
  C runtime.

  [2]: http://linux.die.net/man/3/strptime
  [3]: http://linux.die.net/man/1/date


[Timezone map files][4]
=======================

Starting with version 0.3.0 dateutils has built-in support for
[tzmaps][4].  We've dedicated a branch (orphan `tzmaps`) for their
development and a [website][4] for further information.

  [4]: /dateutils/tzmaps.html


Locale support
==============

Since version 0.4.0 dateutils allows for reading and printing localised
dates: Seeing as input had to be specified explicitly from day 1, this
feature covers weekday and month names only.

As is generally the philosophy of dateutils, locale support does not
depend on system infrastructure (libc in this case) nor does it follow
the usual semantics of setting `LC_TIME`, so different locales can be
used for input and output independent of the user's environment.

All tools support the `--from-locale` parameter while tools that output
date/times also support `--locale`.

    $ dateconv --from-locale it_IT -i '%d %B %Y' '19 maggio 2016'
    2016-05-19
    $ dateconv --locale fr_FR -f '%d %B %Y' 2016-05-19
    19 mai 2016
    $

The [locale file][6] is a simple tab separated text, following the
locale identifier line (`xx_XX`) is the line of abbreviated weekday
names (`%a`) of which there must be 7 corresponding to Mon, Tue, ...,
followed by long weekday names (`%A`), followed by abbreviated month
names (`%b`) of which there must be 12 corresponding to Jan, Feb, ...,
followed last by the long month names (`%B`).

Extending the file is thus a matter of adding 5 lines.  The environment
variable `LOCALE_FILE` can be used to override the default location.

  [6]: https://github.com/hroptatyr/dateutils/blob/master/data/locale


Similar projects
================

In no particular order and without any claim to completeness:

+ dateexpr: <http://www.eskimo.com/~scs/src/#dateexpr>
+ allanfalloon's dateutils: <https://github.com/alanfalloon/dateutils>
+ yest <http://yest.sourceforge.net/>
+ pdd <https://github.com/jarun/pdd>

Use the one that best fits your purpose.  And in case you happen to like
mine, vote: [dateutils' openhub page](https://www.openhub.net/p/dateutils)


Social Media et al
==================

Yes, we're aware of social media:

+ [twitter](https://twitter.com/dateutils)
+ [Openhub](https://www.openhub.net/p/dateutils)
+ [Google Groups Mailing List](https://groups.google.com/d/forum/dateutils)

<!--
  Local variables:
  mode: auto-fill
  fill-column: 72
  filladapt-mode: t
  End:
-->
