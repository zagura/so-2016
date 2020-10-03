# Systemy operacyjne 2015/2016

## Tematy projektów

### Projekt 1. Sterownik urządzenia w systemie Linux

Realizacja sterownika obsługi wybranego urządzenia. Programowanie na poziomie jądra. Użycie dynamicznie ładowanych modułów jądra. Proponowane urządzenie - RAM dysk lub inne urządzenie wirtualne. Dla chętnych oczywiście dopuszczalne oprogramowanie sterownika dla urządzenia fizycznego,

### Projekt 2. Implementacja systemu bazy danych o scentralizowanej i zdecentralizowanej architekturze. Analiza wydajności.
**Założenia:** na podstawie rozdziału 16 [Stevens], dostęp do danych jest zcentralizowany (jeden proces bezpośrednio odwołuje się do plików bazy danych),zdecentralizowany (wiele procesów może odwoływać się do plików z bazą danych).

Informacje zawarte w bazie danych: dowolne, baza danych realizowana przez kilka plików tekstowych imitujących tabele i powiązania między nimi.

**Mechanizmy:** komunikacja między procesami np. sokety,mechanizm zajmowania/ryglowania rekordów.

### Projekt 3. Rozproszony system do monitorowania funkcjonowania grupy komputerów.

**Mechanizmy**: sockety, tworzenie procesów, funkcje systemowe do pozyskiwania informacji o stanie systemu, uruchamiania procesów, komunikacji między nimi

**Architektura:** grupa demonów uruchomionych na różnych komputerach, klient (konsola) konfiguruje demona i pobiera od niego w zadanych odstępach czasu zadane informacje

**Monitorowane informacje:** zalogowani użytkownicy, uruchomione procesy, zmiany plików, zajętość pamięci i inne publicznie dostępne informacje z poziomu użytkownika.

### Projekt 4. Rozproszony szel

**Założenia:** grupa szeli i aktywnych szeli, celem jest wykonywanie zleconego polecenia (lub grupy poleceń) na różnych maszynach zdalnych, na których są aktywne demony szela, wyniki wykonania poleceń na maszynach zdalnych powinny być wyświetlane na na maszynie lokalnej.

**Architektura:** szele klienta + demony

### Projekt 5. Środowisko do kooperatywnej edycji tekstów
**Procesy:** kilka procesów edycyjnych operujących na pliku

**Założenia:** rozproszony edytor (sockety), celem jest modyfikacja w jednym czasie tego samego pliku, blokowanie określonych opcji... przy

**Sposoby rozwiązania**: GTK lub QT.


### Projekt 6. Loader do bezpiecznego wykonywania programów. 

**Założenia:**

Napisanie programu (loadera), który będzie uruchamiał podany program X i pozwalał na jego wykonanie z następującymi założeniami: 
1. Program X ma mieć ograniczony rozmiar maksymalnej możliwej do  wykorzystania pamięci operacyjnej. 
2. Program X nie może wykonywać się dłużej, niż ustalony czas 
3. Program X może dokonywać odczytu ze standardowego wejścia lub z plików, do których ma uprawnienia (kontrolowane przez program loader). 
4. Jw. z plikami do zapisu 
5. Program X nie może korzystać z funkcji systemowych (takich jak fork, exec, operacje na soketach, sygnały, potoki), które mogłyby narazić system, w jakim jest uruchamiany na problemy lub wyciek informacji z systemu (np. pliku /etc/passwd) 
6. Należy założyć, iż program X jest napisany w różnych językach programowania (C, C++, Pascal - FreePascal, Icon) i zlinkowany z różnymi bibliotekami. W związku z tym należy zwrócić uwagę na procedury ładujące i uruchamiające program napisany w tych językach.
7. Program loader powinien zwracać kod wyjścia równy kodowi wyjścia uruchamianego programu (w przypadku gdy wszystko było ok), lub określone kody wyjścia (np. 250, 251,...) w przypadku naruszenia założeń z pkt. 1..5. W drugim przypadku na standardowe wyjście błędu powinien też zostać wypisany krótki komunikat wyjaśniający przyczynę zakończenia programu X.

