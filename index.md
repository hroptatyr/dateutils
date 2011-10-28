---
title: dateutils
layout: default
---

Dateutils
=========

<div id="rtop" class="sidebar-widget">
  <script type="text/javascript"
    src="http://www.ohloh.net/p/586399/widgets/project_languages.js">
  </script>
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

Dateutils commands are prefixed with a `d` for date fiddling and a `t`
for time fiddling, the only exception being `strptime`:

+ `strptime`            Command line version of the C function
+ `dconv`               Convert dates between calendars
+ `dadd`                Add durations to dates
+ `ddiff`               Compute durations from dates
+ `dseq`                Generate sequences of dates
+ `dtest`               Compare dates
+ `dgrep`               Grep dates in input streams
+ `tconv`               Convert time representations
+ `tadd`                Add durations to times
+ `tdiff`               Compute durations between times
+ `tseq`                Generate sequences of times
+ `ttest`               Compare time values
+ `tgrep`               Grep time values in input streams



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
    dconv 2012-03-01-Sun -i "%Y-%m-%c-%a"
    =>
      2012-03-04

  where the ymcw representation means, the %c-th %w of the month in a
  given year.  This is useful if dates are specified like, the third
  Thursday in May for instance.

dtest
-----
  A tool to perform date comparison in the shell, it's modelled after
  test(1) but with proper command line options.

    if dtest now --gt 2010-01-01; then
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

ddiff
-----
  A tool to compute durations between two (or more) dates.  This is
  somewhat the converse of dadd.

    ddiff 2001-02-08 2001-03-02
    =>
      22

    ddiff 2001-02-08 2001-03-10 -f "%m month and %d day"
    =>
      1 month and 1 day

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

tseq
----
  Quite like dseq but for times, and because times are generally less
  intricate than dates, the usage is straight-forward:

    tseq 12:00:00 5m 12:17:00
    =>
      12:00:00
      12:05:00
      12:10:00
      12:15:00

    tseq --compute-from-last 12:00:00 5m 12:17:00
    =>
      12:02:00
      12:07:00
      12:12:00
      12:17:00

    tseq 12:17:00 -10m 12:00:00
    =>
      12:17:00
      12:07:00

tadd
----
  This is tseq's complement as in a duration can be added to one or more
  time values, making this the tool of choice for time arithmetic:

    tadd 12:00:00 17m
    =>
      12:17:00

    tadd 12m34s <<EOF
    12:10:05
    12:50:52
    EOF
    =>
      12:22:39
      13:03:26

tdiff
-----
  This is, in a way, the converse of tadd, as it computes the duration
  between two time values:

    tdiff 12:10:05 12:22:39
    =>
      754s

tgrep
-----
  A tool to extract lines from an input stream that match certain
  criteria, showing either the line or the match:

    tgrep '>=12:00:00' <<EOF
    fileA	11:59:58
    fileB	11:59:59	leap second?
    fileNOON	12:00:00	new version
    fileC	12:03:12
    EOF
    =>
      fileNOON	12:00:00	new version
      fileC	12:03:12

ttest
-----
  A tool to perform time value comparison in the shell, it's modelled after
  test(1) but with proper command line options.

    if ttest 12:00:04 --gt 11:22:33; then
      echo "it's later than 11:22:33"
    fi
    =>
      it's later than 11:22:33

tconv
-----
  A tool to convert time strings to other representations or normalise
  them, it's quite like dconv (see above) but with less calendrical
  systems:

    tconv "23:12:45" -f "%I:%M%P"
    =>
      11:12pm

strptime
--------
  A tool that brings the flexibility of strptime(3) to the command
  line.  While date(1) has support for output formats, it lacks any kind
  of support to read arbitrary input from the domain of dates, in
  particular when the input format is specifically known beforehand and
  only matching dates/times shall be considered.

  Usually, to print something like `Mon, May-01/2000' in ISO 8601,
  people come up with the most prolific recommendations like using perl
  or sed or awk or any two of them, or they come up with a pageful of
  shell code full of bashisms, and when sufficiently pestered they
  `improve' their variant to a dozen pages of portable shell code.

  The strptime tool does the job just fine

    strptime -i "%a, %b-%d/%Y" "Mon, May-01/2000"
    =>
      2000-05-01



Similar projects
================

In no particular order and without any claim to completeness:

+ dateexpr: <http://www.eskimo.com/~scs/src/#dateexpr>
+ allanfalloon's dateutils: <https://github.com/alanfalloon/dateutils>

Use the one that best fits your purpose.  And in case you happen to like
mine, vote: [dateutils' Ohloh page](https://www.ohloh.net/p/dateutils2)

<!--
  Local variables:
  mode: auto-fill
  fill-column: 72
  filladapt-mode: t
  End variables:
-->
