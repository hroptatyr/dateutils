---
title: dateutils
layout: default
---

Dateutils
=========

<div id="rtop" class="sidebar-widget">
  <div class="sidebar-stack">
    <ul>
      <li>
        <script type="text/javascript"
          src="http://www.ohloh.net/p/586399/widgets/project_languages.js">
        </script>
      </li>
    </ul>
  </div>
  <div class="sidebar-stack">
    <ul>
      <li><a href="https://github.com/hroptatyr/dateutils">github page</a></li>
      <li><a href="https://github.com/downloads/hroptatyr/dateutils/dateutils-0.2.1.tar.xz">latest release (dateutils-0.2.1.tar.xz)</a></li>
      <li><a href="/dateutils/binaries">prebuilt binaries</a></li>
    </ul>
  </div>
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
</div>

Dateutils are a bunch of tools that revolve around fiddling with dates
and times in the command line with a strong focus on use cases that
arise when dealing with large amounts of financial data.

Dateutils are hosted primarily on github:

+ github page: <https://github.com/hroptatyr/dateutils>
+ project homepage: <http://hroptatyr.github.com/dateutils/>

Below is a short list of examples that demonstrate what dateutils can
do, for full specs refer to the info and man pages.  For installation
instructions refer to the INSTALL file.

Dateutils commands are prefixed with a `d` but otherwise resemble known
unix commands for reasons of intuition.  The only exception being
`strptime` which is analogous to the libc function of the same name.

+ `strptime`            Command line version of the C function
+ `dadd`                Add durations to dates or times
+ `dconv`               Convert dates or times between calendars
+ `ddiff`               Compute durations between dates or times
+ `dgrep`               Grep dates or times in input streams
+ `dseq`                Generate sequences of dates or times
+ `dtest`               Compare dates or times



Examples
========

I love everything to be explained by example to get a first
impression.  So here it comes.

dseq
----
  A tool mimicking seq(1) but whose inputs are from the domain of dates
  rather than integers.  Typically scripts use something like

    for i in $(seq 0 9); do
        date -d "2010-01-01 +${i} days" "+%F"
    done

  which now can be shortened to

    dseq 2010-01-01 2010-01-10

  with the additional benefit that the end date can be given directly
  instead of being computed from the start date and an interval in
  days.  Also, it provides date specific features that would be a PITA
  to implement using the above seq(1)/date(1) approach, like skipping
  certain weekdays:

    dseq 2010-01-01 2010-01-10 --skip sat,sun
    =>  
      2010-01-01
      2010-01-04
      2010-01-05
      2010-01-06
      2010-01-07
      2010-01-08

  dseq also works on times:

    dseq 12:00:00 5m 12:17:00
    =>
      12:00:00
      12:05:00
      12:10:00
      12:15:00

  and also date-times:

    dseq --compute-from-last 2012-01-02T12:00:00 5m 2012-01-02T12:17:00
    =>
      2012-01-02T12:02:00
      2012-01-02T12:07:00
      2012-01-02T12:12:00
      2012-01-02T12:17:00

dconv
-----
  A tool to convert dates between different calendric systems.  While
  other such tools usually focus on converting Gregorian dates to, say,
  the Chinese calendar, dconv aims at supporting calendric systems which
  are essential in financial contexts.

  To convert a (Gregorian) date into the so called ymcw representation:

    dconv 2012-03-04 -f "%Y-%m-%c-%w"
    =>
      2012-03-01-00

  and vice versa:

    dconv 2012-03-01-Sun -i "%Y-%m-%c-%a" -f '%F'
    =>
      2012-03-04

  where the ymcw representation means, the %c-th %w of the month in a
  given year.  This is useful if dates are specified like, the third
  Thursday in May for instance.

  dconv can also be used to convert occurrences of dates, times or
  date-times in an input stream on the fly

    dconv -S -i '%b/%d %Y at %I:%M %P' <<EOF
    Remember we meet on Mar/03 2012 at 02:30 pm
    EOF
    =>
      Remember we meet on 2012-03-03T14:30:00

  and most prominently to convert between time zones:

    dconv --from-zone "America/Chicago" --zone "Asia/Tokyo" 2012-01-04T09:33:00
    =>
      2012-01-05T00:33:00

    dconv --zone "America/Chicago" now -f "%d %b %Y %T"
    =>
      05 Apr 2012 11:11:57

dtest
-----
  A tool to perform date comparison in the shell, it's modelled after
  test(1) but with proper command line options.

    if dtest today --gt 2010-01-01; then
      echo "yes"
    fi
    =>
      yes

