# Łączność Telefoniczna - Telefon Stacjonarny

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję wirtualnego telefonu stacjonarnego na biurku dyżurnego ruchu, zbudowanego z użyciem uniwersalnego zarządzania zdarzeniami (pętla zdarzeń). Każde pole, tekst, przycisk oraz kontrolka są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

---

## 1. Wygląd telefonu (Widok główny)

Telefon stanowi wierne, wirtualne odwzorowanie fizycznego aparatu telefonicznego (bez logo) opartego na płaskim rzucie z góry. Tło wokół telefonu: biurko z delikatnym gradientem szarości (`#E5E5E5`).

### 1.1 Słuchawka (Lewa strona aparatu)
* **Grafika słuchawki telefonicznej na kablu spiralnym.**
  * Stan domyślny (spoczynek): Słuchawka leży na widełkach.
  * Stan podniesiony: Słuchawka jest obrócona o 15 stopni i przesunięta wizualnie w górę. Rzuca cień.
* **Interakcja (Podniesienie/Odłożenie słuchawki):**
  * Najechanie (Hover): Kursor zmienia się na łapkę (np. zmianą wskaźnika systemowego).
  * Kliknięcie LPM (słuchawka odłożona): Podnosi słuchawkę. Odtwarza się dźwięk zwolnienia widełek. W głośnikach komputera słychać ciągły sygnał zgłoszenia centrali (wolny ton 425 Hz). Na wyświetlaczu LCD pojawia się napis: `Wybierz numer...`.
  * Kliknięcie LPM (słuchawka podniesiona): Odkłada słuchawkę na widełki. Dźwięk odłożenia. Trwające połączenie zostaje przerwane. Dioda „IN USE” gaśnie. Ekran LCD wraca do ekranu głównego (Zegar).

### 1.2 Ekran LCD i Diody (Górna część, środek)
* **Wyświetlacz LCD (Monochromatyczny, zielono-żółte podświetlenie):**
  * Tło: `#90A86E`. Tekst: `#000000`, czcionka naśladująca 7-segmentowy LCD (np. `Digital-7`). Rozmiar 140x50 pikseli.
  * W stanie spoczynku: Wyświetla aktualną godzinę i datę (np. `12:34 25/10`).
  * W trakcie wybierania: Wyświetla wpisywane cyfry.
  * W trakcie połączenia: `Połączenie z: Dyżurny B` lub `Czas: 01:23`.
* **Dioda NEW (Po lewej stronie ekranu):**
  * Kolor wyłączonej diody: `#4A0000`.
  * Gdy jest nieodebrane połączenie: Miga na jasnoczerwono (`#FF0000`) z częstotliwością 1Hz (500ms zapalona, 500ms zgaszona).

### 1.3 Przyciski pod wyświetlaczem
Zestaw 5 małych przycisków w rzędzie, pod ekranem LCD. Najechanie nie powoduje zmiany tła. Wciśnięcie LPM powoduje wizualne "wduszenie" przycisku (zmiana cienia) i dźwięk kliknięcia mechanicznego.
* **Przycisk `[DEL]`**: Kliknięcie LPM podczas wybierania numeru usuwa ostatnią wpisaną cyfrę z ekranu LCD.
* **Przycisk `[VIP]`**: Kliknięcie LPM zmienia ekran LCD na listę numerów VIP, po której można nawigować przyciskami UP/DOWN.
* **Przycisk `[HOLD]`**: Kliknięcie LPM w trakcie rozmowy zawiesza ją. Rozmówca słyszy muzykę. Ekran LCD wyświetla `Rozmowa zawieszona`. Ponowne kliknięcie wznawia rozmowę.
* **Przycisk `[MEMO]`**: Kliknięcie LPM wyświetla rejestr ostatnich połączeń na ekranie LCD.
* **Przycisk `[STORE]`**: Kliknięcie LPM z wpisanym numerem na ekranie otwiera menu przypisywania numeru pod szybkie wybieranie.

### 1.4 Klawiatura numeryczna i funkcyjna (Środek i prawa strona)
* **Przyciski numeryczne `0`-`9`, `*`, `#`**:
  * Kliknięcie LPM: Odtwarza odpowiedni ton DTMF dla danego klawisza. Na ekranie LCD pojawia się kliknięta cyfra/znak. Przycisk wizualnie się wdusza.
* **Dioda "IN USE" (Nad prawym panelem funkcyjnym):**
  * Świeci na jaskrawoczerwono, gdy podniesiono słuchawkę lub wciśnięto przycisk `SPK` (aktywna linia). Zgaszona w stanie spoczynku.
* **Przyciski funkcyjne (Prawa kolumna):**
  * Przycisk `[VOL]` (podzielony na góra/dół): Kliknięcie LPM w górną połowę zwiększa głośność dzwonka/rozmowy, w dolną – zmniejsza. Zmiana ilustrowana paskiem postępu na LCD (np. `VOL: |||||`).
  * Przycisk `[MUTE]`: Kliknięcie LPM wycisza mikrofon. Na przycisku zapala się mała czerwona dioda led. Rozmówca nas nie słyszy. Ponowne kliknięcie wyłącza mute.
  * Przycisk `[FLASH]`: Kliknięcie LPM podczas rozmowy wysyła sygnał przerwy kalibrowanej (Flash) do centrali (zmiana linii).
  * Przycisk `[REDIAL]`: Kliknięcie LPM po odłożeniu i podniesieniu słuchawki natychmiastowo wybiera ponownie ostatni numer, wyświetlając go na LCD.

### 1.5 Przyciski nawigacyjne (Dół)
* **Przyciski `[UP]` i `[DOWN]`**: Kliknięcie LPM służy do przewijania listy kontaktów w pamięci lub listy VIP na ekranie LCD.
* **Przycisk `[SET/DIAL]`**: Kliknięcie LPM podczas wybierania numeru lub wyszukania kontaktu w pamięci, inicjuje połączenie (odpowiednik wciśnięcia zielonej słuchawki na komórce). Ekran zmienia na `Łączenie: [Numer]...`.
* **Przycisk `[ABC]`**: Kliknięcie LPM w trybie wyszukiwania przełącza klawiaturę numeryczną w tryb alfanumeryczny T9, pozwalając na wpisanie liter nazwy kontaktu.

### 1.6 Panel Szybkiego Wybierania (Prawa krawędź aparatu)
* **10 pionowych przycisków membranowych (Od `M1` do `M10`)**:
  * Każdy przycisk posiada prostokątne pole na papierową etykietę, np. `Dyżurny Stacji B`.
  * Po lewej stronie każdego przycisku jest dioda LED.
  * Kiedy stacja dzwoni (np. `M1`): Dioda `M1` miga szybko na czerwono. Słychać dźwięk dzwonka. Ekran LCD wyświetla `Przychodzące: Dyżurny B`.
  * **Odebranie połączenia:** Kliknięcie LPM w przycisk `M1` (lub podniesienie słuchawki). Dioda `M1` zapala się na stałe. Połączenie nawiązane.
  * **Wywołanie (Wychodzące):** W stanie spoczynku, kliknięcie LPM w `M1` automatycznie podnosi linię w trybie głośnomówiącym, przypisany numer zostaje wybrany, dioda `M1` świeci, dioda `IN USE` świeci, a głośnik transmituje sygnał wołania.
* **Przycisk `[SPK]` (Speaker / Głośnomówiący) na samym dole**:
  * Kliknięcie LPM: Działa jak podniesienie/odłożenie słuchawki. Włącza tryb głośnomówiący (zapala się dioda nad `SPK` i dioda `IN USE`).
