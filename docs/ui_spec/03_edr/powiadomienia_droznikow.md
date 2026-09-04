# EDR - Powiadomienia dróżników kolejowych

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) elementy wizualne i interakcje związane z powiadamianiem dróżników w Elektronicznym Dzienniku Ruchu (EDR), zbudowanym na architekturze C++ (np. bazującym na frameworku Qt). Każde pole, tekst i przycisk są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

---

## 1. Wyświetlanie Kolumny 8 (Dziennik Ruchu)
Kolumna 8 ("Powiadomienia dróżników i inne") służy do oznaczania powiadomienia dróżników na przejazdach.

* **Tło kolumny**: Zazwyczaj białe, ale gdy wiersz dotyczy pociągu wjeżdżającego/odjeżdżającego, całe tło komórki zmienia się na jasnoniebieski kolor (`#D0E5F5`), symbolizując aktywność.
* **Wizualizacja stanu powiadomienia**:
  * **Puste**: Brak powiadomienia.
  * **Znak ukośnika (`/`)**: Oznacza, że dróżnik został powiadomiony. Ten wariant stosuje się np. dla przejazdów obsługiwanych bez konkretnych godzin (np. w określonych sytuacjach szlaku jednotorowego).
  * **Godzina (np. `14:23`)**: Oznacza konkretną godzinę zgłoszenia. Wpisana pogrubioną czcionką (Bold) w kolorze czarnym.

## 2. Ręczne Wprowadzanie Powiadomień (Menu Kontekstowe)
Dyżurny ruchu może powiadomić dróżnika klikając bezpośrednio na komórkę w Kolumnie 8.

* **Co klikam:** Kliknięcie PPM (Prawym Przyciskiem Myszy) w obrębie komórki Kolumny 8 dla danego pociągu.
* **Co się dzieje:** Rozwija się systemowe menu kontekstowe (np. wskaźnik QMenu).
* **Opcje Menu:**
  * **`Wstaw ukośnik (/)`**: 
    * Kliknięcie LPM wpisuje we wciąż aktywnej komórce znak `/`, zastępując ewentualną poprzednią zawartość.
  * **`Wstaw obecny czas`**:
    * Kliknięcie LPM pobiera bieżący czas systemowy komputera (w formacie `HH:mm`) i wstawia go do komórki.
  * **`Oczyść powiadomienie`**:
    * Kliknięcie LPM kasuje zawartość komórki.

## 3. Modalne Okno Zbiorczego Powiadamiania (`Powiadomienia dróżników kolejowych`)
W przypadku wielu dróżników lub zaawansowanych szlaków, system posiada dedykowane okno.

* **Wywoływanie Okna:**
  * Kliknięcie LPM w specjalny przycisk `[Powiadom]` (Ikona dzwonka) na górnym pasku narzędzi.
* **Właściwości Okna:**
  * Okno wyświetla się na środku rodzica jako zablokowany dialog modalny (np. QDialog), jest zablokowane przed zmianą rozmiaru.
  * Tło: Jasnoszare (`#F0F0F0`).
  * Nagłówek: Pogrubiony napis `Powiadomienia dróżników kolejowych`.
* **Lista Dróżników (Lista z polami wyboru - checkable item list):**
  * Okno zawiera listę wszystkich posterunków dróżników przyległych do szlaku (np. `Strażnica Przejazdowa nr 1`, `Strażnica nr 2`).
  * Każda pozycja posiada pole wyboru (Checkbox) po lewej stronie.
  * Kliknięcie LPM na pole wyboru zaznacza je (pojawia się haczyk).
* **Sekcja Czasu:**
  * Etykieta: `Czas powiadomienia:`.
  * Pole wyboru czasu (np. `QTimeEdit`): Format `HH:mm`. Domyślnie obecny czas.
* **Dolne Przyciski:**
  * **Przycisk `[Zatwierdź]`**: Tło akcentowane (np. lekki niebieski `#E1F0FA`). Kliknięcie LPM powoduje wpisanie podanego czasu do wszystkich wybranych (zaznaczonych) dróżników w aktualnym pociągu. Okno się zamyka.
  * **Przycisk `[Anuluj]`**: Kliknięcie LPM zwalnia zasoby okna i zamyka je bez dokonywania zmian w systemie.
