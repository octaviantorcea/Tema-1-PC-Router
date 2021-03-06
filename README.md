# Tema 1 - Protocoale de comunicatii

Torcea Octavian 324CA

* rtable.c: implementarea tabelei de rutare si functii auxiliare
    
    * ```parse_rtable```:
        * creeaza o tabela de rutare pe baza unui fisier primit ca argument
        * ordoneaza crescator in functie de prefix intrarile din tabel
        * in caz de egalitate, intrarile sunt ordonate crescator in functie de masca
    
    * ```get_best_route```:
        * intoarce cea mai potrivita intrare din tabela de rutare in functie de adresa IP data ca argument; in cazul in care nu este gasita nicio intrare, intoarce NULL
        * cautarea se efectueaza folosind binary search


* arp_table.c: implementarea tabelei ARP si functii auxiliare
    
    * ```new_arp_table```: creeaza o tabela ARP goala, cu maxim 512 intrari

    * ```add_arp_entry```: adauga in tabela ARP o noua intrare conform argumentelor primite

    * ```get_arp_entry```: intoarce intrarea ce contine aceeasi adresa IP ca cea primita ca argument; in cazul in care o astfel de intrare nu exista, intoarce `NULL`


* router.c: implementarea efectiva a routerului
    * se parseaza tabela de rutare, se creeaza coada pentru pachete si o noua tabela ARP
    * primeste un pachet
    * daca este un pachet de tip ```ICMP_ECHO``` si are ca adresa IP de destinatie adresa IP a interfetei routerului pe care a fost primit pachetul, trimite inapoi catre adresa IP sursa un pachet ```ICMP_ECHOREPLY```
    * daca este un pachet ARP sunt 2 variante:
        * daca este de tip ```ARP_REQUEST``` si adresa IP de destinatie este adresa
          IP a interfetei routerului pe care a fost primit pachetul, se trimite
          un pachet ```ARP_REPLY``` cu adresa MAC a interfetei
        * daca este de tip ```ARP_REPLY```, se va adauga in tabela ARP o noua intrare
          cu adresa IP si MAC primite ca raspuns; in cazul in care coada de
          pachete nu este goala, se va trimite primul pachet din coada catre
          adresa MAC primita ca raspuns din pachetul ```ARP_REPLY``` primit anterior
    * altfel, inseamna ca trebuie facut forward pe pachetul primit:
        * se extrage headerul IP
        * se verifica checksum-ul (daca este gresit, pachetul este aruncat)
        * se verifica daca TTL-ul este mai mare ca 1; in caz contrar, pachetul
          este aruncat si este trimis catre sursa un pachet ICMP de tipul
          ```ICMP_TIME_EXCEEDED```
        * se decrementeaza TTL-ul
        * se face update la checksum
        * se cauta o intrare potrivita in tabela de rutare; in cazul in care nu
          a fost gasita nicio intrare potrivita, pachetul este aruncat si este
          trimis catre sursa un pachet ICMP de tipul ICMP_DEST_UNREACH
        * se cauta apoi o intrare potrivita in tabela ARP; daca nu exista o
          astfel de intrare, routerul va pune o copie a pachetului in coada de
          pachete, va trimite un ```ARP_REQUEST``` pe adresa de broadcast cu IP-ul al
          carui MAC vrea sa il gaseasca; cand va primi un ```ARP_REPLY```, atunci va
          trimite pachetul pus in coada (am presupus ca pentru fiecare pachet
          pus in coada se va primi un ```ARP_REPLY```, iar routerul nu va mai primi
          intre timp alte ARP_REPLY ce nu au legatura cu pachetul pus in coada)
        * daca exista in tabela ARP o intrare potrivita, modifica adresele MAC
          sursa si destinatie si trimite pachetul
