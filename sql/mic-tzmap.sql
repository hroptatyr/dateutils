SPARQL
SELECT
	STRAFTER(str(?s), '/markets#') AS ?mic 
	STRAFTER(str(?z), 'resource/') AS ?timezone
WHERE {
	?s a figi-gii:PricingSource .
	?s time:timeZone ?z .
	?s gas:symbolOf <http://www.iso20022.org/10383/> .
}
ORDER BY ?mic;
