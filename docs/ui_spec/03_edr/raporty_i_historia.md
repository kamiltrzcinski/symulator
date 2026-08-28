# Moduł Raportów i Historii (SUSRK-UI / EDR2)

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję w oknie Raportów i Historii, na podstawie rzeczywistej implementacji. Każde pole, tekst, przycisk oraz okienko są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

---

## 1. Wywołanie Raportów (Pasek Główny)

### 1.1 Przycisk `[Raporty]` na górnym pasku aplikacji (appBar)
Znajduje się w głównej nawigacji okna.
* **Najechanie (Hover / zdarzenie enterEvent)**: Wyzwala zmianę tła na pionowy gradient w kolorze "Mandarynka" (góra: `RGB(255, 220, 150)`, dół: `RGB(255, 154, 26)`).
* **Kliknięcie LPM**: Otwiera menu kontekstowe (obiekt zarządzany przez pętlę zdarzeń, np. `QMenu`) z opcjami wyboru rodzaju raportu.

### 1.2 Menu Kontekstowe
Rozwija się bezpośrednio pod przyciskiem `[Raporty]`.
* **Pozycja "Wyciag z dziennika ruchu"**: Kliknięcie LPM otwiera okno generowania podstawowego raportu z dziennika ruchu.
* **Pozycja "Wyciag z dziennika ruchu z historia zmian"**: Kliknięcie LPM otwiera okno generowania raportu z dziennika ruchu wraz z dołączoną na końcu historią zmian.
* **Pozycja "Wyciag z dziennika telefonicznego"**: Kliknięcie LPM otwiera okno generowania raportu z dziennika telefonicznego.
* **Pozycja "Wyciag z dziennika telefonicznego z historia zmian"**: Kliknięcie LPM otwiera okno generowania raportu z dziennika telefonicznego wraz z historią zmian.

---

## 2. Okno Dialogowe Raportu (`ShowReportDialog`)

Okno generowane jest modalnie wewnątrz pętli zdarzeń. Pojawia się wyśrodkowane na poziomie API względem okna głównego (rodzica). 

### 2.1 Parametry Główne Okna
* **Rozmiar**: 860x620 pikseli.
* **Tło okna**: Szare `#F2F2F2` (RGB: 242, 242, 242).
* **Pasek tytułowy (Title)**: Tekst dostosowuje się w zależności od wybranej w menu opcji (np. "Wyciag z dziennika ruchu" lub "Wyciag z dziennika telefonicznego"). 
* **Przycisk Zamknięcia systemu `[X]`**: Kliknięcie LPM zamyka okno i przerywa przeglądanie raportu.

### 2.2 Sekcja Filtrowania i Kontrolki (Header Controls)
Znajduje się w górnej części okna i zawiera następujące elementy od lewej do prawej:
* **Etykieta (Label) "Od:"**: Tekst statyczny.
* **Pole Daty (wskaźnik na widget daty, np. `QDateEdit`)**: Format krótki (Short). Domyślnie ustawione na dzień dzisiejszy. Kliknięcie LPM w ikonę kalendarza rozwija miesięczny widok wyboru daty początkowej.
* **Etykieta (Label) "Do:"**: Tekst statyczny.
* **Pole Daty (wskaźnik na widget daty)**: Wybór daty końcowej raportu. Działa analogicznie.
* **Lista rozwijana (np. `QComboBox`)**:
  * Widoczna **tylko** przy generowaniu raportów z dziennika ruchu.
  * Domyślnie wybrana jest opcja: "Wszystkie dzienniki ruchu".
  * Kliknięcie LPM w białe tło rozwija listę wszystkich dostępnych dzienników do przefiltrowania.
* **Przycisk `[Odśwież]`**: Posiada styl główny (Primary style). Kliknięcie LPM powoduje wygenerowanie na nowo tekstu raportu z uwzględnieniem obecnych dat oraz wybranego dziennika i wstawienie wyniku do pola tekstowego poniżej.
* **Przycisk `[Zamknij]`**: Kliknięcie LPM zwalnia instancję okna z pamięci bez zapisywania, wracając do głównej aplikacji.

### 2.3 Główna Sekcja Treści Raportu (`txtReport`)
Raport wyświetlany jest w potężnym polu tekstowym, nie posiada graficznej siatki (Grid) ani przycisków drukowania czy dyskietki.

* **Pole edytora tekstu (np. wskaźnik typu `QTextEdit`)**:
  * **Lokalizacja i Rozmiar**: Punkt startowy (X: 12, Y: 44), rozmiar 822x500 pikseli.
  * **Właściwości**: Wieloliniowość, obecne suwaki poziome i pionowe, ustalone jako widok tylko do odczytu (nie można edytować tekstu).
  * **Czcionka**: Stałoszerokościowa (np. ustawiona przez API QFont("Consolas")), rozmiar 9f. Zapewnia stałą szerokość znaków (Monospace) idealną do czytania logów.
  * **Zachowanie tekstowe**: Brak zawijania słów (linie nie zawijają się, należy przewijać poziomo w razie potrzeby). Zwykłe kliknięcie w pole tekstowe pozwala zaznaczyć wygenerowane raporty i je skopiować (Ctrl+C).

### 2.4 Struktura Tekstowa Raportu (`BuildReportText`)
Po kliknięciu przycisku `[Odśwież]` pole tekstowe jest czyszczone i uzupełniane linijka po linijce za pomocą czystego tekstu (plain text):
* **Nagłówek tekstowy**: Wyświetla się tekst informacyjny: `Data wygenerowania: [data i czas]` oraz `Zakres: [Od] - [Do]`.
* **Dane właściwe**: Zrzut wierszy ze wskazanego dziennika formowany jako zwykły, równo ułożony tekst.
* **Historia Zmian (Warunkowo)**: 
  * Jeśli użytkownik wybrał opcję z historią (np. "Wyciag z dziennika ruchu z historia zmian"), na samym dole dopisywany jest nagłówek "Historia zmian:".
  * Kolejne wersy historii logów lądują pod spodem w formacie: `YYYY-MM-DD HH:MM:SS | User | Area | Description`. Zamiast interaktywnego zaznaczania, historia jest na twardo wklejana na koniec raportu.
