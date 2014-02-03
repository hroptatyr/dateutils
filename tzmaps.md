---
title: dateutils tzmaps
layout: default
---

Timezone map files
==================

tzmap files (suffix `.tzm`) provide an alternative means to specify the
timezone of a date/time.  By default, in [dateutils][1] and many other
software environments, timezones are either specified directly, e.g. by
appending the offset to UTC as in `+04:00`, or, more accurately when
DST shifts are taken into account, by using an [IANA zoneinfo name][2],
e.g. `CEST` or `Europe/Berlin`.

Often however, the appropriate IANA zoneinfo name requires profound
domain knowledge, e.g. you'd have to know that all parts of the UK share
the same zonename (`GB` or `Europe/London`) and while there is
`Europe/London` and `Europe/Belfast`, by analogy `Europe/Edinburgh` and
`Europe/Cardiff` are missing.  And while most of Western mainland Europe
nowadays uses CEST, it is important to distinguish strictly between the
cities for dates before 1960.  Such intricacies might be more or less
commonly known, but when you approach timezones from a different domain,
e.g. airport codes, the problems start to multiply.

tzmap files take a specifier in a particular domain and map it to one or
more IANA zoneinfo names.  Whenever the mapping is unique the software
environment ought to accept the specifier in-lieu of a zoneinfo name.
In particular, [dateutils][1] (from v0.3.0 onwards) allows alternative
specifiers in the format `DOMAIN:SPECIFIER`, where `DOMAIN.tzmap` is a
file in the tzmap search path and `SPECIFIER` is a valid entry therein
mapping it to a zoneinfo name.

tzmap files are simply tab-separated lists of the format:

    SPECIFIER \t ZONENAME [\t ZONENAME]...

sorted by `SPECIFIER`.  On this site we provide mappings from IATA and
ICAO airport codes.

iata.tzmap
----------

- Domain: IATA 3-letter airport codes
- Source: [The GeoNames geographical database][3]
- Licence: [Creative Commons Attribution 3.0 License][4]

icao.tzmap
----------

- Domain: ICAO 4-letter airport codes
- Source: [The GeoNames geographical database][3]
- Licence: [Creative Commons Attribution 3.0 License][4]


  [1]: http://www.fresse.org/dateutils/
  [2]: http://www.iana.org/time-zones
  [3]: http://download.geonames.org/export/dump/
  [4]: http://creativecommons.org/licenses/by/3.0/

<!--
  Local variables:
  mode: auto-fill
  fill-column: 72
  filladapt-mode: t
  End:
-->
