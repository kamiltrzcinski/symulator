# EDR - Wytyczne Graficzne i Style (styles)

Ten plik zawiera szczegółowy opis niestandardowych stylów graficznych, efektów przejścia, reakcji na kursor myszy oraz prezentacji wizualnej danych w module EDR.

---

## 1. Efekty Przejścia i Animacje (Transitions)
* **Przybliżanie Dziennika Ruchu:** Zmiana wysokości podziału ekranu (Split Screen) przy przeciąganiu linii podziału – płynne skalowanie elementów (animacja w QSS / stylach interfejsu np. `transition: height 0.2s ease-out`).
* **Pojawianie się ikon ostrzegawczych (np. Tarcza D1, Słuchawka):** 
  * Wskaźnik nie pojawia się nagle – płynnie rozjaśnia się (`fade-in`) od 0% do 100% krycia w czasie 0.3s.
  * **Pulsowanie alarmowe:** Ikona nowo wprowadzonego obostrzenia powoli pulsuje (zmiana krycia od 70% do 100% z okresem 1.5s w pętli).

---

## 2. Reakcje na kursor myszy (Hover & Active States)
* **Paski kafelków zajętości torów:**
  * **Najechanie (Hover):** Kafelek toru stacyjnego zyskuje cienką, białą ramkę wewnętrzną o grubości 1px i delikatny cień zewnętrzny (`box-shadow: 0px 0px 5px rgba(255,255,255,0.5)`). Kursor zmienia się w łapkę (`pointer`).
  * **Wciśnięcie (Active/Click):** Kafelek wizualnie przesuwa się o 1px w dół, a jego cień znika (efekt fizycznego wciśnięcia).
  * **Przeciąganie (Drag & Drop):** Przeciągany numer pociągu wyświetla się jako półprzezroczysty prostokąt (opacity 60%) podążający za kursorem. Kursor przyjmuje postać dłoni chwytającej (`grabbing`).
* **Wiersze tabeli EDR:**
  * Najechanie na wiersz pociągu podświetla go na bardzo jasnoszary kolor (o 5% jaśniejszy od bazowego tła wiersza), ułatwiając czytanie w poziomie.
  * Zaznaczony wiersz świeci ciągłym pastelowo-pomarańczowym tłem.

---

## 3. Prezentacja Wizualna Danych (Stylizacja specyficzna)
* **Pola wymagane (Jasnoniebieskie):**
  * Niestandardowy pastelowy gradient niebieski: od `#D9EAD3` na górze komórki do `#cfe2f3` na dole.
  * W trybie edycji ramka aktywnego pola zmienia kolor na wyrazisty niebieski (`#3c78d8`) o grubości 2px.
* **Wykreślenia i Anulowania (Zablokowane wpisy):**
  * **Wykreślenie błędnego wiersza:** Na wiersz nakładana jest czarna, pozioma linia o grubości 2px przechodząca przez sam środek tekstu (efekt fizycznego skreślenia długopisem). Cały wiersz zyskuje stopień przezroczystości (opacity 50%).
  * **Anulowanie żądania (Krzyżyk X):** Numer pociągu jest przekreślony czerwoną linią układającą się w znak "X". Kursor nad tym polem zmienia się na ikonę zakazu (`not-allowed`).
* **Pomiary i statusy historyczne w raportach:**
  * Poprzednie, nieaktualne stany wierszy (historia zmian) są pisane czcionką szarą (`#888888`) o mniejszym kroju (9pt) i regularnym stylu.
  * Aktualny, zatwierdzony stan wiersza na dole sekcji jest pisany kolorem głębokiej czerni (`#111111`), czcionką pogrubioną (bold) o rozmiarze 10pt.
