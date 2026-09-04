# Interfejs Podglądu Monitorów (LCS)

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję w mechanizmie Podglądu Monitorów na stanowisku dyżurnego ruchu. Każde pole, tekst, przycisk oraz okienko są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

---

## 1. Główne Okno Aplikacji (Wirtualne Biurko - klasa DeskView)

Rozmiar minimalny: dostosowuje się do ekranu roboczego (pełny ekran). Tło okna imituje teksturę roboczego stanowiska / blatu.

### 1.1 Siatka Miniatur Monitorów (MonitorGrid)
Siatka układająca od 3 do 6 reprezentacji monitorów dyżurnego.
* **Miniatura Monitora (Kontener `MonitorThumbnail`)**: 
  * Wyświetla pomniejszony podgląd działania konkretnego systemu na żywo (np. mały EDR, pulpit stacji A, CCTV).
  * **Etykieta nad monitorem (Label)**: Np. "Monitor 1 – Pulpit nastawczy stacji TESTOWO". Tylko odczyt.
  * **Najechanie (Hover)** na obszar miniatury: Powoduje delikatne podświetlenie kontenera (biała lub jasnoszara obwódka).
  * **Kliknięcie LPM na obszarze miniatury**: Rozpoczyna płynną animację powiększania (Zoom-in), aż dany monitor zajmie 100% ekranu roboczego (przejście do trybu pełnoekranowego).

### 1.2 Przyciski podrzędne (w obrębie miniatury)
* **Przycisk `[Uruchom w oknie]`**:
  * Lokalizacja: Na dole miniatury.
  * Najechanie (Hover): Przycisk podświetla się.
  * Kliknięcie LPM: Odłącza podgląd i otwiera go w nowym, pływającym, autonomicznym oknie (pozwala na dowolne skalowanie i przenoszenie na inne ekrany fizyczne).
* **Przycisk z ikoną Lupy `[ZoomButton]`**:
  * Lokalizacja: W prawym górnym rogu miniatury. Tło przezroczyste, widoczny symbol lupy.
  * Kliknięcie LPM: Działanie w pełni tożsame z kliknięciem w środek miniatury (przejście do powiększenia pełnoekranowego).

---

## 2. Tryb Pełnoekranowy Monitora (klasa FullscreenMonitorView)

Monitor zajmuje całą dostępną rozdzielczość. Środek przekazuje wszystkie kliknięcia (LPM, PPM) i znaki z klawiatury bezpośrednio do przybliżonego systemu (np. obsługa rozjazdów EDR, klikanie w podglądzie).

### 2.1 Pasek Nawigacyjny Nakładkowy (Overlay Toolbar)
Półprzezroczysty pasek w górnej krawędzi ekranu rysowany w wyższej warstwie, nakładający się na główny podgląd. Może posiadać efekt łagodnego znikania po zjechaniu myszką w dół.

* **Pole rozwijane `[Przełącz podgląd...]` (ComboBox)**:
  * Lokalizacja: W lewym górnym rogu. Tło jasnoszare.
  * Kliknięcie LPM w pole: Rozwija pionową listę pozostałych aktywnych monitorów biurka (np. "Monitor 3 – Kamery CCTV").
  * Najechanie (Hover) na element z listy: Zmienia kolor tła elementu na jasnoniebieski.
  * Kliknięcie LPM w element: Zwija listę ComboBox i błyskawicznie "przeskakuje" obraz, zmieniając treść pełnoekranową na wybrany monitor (bez animacji powrotu do biurka).
* **Przycisk powrotu `[Zamknij podgląd (Powrót)]`**:
  * Lokalizacja: W prawym górnym rogu.
  * Wygląd: Czerwone tło (`#D32F2F`), biały, pogrubiony tekst.
  * Najechanie (Hover): Zmienia tło na lekko jaśniejsze (`#F44336`).
  * Kliknięcie LPM: System opuszcza tryb pełnoekranowy, odtwarza płynną animację zmniejszania (Zoom-out) z powrotem do układu Wirtualnego Biurka i siatki miniatur.
  * Atrybut: Przycisk reaguje również na wciśnięcie klawisza `Escape` na klawiaturze (nasłuchuje zdarzenia wciśnięcia klawisza).
