# Radiotelefony - Typ 2: Konsola "Radionika" (Kartuzy)

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję uproszczonej konsoli dotykowej systemu radiowego Radionika, stosowanej w mniejszych nastawniach, zbudowanej przy wykorzystaniu wirtualnych metod rysowania (jak w ImGui).

---

## 1. Główne Okno Aplikacji (Konsola Główna)
Tło aplikacji jest jasnoszare (`#F0F0F0`). W centralnej części znajdują się duże poziome paski (Karty Kanałów), po prawej stronie pionowy panel z suwakami.

### 1.1 Karty Kanałów (Kanał Pociągowy i Drogowy)
Każdy kanał reprezentowany jest przez szeroki, prostokątny sub-blok (Panel).
* **Elementy składowe sub-bloku kanału (np. Kanał 001):**
  * **Tło i obramowanie:** Najechanie kursorem (Hover) nie powoduje efektu. 
  * **Kliknięcie LPM w środek paska:** Zaznacza kanał jako Aktywny. Tło całego paska zmienia kolor na jaskrawożółty (`#FFEB3B`), ramka staje się pogrubiona. Odznacza inny ewentualnie wybrany kanał. Dźwięk systemowego "pik".
  * **Pionowy niebieski pasek z ikonami po lewej stronie:** 
    * **Ikona kłódki (Blokada panelu):** Kliknięcie LPM zamyka kłódkę (zmienia ikonę na zamkniętą). Kolejne kliknięcia w pasek kanału są ignorowane (zapobiega przypadkowej zmianie kanału).
    * **Ikona głośnika (Wyciszenie/Odsłuch):** Kliknięcie LPM zmienia ikonę na przekreślony głośnik (kolor ikony czerwony). Nasłuch kanału jest fizycznie ucięty z głośników komputera. Ponowne kliknięcie LPM odblokowuje nasłuch (ikona zwykłego głośnika).

* **Górny rząd małych ikon wewnątrz sub-bloku (Tylko wskaźniki - brak akcji kliknięcia):**
  * **Ikona `Anteny`:** Świeci na czerwono, gdy inny dyżurny lub maszynista nadaje.
  * **Ikona `Pociągu`:** Informuje o aktywnym statusie sieci dla pojazdów.
  * **Ikona `Trójkąta Ostrzegawczego`:** Zaczyna dynamicznie migać (żółto-czerwono), jeśli na kanale zarejestrowano błędy lub alarmy niższej wagi.
  * **Cyfra Priorytetu (np. `3`):** Wyświetlana stałym tekstem.

* **Dolny rząd przycisków funkcyjnych kanału:**
  Najechanie (Hover) na jakikolwiek przycisk dolny robi delikatne obramowanie w kolorze szarym.
  * **Przyciski ze strzałkami `[1]` i `[3]`:** Kliknięcie LPM wysyła selektywne wywołanie.
  * **Przycisk z ikoną Fali (Nasłuch):** Kliknięcie LPM wymusza stałe otwarcie blokady szumów (Squelch) dla tego kanału. Słychać szum (biały szum) z głośników. Wciska się wizualnie. Ponowne kliknięcie LPM wyłącza Squelch (cisza).
  * **Przycisk `[A]` (Duży, czerwony przycisk z białą literą A):**
    * Kliknięcie LPM: Uruchamia nadawanie sygnału alarmowego RADIOSTOP (trzy tony) na tym konkretnym kanale. Tło całego paska kanału zaczyna naprzemiennie migać (żółty / czerwony) z częstotliwością 2Hz. Dźwięk alarmu słychać w głośnikach stacji.
    * Ponowne kliknięcie LPM (na migającym pasku) podczas trwania alarmu: Zatrzymuje emisję RADIOSTOP. Pasek przestaje migać, powraca do stanu żółtego (aktywnego).

### 1.2 Panel boczny głośności (Suwaki)
Po prawej stronie ekranu, pionowy wąski panel na szarym tle.
* **Pionowy suwak (Slider) `Audio`:**
  * Służy do regulacji głośności mowy i dźwięków operacyjnych.
  * **Interakcja (Drag & Drop):** Wciśnięcie LPM na suwaku (tzw. "thumb") i poruszanie w górę/dół przesuwa suwak i procentowy wskaźnik nad nim (`100%`, `50%`, `0%`).
  * **Kliknięcie LPM na tor (Track) suwaka:** Przeskakuje wartość skokowo do miejsca kliknięcia.
* **Pionowy suwak (Slider) `Tło`:**
  * Służy do regulacji głośności szumów, pisków i hałasów tła symulowanego.
  * Działa identycznie jak suwak "Audio".
