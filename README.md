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

[iata.tzmap][5]
---------------

- Domain: IATA 3-letter airport codes
- Author: [Sebastian Freundt][11]
- Source: [The GeoNames geographical database][3]
- Licence: [Creative Commons Attribution 3.0 License][4]

This list maps airports by their IATA 3-letter airport code to the local
IANA timezone.  It is generally assumed that the airport in question
indeed operates in local time.

[icao.tzmap][6]
---------------

- Domain: ICAO 4-letter airport codes
- Author: [Sebastian Freundt][11]
- Source: [The GeoNames geographical database][3]
- Licence: [Creative Commons Attribution 3.0 License][4]

This list maps airports by their ICAO 4-letter airport code to the local
IANA timezone.  It is generally assumed that the airport in question
indeed operates in local time.

[mic.tzmap][9]
--------------

- Domain: ISO-10383 4-letter Market Identifier Codes
- Author: [Sebastian Freundt][11]
- Source: [Codes for exchanges and market identification][10]
- Licence: [Creative Commons Attribution 3.0 License][4]

This list maps market codes to their IANA timezones based on a market's
official trading hours and report times.  I.e. the mapping does not
necessarily reflect where the market is headquartered.

Development
-----------

The above .tzmap files are managed in the [`tzmaps` branch][8] of the
[dateutils repository][7].  Patches are welcome.

  [1]: http://www.fresse.org/dateutils/
  [2]: http://www.iana.org/time-zones
  [3]: http://download.geonames.org/export/dump/
  [4]: http://creativecommons.org/licenses/by/3.0/
  [5]: https://raw.github.com/hroptatyr/dateutils/tzmaps/iata.tzmap
  [6]: https://raw.github.com/hroptatyr/dateutils/tzmaps/icao.tzmap
  [7]: https://github.com/hroptatyr/dateutils
  [8]: https://github.com/hroptatyr/dateutils/tree/tzmaps
  [9]: https://raw.github.com/hroptatyr/dateutils/tzmaps/mic.tzmap
  [10]: http://www.iso15022.org/MIC/homepageMIC.htm
  [11]: http://www.fresse.org/

<!--
  Local variables:
  mode: auto-fill
  fill-column: 72
  filladapt-mode: t
  End:
-->
