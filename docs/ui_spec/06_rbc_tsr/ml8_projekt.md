# Projekt Interfejsu ML8 (Mors-Siemens) dla RBC / TSR

Niniejszy dokument opisuje rygorystycznie szczegółową specyfikację interfejsu systemu ML8, zaimplementowaną natywnie w C++ z wykorzystaniem mechanizmu sygnałów i slotów do obsługi zdarzeń UI. Architektura opiera się na dwóch głównych zakładkach. Dokument rozkłada na czynniki pierwsze każdy element interfejsu (włącznie z najprostszymi przyciskami okiennymi i wywołaniami wskaźników).

---

## 1. Nawigacja Główna (Pasek Zakładek)
Na samej górze ekranu (lub po lewej stronie, w zależności od preferencji stanowiska) znajduje się główny pasek nawigacyjny służący do przełączania się między modułami.
* **Przycisk zakładki `[TSR]`**: Kliknięcie lewym przyciskiem myszy (LPM) emituje sygnał ładujący moduł schematu torowego i zarządzania ograniczeniami na ciemnym tle. Jeśli użytkownik jest już na tej zakładce, przycisk jest podświetlony (flaga aktywności widżetu), a ponowne kliknięcie nie wywołuje żadnej akcji (sprawdzany warunek stanu).
* **Przycisk zakładki `[POCIĄGI]`**: Kliknięcie LPM przełącza widok na jasną tabelę rejestru pociągów, ładując inny układ widżetów do pamięci.
* **Przycisk `[X]` (Zakończ Sesję / Wyloguj) w prawym górnym rogu ekranu**: Kliknięcie LPM tworzy nową instancję okna dialogowego (wywołanie konstruktora okna) z pytaniem "Czy na pewno chcesz się wylogować z pulpitu RBC?". 
  * W oknie dialogowym przycisk `[ZAMKNIJ]` (lub ikona `[X]` w jego prawym górnym rogu) - kliknięcie anuluje akcję i zamyka okno (wywołując jego destruktor).
  * W oknie dialogowym przycisk `[WYLOGUJ]` - kliknięcie emituje sygnał powodujący wylogowanie dyżurnego ruchu z systemu i powrót do ekranu logowania, zamykając obecną sesję i kasując lokalne zasoby ustrukturyzowane (czyszczenie wskaźników obiektów).

---

## 2. Zakładka 1: Obsługa TSR i Widok Stacji (Ciemny Motyw)

Zgodnie z wymaganiami Ie-104.2, obszar ten prezentuje wektorowy układ torowy na czarnym tle.

### 2.1. Narzędzia Główne TSR
W górnej części zakładki TSR znajduje się pasek akcji zawierający następujące przyciski (podpięte przez wskaźniki pod pętlę widoku):
* **Przycisk `[Nowy TSR]`**: Kliknięcie LPM dynamicznie tworzy nowe okno "Definicja nowego TSR" i przypisuje callback, który zmienia tryb zdarzeń kursora na "wskazywanie obiektów" na mapie.
* **Przycisk `[Wydruk]` (Ikona drukarki)**: Kliknięcie LPM otwiera systemowe okno drukowania z podglądem wygenerowanego raportu.

