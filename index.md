---
title: dateutils
layout: default
---

Dateutils
=========

Dateutils are a bunch of tools that revolve around fiddling with dates
and times in the command line with a strong focus on use cases that
arise when dealing with large amounts of financial data.

Dateutils are hosted primarily on github:

  https://github.com/hroptatyr/dateutils

Below is a short list of examples that demonstrate what dateutils can
do, for full specs refer to the info and man pages.  For installation
instructions refer to the INSTALL file.

Dateutils provides new commands:
+ strptime            Command line version of the C function
+ dcal                Convert dates between calendars
+ dadd                Add durations to dates
+ ddiff               Compute durations from dates
+ dseq                Generate sequences of dates
+ dtest               Compare dates
+ tdiff               Compute durations between times
+ tseq                Generate sequences of times



Examples
========

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

dcal
----
  A tool to convert dates between different calendric systems.  While
  other such tools usually focus on converting Gregorian dates to, say,
  the Chinese calendar, dcal aims at supporting calendric systems which
  are essential in financial contexts.

  To convert a (Gregorian) date into the so called ymcw representation:
    dcal 2012-03-04 -f "%Y-%m-%c-%w"
    =>
      2012-03-01-00

  and vice versa:
    dcal 2012-03-01-Sun -i "%Y-%m-%c-%a"
    =>
      2012-03-04

  where the ymcw representation means, the %c-th %w of the month in a
  given year.  This is useful if dates are specified like, the third
  Thursday in May for instance.

dtest
-----
  A tool to perform date comparison in the shell, it's modelled after
  test(1) but with proper command line options.

    if src/dtest now --gt 2010-01-01; then
      echo "yes"
    fi
    =>
      yes

dadd
----
  A tool to perform date/duration arithmetic in the shell.  Given a date
  and a a list of durations this will compute new dates.  Given a
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
      0m22d

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