Wszystkie parametry konieczne do uruchomienia programu X powinny być przekazywane do programu `loader` przez linię poleceń, przykładowo: `loader <nazwa_programu_X> -maxmem 10MB -maxtime 20`
**Mechanizmy:**

sterowanie procesami (fork, exec, wait, exit), ustalanie ograniczeń wykonywania procesu (getrusage, setrusage), analiza wywołań systemowych (ptrace)


### Projekt 7. Monitorowanie i analiza przydziału pamięci dla procesu
Należy przygotować bibliotekę umożliwiającą analizę zużycia pamięci przez proces (w KB/MB oraz stronach). Należy przeanalizować uruchomienie procesu, operacje alokacji zasobów (*alloc/malloc/realloc/new* itp.) oraz ich odpowiedniej dealokacji, ładowanie biblitek dynamicznych oraz stosowanie mechanizmów pamięci współdzielonej (*shm*) oraz mapowania plików w pamięci *mmap*, a także obserwować wykonanie funkcji *brk*. Należy wykorzystać polecenie *strace*, funkcję i strukturę *mallinfo* oraz funkcję *mallopt*. Przeanalizować zajętość pamięci (i pola struktury *mallinfo*) dla różnych parametrów konfigurujących i uruchamiać w celu analizy kilka własnych testowych aplikacji oraz wybrane dostępne aplikacje .

### Projekt 8. Analiza przebiegu i efektywności szeregowania procesów w systemie Linux dla różnych polityk szeregowania.
Należy przygotować różne zestawy procesów działające według różnych polityk szeregowania i wykonujące różne akcje a następnie przeprowadzić i przeanalizować odpowiednie eksperymenty.

### Projekt 9. Napisać środowisko do demonstracji algorytmów rozwiązywania kilku wybranych mechanizmów synchronizacyjnych
Przykładowe problemy: producenci/konsumenci, czytelnicy/pisarze, 5 filozofów, lotniskowiec, śpiący fryzjer itp.

### Projekt 10. Monitorowanie grupy komputerów, użycie mechanizmu FUSE.

