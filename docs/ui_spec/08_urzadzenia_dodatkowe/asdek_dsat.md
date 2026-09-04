# Interfejs Terminala Diagnostyki Taboru (DSAT / ASDEK)

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję w programie Terminala DSAT/ASDEK. Każde pole, tekst, przycisk oraz okienko są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

---

## 1. Główne Okno Aplikacji (klasa DsatMainWindow)

Rozmiar minimalny: 1024x768, wyśrodkowane na pełnym ekranie. Tło okna: ciemnoszare (`#2A2A2A`).

### 1.1 Górny pasek statusu (Top Panel)
Pasek na czarnym tle (`#000000`) z białym tekstem.
* **Logo systemu `ISDR/SSD`**: Z lewej strony, obrazek 40x40 px z napisem "SIECIOWY SYSTEM DIAGNOSTYKI TABORU".
* **Wskaźnik stanu systemu**: Ikona w kształcie kwadratu. Zielone `N` oznacza tryb normalny.
* **Zegar i Data (DigitalClock)**: Wyświetlacz w kolorze jaskrawozielonym (`#00FF00`) po prawej stronie.

### 1.2 Górny panel wyboru zespołów torowych (Stacji)
Tło szare (`#333333`).
* **Przycisk stacji pomiarowej `[POCIĄG OK]` / `[Alarm]`**:
  * Tło: Domyślnie zielone (`#00CC00`). W stanie alarmu zmienia się na czerwone (`#CC0000`).
  * Kliknięcie LPM: Przełącza kontekst głównego ekranu logów na tę stację.
* **Przycisk podrzędny `[TEST OK]`**:
  * Kliknięcie LPM w przycisk pod wybraną stacją wywołuje procedurę autodiagnostyki i drukuje logi na głównym ekranie. Status stacji pozostaje jako zielony.

### 1.3 Główny ekran logów/raportów (TerminalConsole)
Centralna sekcja symulująca wydruk terminala (monospaced font).
* **Tło Konsoli**: 
  * W stanie braku awarii: Tło zielone (`#006600`), czcionka biała lub jasnozielona.
  * W stanie awarii: Tło czerwone (`#990000`), czcionka biała. Oznacza przekroczenie temperatury koła/maźnicy lub płaskie koło (status `OSTR` lub `STOP/GRAN`).
* **Potwierdzenie rejestru (Małe czerwone okienko z symbolem X)**:
  * Pojawia się w lewym górnym rogu obok nagłówka raportu. Gdy raport w ERSAT zostanie całkowicie uzupełniony przez dyżurnego, okienko zmienia status na zrealizowany poprzez pojawienie się w nim znaku `X`.

### 1.4 Panel informacyjny po prawej (Szczegóły - DetailsPanel)
Tło czarne (`#000000`). Pola tylko do odczytu:
* `Lokalizacja:` np. `LST Testowo`.
* `Stacja:` np. `SD Piaski 1`.
* `Typ:` `ASDEK/PM/GM/GH/OK`.
* `V[km/h]:` Wartość prędkości pociągu (np. `115`).
* `OSIE:` Liczba osi.
* **Numer pociągu:** [numer pociągu].

### 1.5 Dolny panel przycisków funkcyjnych (FunctionButtonsToolbar)
Rozkład na dwa rzędy przycisków (szare z czarnym tekstem).
* **Rząd 1:**
  * **Przycisk `[ALARM - F1]`**: Kliknięcie LPM lub wciśnięcie klawisza `F1` wycisza ciągły sygnał dźwiękowy na stanowisku. Czerwone tło logów oraz czerwony przycisk stacji pozostają aktywne do momentu rozliczenia alarmu.
  * **Przycisk `[TEST - F2]`**: Kliknięcie LPM lub `F2` uruchamia test.
  * **Przycisk `[RAPORTY - F3]`**: Kliknięcie LPM lub `F3` otwiera logikę raportów.
  * **Przycisk `[SERWIS - F5]`**: Kliknięcie LPM lub `F5`.
  * **Przycisk `[ARCHIWUM - F10]`**: Kliknięcie LPM lub `F10`.