### 2.2. Okno "Definicja nowego TSR"
Po kliknięciu `[Nowy TSR]` tworzona jest dynamicznie instancja obiektu okna.
* **Przycisk `[X]` (w prawym górnym rogu okna)**: Kliknięcie LPM zwalnia pamięć obiektu okna (wywołanie destruktora), przerywa proces tworzenia TSR i porzuca niezapisany bufor danych.
* **Pole tekstowe `[Identyfikator TSR]`**: Kliknięcie LPM w instancję widżetu pola aktywuje kursor (miganie) poprzez sygnał `onFocus`. Pozwala na wpisanie nazwy z klawiatury.
* **Przycisk `[Wybierz obiekt początkowy]`**: Kliknięcie LPM ustawia we właściwościach obiektu flagę podświetlenia na `true`. Następnie logika przechwytuje zdarzenie z głównego okna po kliknięciu obiektu na mapie, uzupełniając pole nazwą klikniętego elementu (za pomocą pobrania łańcucha znaków z wybranego wskaźnika stacji) i podświetlenie gaśnie.
* **Pole liczbowe `[Korekta początkowa (m)]`**: Kliknięcie w pole pozwala wpisać z klawiatury wartość przesunięcia (buforowane do wewnętrznej struktury).
* **Przycisk `[Wybierz obiekt końcowy]`**: Zasada działania identyczna jak dla obiektu początkowego.
* **Pole liczbowe `[Korekta końcowa (m)]`**: Kliknięcie pozwala wpisać wartość przesunięcia końcowego.
* **Przycisk `[Wybierz obiekty pośrednie]`**: Umożliwia "wyklikanie" wielu obiektów. Dodaje kolejne wskaźniki obiektów klikniętych na mapie do wektora z listą.
* **Lista rozwijana `[Kierunek]`**: Kliknięcie LPM rozwija widżet typu Dropdown. Użytkownik ma do wyboru opcje. Kliknięcie wywołuje slot zapisujący wybraną wartość.
* **Lista rozwijana `[Prędkość km/h]`**: Działa jako Dropdown. Kliknięcie wybranej wartości generuje event przypisania wartości.
* **Lista rozwijana `[Przyczyna]`**: Dropdown definiujący powód dla DMI z predefiniowanych.
* **Przycisk `[ANULUJ]` (na dole okna)**: Kliknięcie LPM aktywuje wskaźnik i zamyka okno bez zapisu stanu - wywołując destruktor i usuwając obiekt okna z pamięci operacyjnej.
* **Przycisk `[ZAPISZ I AKTYWUJ]` (na dole okna)**: Kliknięcie LPM uruchamia slot weryfikacyjny. Jeśli dane są poprawne, dane struktury są serializowane i transmitowane po gnieździe do RBC, po czym samoczynnie wywoływany jest destruktor okna "Definicja nowego TSR". Na mapie (w pętli jej rysowania) pojawia się wyrenderowany pomarańczowy obszar.

### 2.3. Edycja i Usuwanie TSR na mapie
* **Kliknięcie LPM na pomarańczowy obszar aktywnego TSR na mapie**: Zdarzenie wywołuje zainicjowanie klasy małego okna pop-up powiązanego wskaźnikiem z wybranym TSR.
* **W oknie pop-up szczegółów**:
  * **Przycisk `[X]`**: Zwalnia instancję pop-upa.
  * **Przycisk `[DEZAKTYWUJ]`**: Kliknięcie LPM emituje sygnał wyłączający tryb pracy danego TSR w backendzie, aktualizuje parametry widoku obszaru na szare, a następnie niszczy sam pop-up.
  * **Przycisk `[USUŃ]`**: Kliknięcie LPM inicjuje dodatkowe okienko z ostrzeżeniem weryfikującym "Czy na pewno usunąć TSR?".
    * **Przycisk `[Tak]`**: Zwalnia dany wektor powiązany ze strukturą TSR, usuwa TSR całkowicie z pamięci systemu i wysyła polecenie kasacji, na koniec wywołując destruktory okien.
    * **Przycisk `[Nie]` / `[X]`**: Wywołuje zniszczenie ostrzeżenia pozostawiając TSR bez zmian.

---

## 3. Zakładka 2: Rejestr Pociągów w RBC (Jasny Motyw)

### 3.1. Tabela Pociągów
Ekran ten to instancja widżetu tabelarycznego ułożonego w rzędy.
* **Najechanie kursorem myszy na wiersz (Zdarzenie hover)**: Pętla graficzna przechwytuje event `onMouseEnter` na wierszu i na czas tego zdarzenia rysuje inne tło (np. flagą w strukturze wyświetlania).
* **Pojedyncze kliknięcie LPM na wiersz pociągu**: Zapisuje główny wskaźnik kontekstowy na konkretny obiekt tego pociągu (wymusza kolor zaznaczenia komórek przez `onMouseClick`) i rozwija panel u dołu z przekazaną referencją na ten pociąg. Przekliknięcie gdzie indziej aktualizuje ten wskaźnik i powiązany panel.

### 3.2. Pasek Narzędziowy Wybranego Pociągu (Pod tabelą)
Obsługuje akcje przypisane przez dany wskaźnik obiektu.

#### 3.2.1. Przycisk `[Szczegóły / Dane pokładowe]`
* **Kliknięcie LPM**: Konstruuje okno typu child od szczegółów pociągu.
* **W oknie "Szczegóły pociągu"**:
  * **Przycisk `[X]` (w prawym górnym rogu)**: Destrukcja (zwolnienie pamięci) obiektu podokna.
  * Okno wyświetla przypisane pola wartości i teksty, lecz ustawia ich natywne flagi kontrolne UI na stan zabezpieczenia trybem `read_only` (brak przetwarzania eventów klawiatury dla tych pól).
  * **Przycisk `[ZAMKNIJ]` (na dole okna)**: Ten sam callback co dla `[X]` - zwolnienie z pamięci okna.