dadd
----
  A tool to perform date arithmetic (date maths) in the shell.  Given
  a date and a list of durations this will compute new dates.  Given a
  duration and a list of dates this will compute new dates.

    dadd 2010-02-02 +4d
    =>
      2010-02-06

    dadd 2010-02-02 +1w
    =>
      2010-02-09

    dadd -1d <<EOF
    2001-01-05
    2001-01-01
    EOF
    =>
      2001-01-04
      2000-12-31

  Adding durations to times:

    dadd 12:05:00 +10m
    =>
      12:15:00

  and even date-times:

    dadd 2012-03-12T12:05:00 -1d4h
    =>
      2012-03-11T08:05:00

  As of version v0.2.2 leap-second adjusted calculations are built-in.
  Use the unit `rs` to denote "real" seconds:

    dadd '2012-06-30 23:59:30' +30rs
    =>
      2012-06-30T23:59:60

  as opposed to:

    dadd '2012-06-30 23:59:30' +30s
    =>
      2012-07-01T00:00:00

ddiff
-----
  A tool to calculate the difference between two (or more) dates.  This
  is somewhat the converse of dadd.  Outputs will be durations that,
  when added to the first date, give the second date.

  Get the number of days between two dates:

    ddiff 2001-02-08 2001-03-02
    =>
      22

  The duration format can be controlled through the `-f` switch:

    ddiff 2001-02-08 2001-03-09 -f "%m month and %d day"
    =>
      1 month and 1 day

  ddiff also accepts time stamps as input:

    ddiff 2012-03-01T12:17:00 2012-03-02T14:00:00
    =>
      92580s

  The `-f` switch does the right thing:

    ddiff 2012-03-01T12:17:00 2012-03-02T14:00:00 -f '%dd %Ss'
    =>
      1d 6180s

  compare to:

    ddiff 2012-03-01T12:17:00 2012-03-02T14:00:00 -f '%dd %Hh %Ss'
    =>
      1d 1h 2580s

  As of version v0.2.2 leap-second adjusted calculations can be made.
  Use the format specifier `%rS` to get the elapsed time in "real"
  seconds:

    ddiff '2012-06-30 23:59:30' '2012-07-01 00:00:30' -f '%rS'
    =>
      61

dgrep
-----
  A tool to extract lines from an input stream that match certain
  criteria, showing either the line or the match:

    dgrep '<2012-03-01' <<EOF
    Feb	2012-02-28
    Feb	2012-02-29	leap day
    Mar	2012-03-01
    Mar	2012-03-02
    EOF
    =>
      Feb	2012-02-28
      Feb	2012-02-29	leap day

dround
------
  New in dateutils 0.2.1.
  A tool to "round" dates or time stamps to a recurring point in time,
  like the next/previous January or the next/previous Thursday.

  Round (backwards) to the first of the current month:

    dround '2011-08-22' -1
    =>
      2011-08-01

  Round a stream of dates strictly to the next month's first:

    dround -S -n 1 <<EOF
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

  Round a timeseries to the next full or half hour (and convert to ISO):

    dround -S 30m -i '%d/%m/%Y %T' -f '%F %T' <<EOF
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

strptime
--------
  A tool that brings the flexibility of strptime(3) to the command
  line.  While date(1) has support for output formats, it lacks any kind
  of support to read arbitrary input from the domain of dates, in
  particular when the input format is specifically known beforehand and
  only matching dates/times shall be considered.

  Usually, to print something like `Mon, May-01/2000` in ISO 8601,
  people come up with the most prolific recommendations like using perl
  or sed or awk or any two of them, or they come up with a pageful of
  shell code full of bashisms, and when sufficiently pestered they
  "improve" their variant to a dozen pages of portable shell code.

  The strptime tool does the job just fine

    strptime -i "%a, %b-%d/%Y" "Mon, May-01/2000"
    =>
      2000-05-01

  just like you would have done in C.


Similar projects
================

In no particular order and without any claim to completeness:

+ dateexpr: <http://www.eskimo.com/~scs/src/#dateexpr>
+ allanfalloon's dateutils: <https://github.com/alanfalloon/dateutils>
+ yest <http://yest.sourceforge.net/>

Use the one that best fits your purpose.  And in case you happen to like
mine, vote: [dateutils' Ohloh page](https://www.ohloh.net/p/dateutils2)

<!--
  Local variables:
  mode: auto-fill
  fill-column: 72
  filladapt-mode: t
  End:
-->
