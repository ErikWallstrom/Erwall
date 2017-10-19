# Gymnasiearbete - Skapa ett programmeringsspråk: Rui

## Frågeställningar

* Vilka delar är en kompiler uppbyggd av?
* Hur bra fungerar C som ett programmeringsspråk för att skapa en kompiler?
* Är C ett bra mål att generera kod till?
* Vad krävs av ett programmeringsspråk för att det ska anses som "bra"?
* Hur ska man planera ett projekt för att tillåta enkel felsökning och tillägg \
  av nya funktioner?

## Tidsplan

| Vecka	| Plan																|
|-------|-------------------------------------------------------------------|
| 41	| Research, påbörja design av språket								|
| 42	| Design av språk, börja programmera en lexer-prototyp				| 
| 43	| Göra klart lexer-prototypen, börja skapa en prototyp av en parser	|
| 44	| Fortsätta arbetet på parser-prototypen, få den att fungera		|	
| 45	| Skapa en prototyp av en code-generator							|
| 46	| Ha en fungerande prototyp av en kompiler							|
| 47	| Påbörja optimering												|
| 48	| Förbättra lexer och parser										|
| 49	| Förbättra kod-generatorn											|
| 50	| Börja arbeta på ett standard-bibliotek för språket				|
| 51	| Fortsätta arbeta på standard-biblioteket							|
| 52	| Försöka få en C API att fungera för språket						|
| 1		| Extra tid															|
| 2     | Extra tid															|
| 3		| Extra tid															|
| 4		| Skrivande av rapport												|
| 5		| Skrivande av rapport												|
| 6		| Rapport färdig													|

## Goals

* Logical consistent syntax
* Explicit
* Fast and low level

## Ideas

* Arrays as first-class citizens
* Size of arrays known
* Compile time bounds checking (zero cost)
* Strict types (no implicit int to float or enum to int conversions)
* Tagged unions by default
* Nested comments
* References (non-null pointers)
* Function definitions in functions
* Defer (like Go)
* Anonymous struct in structs (like Go)
* Compatible with C

* Literal heavy (why declare variables when what you really want are values?) (?)
* Default values for structs (?)
* Switch working for all types (?)
* Const by default (?)
* Types should begin with capital letter (?)
* Resizable arrays (?)
* Type inference (?)

![](http://www.gnu.org/graphics/gplv3-127x51.png "GPLv3")