#### 3.2.2. Przycisk `[Wiadomości Tekstowe]`
* **Kliknięcie LPM**: Tworzy nową instancję klasy kontrolera korespondencji okienkowej z maszynistą.
* **W oknie "Korespondencja z maszynistą"**:
  * **Przycisk `[X]` (w prawym górnym rogu)**: Zwalnia komunikator (destruktor).
  * **Pole historii (okno przewijane)**: Widżet pola tekstowego skonfigurowany pod rzutowanie logów z wyłączoną edycją z klawiatury (flaga UI). Pasek pozwala tylko na zdarzenia przewijania myszką/klawiszami.
  * **Pole tekstowe "Wpisz nową wiadomość"**: Interfejs bufora wprowadzającego - LPM podrzuca focus klawiatury.
  * **Element sprawdzający `[Wymagaj potwierdzenia]` (Widżet Checkbox)**: LPM przesyła sygnał wymuszający modyfikację lokalnej zmiennej boolean "ptaszka", która wymusza przerysowanie nowej formy kontrolki.
  * **Przycisk `[WYŚLIJ]`**: Kliknięcie LPM emituje slot pobierający dane buforu oraz flagi elementu sprawdzającego - generując komendę zapisu, wysłania do strumienia, następnie wykonując slot wyczyszczenia pola wpisywania.
  * **Przycisk `[ZAMKNIJ]` (na dole)**: Usunięcie i zwolnienie zasobu interfejsu korespondencji z pamięci.

#### 3.2.3. Przycisk `[Awaryjne Zatrzymanie Pociągu]` (Wyróżniony na czerwono)
* **Kliknięcie LPM**: Tworzy nakładające się z wyższym priorytetem okno modalne potwierdzenia (flaga z blokadą obsługi zdarzeń innych z tyłu `setWindowModality`).
* **W oknie ostrzegawczym "UWAGA: Potwierdzenie zatrzymania"**:
  * **Brak przycisku `[X]`**: Konstruktor jest wywoływany w konfiguracji ukrywającej przyciski zamykania z ramki systemowej.
  * **Przycisk `[ANULUJ]` (szary)**: Callback powiązany niszczy z pamięci dany obiekt ostrzeżenia, kasując jego pętle weryfikacji.
  * **Przycisk `[ZATRZYMAJ!]` (czerwony)**: Callback powiązany wysyła komendę zatrzymania używając wewnętrznego strumienia sieciowego, a następnie `delete this;` wymusza samoczynne zniszczenie struktury okna komunikatu. Rysowanie tabeli dopisuje na sztywno flagę awarii u pociągu (dodatkowa modyfikacja graficzna z czerwonym kółkiem przy indeksie pociągu).

#### 3.2.4. Przycisk `[Wyrejestruj Pociąg]`
* **Kliknięcie LPM**: Dynamiczna alokacja okna typu popup dla wyrejestrowania.
* **W oknie "Wyrejestrowanie"**:
  * **Przycisk `[X]`**: Destrukcja pop-upa.
  * **Przycisk `[NIE]`**: Destrukcja pop-upa.
  * **Przycisk `[TAK]`**: Rozpoczyna wyrejestrowanie (usuwając logikę obiektu z tabeli powiązanych), podokno robi `delete this`, pociąg znika po zwolnieniu z wektora obiektów.

### 3.3. Rejestracja zdarzeń i alarmów
U dołu lub w wydzielonej zakładce pojawiają się powiadomienia nałożone poprzez menedżer sygnałów zewnętrznych.
* **Gdy pojawi się nowy ALARM (np. gorące hamulce)**: Na ekran wrzucony jest instancjonowany element typu banner wysyłający na okrągło callback przerysowania kolorów (migotanie).
* **Przycisk `[POTWIERDŹ ALARM]` (na bannerze)**: LPM przez operatora uderza w sygnał zaprzestania działania timera migotania i mutuje audio z modułu sprzętowego. Banner usuwa powiązane eventy by trafić do archiwum eventów a instancja ostrzeżenia graficznego wędruje na zwolnienie pamięciowe (pojedynczy wpis zostaje logowany).