Wykorzystując bibliotekę FUSE (http://fuse.sourceforge.net) zaimplementować pseudo system plików do monitorowania komputerów w sieci.

**Założenia:**
* Jeden serwer i wiele klientów
* Na serwerze działa demon, który odpytuje klientów o podstawowe informacje systemowe (zalogowani użytkownicy, uruchomione procesy, obciążenie systemu, otwarte pliki, aktualna data/godzina, itd)
* Informacje te udostępniane są na serwerze w postaci pseudo systemu plików. Po zamontowaniu, tego systemu plików widzimy katalogi nazwane tak jak adresy IP klientów. W katalogach są podkatalogi do opisu procesów, zalogowanych użytkowników/etc. Np. 1.2.3.4/proc może opisywać procesy na kliencie o adresie 1.2.3.4 W tym katalogu mogą być pliki nazwane tak jak PIDy uruchomionych procesów (ich treścią byłyby informacje o procesie - nazwa polecenia/zajęta pamięć/czas/itp)
* Na klientach działają demony, które rejestrują się na serwerze a następnie odpowiadają na zapytania serwera (serwer wysyła zapytania, gdy ktoś przegląda pseudo system plików)
* System plików tylko do odczytu
* Komunikacja po TCP

### Projekt 11. Analiza i porównanie mechanizmów wątków w systemach Uniksowych, ich możliwości i wydajności
Porównanie realizacji wątków w systemie Linux dla różnych wersji jądra. Możliwe użycie innych systemów uniksowych jak *Open Solaris* lub *FreeBSD*.


### Projekt 12. Symulacja tunelu kolejowego przy użyciu mechanizmów systemowych

**Założenia:** 1 tor w tunelu, kilka linii kolejowych prowadzących do/z tunelu (np. po dwie dwukierunkowe z każdej strony), generator generuje pociągi o różnych priorytetach (ekspres, pospieszny, towarowy), pociąg jest procesem,struktura w pamięci wspólnej opisuje sytuację (położenie) na torach wjazdowych do tunelu, priorytety są rozstrzygane w momencie decyzji, kto ma wjechać do i z tunelu.

**Procesy:** zarządca dopuszczający pociągi do wjazdu, pociągi, korzystamy z monitorów-semaforów, zarządca powinien posiadać informację od pociągu, jeśli/czy ten chce wjechać do tunelu, oraz odnośnie wjazdu i wyjazdu z tunelu



### Projekt 13. Symulacja sieci komunikacyjnej przy użyciu mechanizmów systemowych

**Założenia:** ludzie podróżujący autobusami, autobusy przyporządkowane do linii (A, B, C...), co określa przystanki i przystanki przesiadkowe, kilka przystanków buduje trasę w ramach każdej linii, rozkład jazdy w pliku,informacje o punktach przesiadkowych,

**Cel:**  pasażer chce przejechać z punktu X do Y.

**Procesy:** autobusy i przystanki (które generują nowe zlecenia transportowe oraz przechowują informacje o pasażerach przebywających na danym przystanku i ich celach podróżny)...


### Projekt 14. Symulacja zakładu napraw samochodów przy użyciu mechanizmów systemowych

**Mechanizmy:** sokety, kolejki komunikatów, pamięć dzielona, potoki sygnały i inne, funkcje zarządzania procesami.

**Procesy:** zakład samochodowy przyjmujący zlecenia, procesy-mechanicy dokonujący napraw,proces-generator zleceń napraw,

**Zasoby dzielone:**stanowiska do napraw samochodów (ograniczona ilość)

**Założenia:** proces-tworzy procesy mechaników, tworzony jest zasób dzielony, zawierający stanowiska napraw samochodu, proces: generator samochodów o dowolnym rozkładzie, samochód może być reprezentowany przez ciąg znaków,

**Uzupełnienia:** różne rodzaje napraw, o większej lub mniejszej złożoności


### Projekt 15. Symulacja lotnisk przy użyciu mechanizmów systemowych

Koncepcja analogiczna do tej w projekcie 14.

### Projekt 16. Program do tworzenia kopii zapasowych na wzór Dropboxa
Program kliencki powinien monitorować zadany folder i synchronizować zawartość z serwerem, a także pobierać zmiany wysłane przez inne programy klienckie na serwer. Serwer powinien tworzyć wersje zmienionych plików tak, aby można było cofnąć zmiany w pliku do wybranej wersji z zadanego dnia.


### Projekt 17. Serwer HTTP
Napisać wielowątkowy serwer HTTP z możliwością konfiguracji w pliku tekstowym. Program loguje swoją aktywność do plików *.log. Program należy napisać z wykorzystaniem socketów i wątków. Należy zaimplementować obsługę zapytań GET i POST oraz standardowe kody odpowiedzi. Realizacja obiektowa w języku C++. Serwer powinien obsługiwać popularne sygnały np. SIGINT. Program nie może korzystać z gotowej biblioteki do tworzenia serwera.


### rojekt 18. Implementacja (uproszczonego) repozytorium kodu.
Projekt polega na zaimplementowaniu (uproszczonej) wersji sieciowego repozytorium kodu z wielodostępem. Rozwiązanie powinno umożliwiać (oczywiście w uproszczonej formie) pobranie kodu źródłowego, śledzenie i zapisywanie zmian w repozytorium, notyfikowanie o wprowadzonych zmianach. Architektura oraz lista dostępnych funkcjonalności wg własnego projektu. 

**Mechanizmy:** komunikacja między procesami np. sokety,mechanizm zajmowania/ryglowania rekordów.

### Projekt 19. Sieciowa gra w statki - wersja Linux.

Projekt polega na zaimplementowaniu sieciowej wersji gry w statki umożliwiającej prowadzenie rozgrywki wieloosobowej. Warstwa wizualizacji oraz walidacji reguł gry może być uproszczona należy się natomiast skoncentrować na wykorzystaniu różnych mechanizmów systemowych (sockety, sygnały etc). Projekt należy zaimplementować możliwie niskopoziomowo (C) wykorzystując jedynie biblioteke(i) do budowy interfejsu użytkownika. Poza funkcjonalnościami typu przyłączenie i wyjście z gry, rozpoczęcie, prowadzenie i zakończenie rozgrywki możliwe powinno być także zapisywanie oraz dostęp do (historii) wyników.  


### Projekt 20. Sieciowa gra w statki - wersja Windows.

Projekt polega na zaimplementowaniu sieciowej wersji gry w statki umożliwiającej prowadzenie rozgrywki wieloosobowej. Warstwa wizualizacji oraz walidacji reguł gry może być uproszczona należy się natomiast skoncentrować na wykorzystaniu różnych mechanizmów systemowych (sockety, sygnały etc). Projekt należy zaimplementować możliwie niskopoziomowo (C) wykorzystując jedynie biblioteke(i) do budowy interfejsu użytkownika. Poza funkcjonalnościami typu przyłączenie i wyjście z gry, rozpoczęcie, prowadzenie i zakończenie rozgrywki możliwe powinno być także zapisywanie oraz dostęp do (historii) wyników.  

### Projekt 21. Sieciowa gra w kółko-i-krzyżyk - wersja Linux.

Projekt polega na zaimplementowaniu sieciowej wersji gry w kółko-i-krzyżyk. Warstwa wizualizacji oraz walidacji reguł gry może być uproszczona należy się natomiast skoncentrować na wykorzystaniu różnych mechanizmów systemowych (sockety, sygnały etc). Projekt należy zaimplementować możliwie niskopoziomowo (C) wykorzystując jedynie biblioteke(i) do budowy interfejsu użytkownika. Poza funkcjonalnościami typu przyłączenie i wyjście z gry, rozpoczęcie, prowadzenie i zakończenie rozgrywki możliwe powinno być także zapisywanie oraz dostęp do (historii) wyników. Rozgrywka powinna być prowadzona na dużej (nieskończonej) planszy. 

### Projekt 22. Sieciowa gra w kółko-i-krzyżyk - wersja Windows.

Projekt polega na zaimplementowaniu sieciowej wersji gry w kółko-i-krzyżyk. Warstwa wizualizacji oraz walidacji reguł gry może być uproszczona należy się natomiast skoncentrować na wykorzystaniu różnych mechanizmów systemowych (sockety, sygnały etc). Projekt należy zaimplementować możliwie niskopoziomowo (C) wykorzystując jedynie biblioteke(i) do budowy interfejsu użytkownika. Poza funkcjonalnościami typu przyłączenie i wyjście z gry, rozpoczęcie, prowadzenie i zakończenie rozgrywki możliwe powinno być także zapisywanie oraz dostęp do (historii) wyników. Rozgrywka powinna być prowadzona na dużej (nieskończonej) planszy. 

### Projekt 23. Implementacja komunikatora sieciowego dla systemu Android.

Projekt polega na zaimplementowaniu komunikatora sieciowego dla systemu Android. Strona serwerowa rozwiązania może zostać zaimplementowana na dowolnej platformie (można skorzystać z gotowych bibliotek/serwerów), natomiast strona kliencka powinna zostać zaimplementowana możliwie niskopoziomowo (natywnie) na system Android. Zakres funkcjonalny oraz interfejs użytkownika mogą być uproszczone do niezbędnego minimum natomiast Projekt powinien koncentrować się na wykorzystaniu (różnych) (niskopoziomowych) mechanizmów systemowych.

### Projekt 24. Porównanie wydajności kilku różnych implementacji dynamicznej alokacji pamięci

Projekt polega na porównaniu wydajności kilku implementacji funkcji *malloc*. Testy wykonać należy dla kilku różnych profili użycia:

* programy jedno- i wielowątkowe
* wiele drobnych alokacji vs mniejsza ilość większych
* sama alokacja vs alokacja przemieszana ze zwalnianiem

Należy porównać czas działania, skalowalność, narzut pamięciowy.

Implementacje do porównania:

* standardowy malloc dostępny w systemie Linux
* Hoard malloc (https://github.com/emeryberger/Hoard)
* TCMalloc (http://goog-perftools.sourceforge.net/doc/tcmalloc.html)

Można dodatkowo przetestować też jakieś inne, np. dlmalloc (ftp://g.oswego.edu/pub/misc/malloc.c)

Należy przetestować różne konfiguracje rozważanych alokatorów. W opracowaniu powinien znaleźć się opis wykorzystywanych przez nie algorytmów.