* **Rząd 2:**
  * **Przycisk `[NUMER POCIĄGU - F4]`**: Kliknięcie LPM lub `F4`.
  * **Przycisk `[POWTÓRZ RAPORT - F6]`**: Kliknięcie LPM lub `F6`.
  * **Przycisk `[RAPORT GRAFICZNY - F7]`**: Kliknięcie LPM lub `F7` otwiera nowe okienko z wykresem.
  * **Przycisk `[HASŁO - F8]`**: Kliknięcie LPM lub `F8`.
  * **Przycisk `[ERSAT - F9]`**: Kliknięcie LPM lub `F9` otwiera okno modalne Elektronicznego Rejestru `ErsatWindow`.
* Na dole po lewej małe pole statusu `[ERSAT]` (tylko do odczytu).

### 1.6 Ostrzeżenia Dźwiękowe i Pływające
Wykrycie stanu awaryjnego wymusza: zmianę przycisku stacji (góra) na `[Alarm]` (czerwone tło), okno logów zmienia tło na czerwone drukując usterkę, nastawnia uruchamia ciągły sygnał dźwiękowy.

---

## 2. Okno Modalne "Raport Graficzny" (klasa GraphicReportWindow)

Okno wyskakuje na środku (CenterOwner), wymiary około 800x600 pikseli.
* **Pasek tytułowy**: Z czerwonym krzyżykiem `[X]`. Kliknięcie LPM zamyka okno.
* **Wykres główny (Chart)**: Przedstawia na wykresie słupkowym/liniowym zmierzone temperatury maźnic i hamulców.
* **Przycisk `[Powrót]`**: Nasłuchuje zdarzenia wciśnięcia klawisza `Escape` w celu zamknięcia okna. Kliknięcie LPM niszczy całe okno bez zmian.

---

## 3. Okno Modalne "Elektroniczny Rejestr (ERSAT)" (klasa ErsatWindow)

Okno wyskakuje na środku (CenterOwner). Posiada tło szare. Dyżurny musi tu uzupełnić dane o usterce.

### 3.1 Pola danych rejestru
* **Pole tekstowe `[Numer pociągu]` (TextBox)**: Kliknięcie LPM aktywuje kursor, wpisywanie z klawiatury.
* **Pole tekstowe `[Dane potwierdzającego]` (TextBox)**: Kliknięcie LPM aktywuje kursor do wpisania imienia i nazwiska.
* **Pole tekstowe `[Numer pojazdu (EVN)]`**: Wpisanie numeru.
* **Pole tekstowe `[Przewoźnik]`**: Wpisanie nazwy przewoźnika.
* **Lista rozwijana `[Wynik weryfikacji]` (ComboBox)**:
  * Kliknięcie LPM w białe tło rozwija listę opcji, m.in. „Kontynuacja jazdy” lub „Wyłączenie pojazdu z ruchu”.
  * Najechanie (Hover) na pozycję podświetla ją.
  * Kliknięcie LPM w pozycję wybiera ją i zamyka listę rozwijaną.

### 3.2 Dolne Przyciski Okna
* **Przycisk `[Zapisz przekroczenie]`**: Kliknięcie LPM zapisuje wpisane w polach dane dla konkretnego, zaznaczonego w rejestrze przekroczenia. Procedurę należy powtórzyć dla każdego przekroczenia z raportu.
* **Akcja Zatwierdzenia całości (Koniec pracy)**: Gdy wszystkie usterki zostaną przypisane i zapisane, okno ERSAT zamyka się. Powraca główne okno, a małe czerwone pole w głównym oknie logów obok raportu aktualizuje się o znak `X`. Oznacza to pełne rozliczenie alarmu przez dyżurnego ruchu.
