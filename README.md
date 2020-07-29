# search

## TLDR;

Cross-platform search engine written in C++.

## Getting Started

TODO

## Querys

Space delimetered keywords. Un-prefixed keyword results are full joined, '+' inner joins that keyword results, '-' excludes keyword results:

icecream +chocolate -vanila

## Todo

* File type filtering. ( Optimization, E.G. don't parse compressed or binary files )
* File type comprehension. ( E.G. Ability to parse compressed archives, XML, PDF, DOCX files without taking in metadata )
* Text encoding, utf8, usc16 ... ( Only ASCII currently supported )
