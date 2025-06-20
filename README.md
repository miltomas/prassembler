# prassembler
globální knihovny - glibc (stdlib.h, string.h..)

1. **čtení argumentů, specifikace vstupu (stdin, soubor), formát výstupu**
2. **první průchod vstupu**
   - převést non-whitespace řetězec na token podle kategorie (label, instrukce, immediate, mem-access..)
   - ukládání aktuálně parsované instrukce
   - ověřit počet operandů k instrukci, jejich typ (ukládání tokenů v poli, index dopředu)
   - vyvodit velikost instrukce, aktualizovat pozici v souboru (výstupním)
   - do dynamického pole uložení struktury instrukce se zjištěnými operandy, velikostí..
     
   _pokud label -> zapsat aktuální pozici, název do tabulky symbolů (nejspíš prostě dynamické pole)_

   _pokud se objeví nečekaný znak, operand, přesáhnutý limit immediate .. error, exit_
- (porovnání podle předdefinované struktury instrukcí a jejich vlastností - implementace zatim nejasná, ale nejspíš tabulka na ktery vyhledavani s tokenem => 	match vrati strukturu s info o instrukci, jinak to neni instrukce)
3. **druhý průchod (na strukturách instrukce)**
   - zpracovat případný suffix instrukce
   - nahradit labely adresami (relativní jumpy zatim neexistují)
   - enkódovat instrukci podle šablony
   - poslat enkódovanou instrukci na output (zanedbani ELF)
     
   _pokud label není definovaný .. error, exit_
4. **epilog**

Předpokládám, že implementace bude vrcholný C experience => pouze standardní knihovna.
*jo ale taky teda GNU gperf nakonec.*

**další rešerše architektury potřeba**

**další rešerše formátu ELF potřeba**
