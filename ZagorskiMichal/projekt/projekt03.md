# Projekt 3. Rozproszony system do monitorowania funkcjonowania grupy komputerów.

* **Mechanizmy:** sockety, tworzenie procesów, funkcje systemowe do pozyskiwania informacji o stanie systemu, uruchamiania procesów, komunikacji między nimi
* **Architektura:** grupa demonów uruchomionych na różnych komputerach, klient (konsola) konfiguruje demona i pobiera od niego w zadanych odstępach czasu zadane informacje
* **Monitorowane informacje:** zalogowani użytkownicy, uruchomione procesy, zmiany plików, zajętość pamięci i inne publicznie dostępne informacje z poziomu użytkownika.