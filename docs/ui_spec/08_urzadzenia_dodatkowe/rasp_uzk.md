# Interfejs Urządzenia Zdalnej Kontroli RASP-UZK

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję w programie RASP-UZK, zbudowanym przy użyciu wirtualnego rysowania i pętli zdarzeń graficznych. Każde pole, tekst, przycisk oraz okienko są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

---

## 1. Główne Okno Aplikacji (Widok Ogólny) (klasa RaspMainWindow)

Rozmiar minimalny: 1024x768, wyśrodkowane. Tło okna: jasnoszare (`#E0E0E0`).

### 1.1 Tabela Przejazdów (DataGrid z układem pionowym)
Główny obszar to siatka prezentująca stan przejazdów w systemie. Każda kolumna odpowiada jednemu przejazdowi (np. `km 22.578`).
* **Przycisk nagłówka (Nazwa przejazdu np. `[km 22.578]`)**: 
  * Tło szare. Na "Hover" podświetlenie krawędzi.
  * Kliknięcie LPM: Przełącza widok z tabeli na Okno Szczegółowe dla tego konkretnego przejazdu.
* **Wiersze statusowe (Kontrolki kwadratowe z tekstem, tylko do odczytu)**:
  * Najechanie (Hover) na którekolwiek z poniższych pól nie wywołuje akcji.
  * **STEROWANIE**:
    * Zielone tło (`#00FF00`) - sterowanie automatyczne.
    * Pomarańczowe tło (`#FFA500`) - sterowanie lokalne.
  * **STAN**:
    * Szare tło (`#C0C0C0`) - oczekiwanie (brak pociągu).
    * Fioletowe tło (`#8A2BE2`) - ostrzeganie (przejazd w fazie zamykania lub zamknięty).
  * **WYZEROWANIE**:
    * Szare - stan zasadniczy.
    * Fioletowe - urządzenia wyzerowane, system oczekuje na pierwszy pociąg.
  * **USTERKA I (usterki pilne)**:
    * Szare - stan zasadniczy (brak usterek).
    * Czerwone migające (Blinking Animation) - usterka kategorii I.
  * **USTERKA II (usterki ostrzegawcze)**:
    * Szare - stan zasadniczy.
    * Czerwone stałe (`#FF0000`) - usterka kategorii II.
  * **TRANSMISJA**:
    * Szare - transmisja poprawna (OK).
    * Czerwone - brak transmisji.
  * **TOR I / TOR II**:
    * Szare - czujniki załączone.
    * Czerwone - czujniki wyłączone.

---

## 2. Okno Szczegółowe Przejazdu (klasa RaspDetailWindow)

Ekran ten nakłada się na główny interfejs (widok typu pełny ekran danej zakładki). Otwiera się po wybraniu konkretnego kilometrażu przejazdu.

### 2.1 Rząd wskaźników usterek (Górny Pasek)
Zestaw lampek kontrolnych na górze (prostokątne lub okrągłe obiekty). Świecą na szaro w stanie spoczynku lub czerwono przy wystąpieniu problemu.
* Kontrolki: `[Awaria SYSTEMU]`, `[Awaria CZUJNIKÓW]`, `[Awaria NAPĘDU]`, `[Wyłam. DRĄGA]`, `[Awaria SYGNALIZAT.]`, `[Rozładowane AKUM.]`, `[Brak transm.]`, `[Awaria SRK]`. Tylko odczyt.

### 2.2 Plan schematyczny przejazdu (Środek)
Centralna sekcja okna prezentująca graficzne odwzorowanie przejazdu.
* Tor 1 i Tor 2 z zaznaczonymi kierunkami (np. Rudzienice Suskie -> Iława Główna).
* **Sygnalizatory drogowe (`S1`, `S2`, `S3`, `S4`)**: Kropki animowane dynamicznie (zmiany kolorów).
* **Rogatki (`N1`, `N2`, `N3`, `N4`)**: Pola fioletowe zmieniające kąt nachylenia/stan wizualny w zależności od pozycji drąga.
* **Czujniki (np. `C I` do `C XIV`)**: Zmiana koloru stref w momencie przejazdu pociągu (wszystkie obiekty są zablokowane na kliknięcia, służą tylko do odczytu).

### 2.3 Panel sterowania i poleceń (Lewa Strona)
Przyciski interaktywne po lewej stronie ekranu.
* **Przycisk `[Zezwolenie na ster. LOKALNE]`**: Szary. Kliknięcie LPM zmienia status sterowania.
* **Wskaźnik `[Sterowanie AUTOMATYCZNE]`**: Tylko odczyt (zielone pole).
* **Przyciski blokady torów `[T1 ZAŁ.]` / `[T1 WYŁ.]` oraz `[T2 ZAŁ.]` / `[T2 WYŁ.]`**:
  * Kliknięcie LPM w `[T1 WYŁ.]`: Wciska się wizualnie. Dezaktywuje automatyczne czujniki dla Toru 1 (rogatki nie będą się automatycznie zamykać). Pole statusowe "TOR I" zmienia kolor na czerwony w Oknie Ogólnym.
* **Przycisk `[ZEROWANIE]`**:
  * Szary przycisk.
  * Kliknięcie LPM: Wysyła polecenie resetu urządzeń po awarii/wyłamaniu. System wraca do normy, a pole "WYZEROWANIE" w Oknie Ogólnym zapala się na fioletowo.

### 2.4 Konsola zdarzeń (Dolna część - EventLogTextBox)
Lista zdarzeń bieżących na samym dole.
* Pole tekstowe ustawione tylko do odczytu. 
* Kliknięcie LPM aktywuje w nim kursor i pozwala na zaznaczenie oraz kopiowanie logów (`Ctrl+C`), m.in. z formacie `2020-06-15 06:45:51 OCZEKIWANIE`.
* Posiada wbudowany pionowy pasek przewijania `ScrollBar`.

### 2.5 Dolny Pasek Przełączania
Zlokalizowany pod konsolą, zapewnia nawigację.
* **Przyciski szybkiego przełączania (np. `[km. 245.768]`, `[km. 241.768]`)**:
  * Kliknięcie LPM w przycisk: Natychmiast zmienia zawartość wyświetlaną w obecnym Oknie Szczegółowym na schemat i układ wybranego przejazdu (bez wracania do siatki).
* **Przycisk `[Wszystkie]`**: 
  * Kliknięcie LPM: Ukrywa/niszczy widok Okna Szczegółowego i powoduje powrót do widoku głównej tabeli (Okna Ogólnego).
