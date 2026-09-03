# Specyfikacja UI: EBI Screen 300 - Logowanie, Autoryzacja i Zarządzanie Oknami

## 1. Architektura Ogólna (C++ / Qt)
Opisujemy tutaj zachowanie systemu z perspektywy pętli zdarzeń (Event Loop - `QEventLoop`), architektury Model-View (np. `QTableView` + `QAbstractTableModel`) oraz komunikacji opartej na mechanizmie sygnałów i slotów (Signals & Slots). Każda akcja użytkownika (kliknięcie myszą, wciśnięcie klawisza) generuje zdarzenie (np. `QMouseEvent`), które jest przechwytywane przez główną pętlę zdarzeń, a następnie emituje odpowiedni sygnał połączony ze slotem w logice biznesowej.

## 2. Proces Logowania i Uwierzytelniania

### 2.1. Inicjalizacja i Prezentacja Stanu Połączenia
- **UI:** W lewym górnym rogu głównego okna (`QMainWindow`) znajdują się wskaźniki (zbudowane np. na bazie `QLabel` lub dedykowanych widgetów rysowanych przez `paintEvent`), prezentujące stan połączenia z serwerem.
- **Logika:** Wątek sieciowy (Network Thread) cyklicznie odpytuje serwer. Zmiana stanu (np. z "Brak transmisji" na "Gotowy") powoduje wyemitowanie sygnału `connectionStateChanged(enum State)`.
- **Slot:** Główny wątek GUI odbiera sygnał i wywołuje slot `updateConnectionIndicator(enum State)`, który zmienia arkusz stylów (`QSS`) lub ikonę widgetu.

### 2.2. Okno Logowania
- **Zdarzenie otwarcia:** Po starcie aplikacji lub po kliknięciu z menu `Logowanie -> Zaloguj` (akcja `QAction::triggered`), pętla zdarzeń otwiera okno dialogowe logowania (`QDialog`).
- **Komponenty Okna:**
  - `QLineEdit` dla nazwy użytkownika.
  - `QLineEdit` z ustawionym `echoMode(QLineEdit::Password)` dla hasła.
  - Przyciski `QPushButton` "OK" oraz "Anuluj".
- **Przepływ zdarzeń (Event Flow):**
  1. Użytkownik wpisuje dane i klika "OK". Generowane jest zdarzenie myszy przeradzające się w sygnał `clicked()` przycisku.
  2. Slot `onLoginButtonClicked()` pobiera teksty z `QLineEdit`.
  3. Emitowany jest sygnał `authenticate(QString user, QString password)` do warstwy autoryzacji.
  4. Okno logowania blokuje się (np. `setDisabled(true)`) na czas weryfikacji, aby zapobiec wielokrotnym kliknięciom (współdziałanie z pętlą zdarzeń).
  5. Jeśli uwierzytelnienie przebiegnie pomyślnie, warstwa autoryzacji emituje sygnał `loginSuccessful()`. 
  6. Dialog logowania zamyka się (`accept()`), a na pasku informacyjnym (`QLabel`) pole zostaje zaktualizowane na "Zalogowany: [Imię i Nazwisko]". Zalogowanie nowego użytkownika automatycznie wylogowuje poprzedniego (zmiana kontekstu sesji).

## 3. Przejmowanie Autoryzacji (Obszary Sterowania)

### 3.1. Wyświetlanie Okna Autoryzacji
- **Opis:** Po poprawnym zalogowaniu, użytkownik musi przydzielić sobie obszary autoryzacji (stacje, funkcje).
- **Zarządzanie widokiem:** Okno autoryzacji to siatka przycisków (`QGridLayout` lub interaktywny `QTableView`).
- **Kolorystyka i Stany (Model):**
  - **Czerwony:** brak przydziału autoryzacji (nikt nie ma).
  - **Żółty:** zalogowany użytkownik posiada autoryzację.
  - **Szary:** inny użytkownik ma autoryzację.
  - **Biały / Czarny:** poza kontrolą / brak uprawnień.
- **Wybór obszaru:**
  1. Kliknięcie lewym przyciskiem myszy na określony kwadrat (obszar) generuje sygnał w widoku.
  2. Kliknięty obszar otrzymuje status "Zaznaczony" - pojawia się biały krzyżyk (nakładany przez delegat `QStyledItemDelegate::paint`).
  3. Aby przywrócić stan (odznaczyć), użytkownik klika przycisk "Przywróć" (niebieska strzałka).

### 3.2. Zatwierdzanie i Przejmowanie Autoryzacji
- **Zatwierdzenie:** Kliknięcie ikony z zielonym symbolem (przycisk "Zatwierdź") emituje sygnał `requestAuthorization(QList<AreaId>)`.
- **Przejmowanie (Takeover):** Jeśli obszar był szary (należał do kogoś innego), system wyświetla okno potwierdzenia (`QMessageBox::question`). Po zatwierdzeniu przez operatora, emitowany jest sygnał wymuszający odebranie uprawnień innemu użytkownikowi (serwer synchronizuje stan).
- **Przełączenie serwerów:** W razie awarii i przełączenia na serwer rezerwowy, autoryzacje są lokalnie tracone. Okno otwiera się z zapamiętanym "poprzednim" stanem, a operator musi tylko kliknąć "Zatwierdź", by ponownie je zażądać z nowego serwera.

## 4. Zarządzanie Oknami Zdarzeń i Alarmów

Obydwa okna opierają się na zaawansowanym zastosowaniu architektury Model-View.
- **Model:** `QAbstractTableModel` przechowujący dane (każdy wiersz to osobna instancja struktury C++ reprezentującej Zdarzenie lub Alarm).
- **Filtrowanie i Sortowanie:** `QSortFilterProxyModel` nakładany pomiędzy model danych a widok. Pozwala na błyskawiczne, asynchroniczne filtrowanie bez zmiany oryginalnych danych.
- **Widok:** `QTableView` z włączoną elastyczną szerokością kolumn i opcją zwijania.

### 4.1. Okno Zdarzeń

#### 4.1.1. Struktura i Filtry
- **Kolumny:** Data/czas, Obszar, Stacja, Typ obiektu, Obiekt, Kategoria, Tekst, Operator, Notatka, Źródło.
- **Filtrowanie:** W nagłówkach kolumn (`QHeaderView`) zaimplementowano interaktywne przyciski (ComboBox/Menu Dropdown). Wybranie elementu emituje sygnał `filterChanged(Column, Value)`, który przekazuje wyrażenie regularne (`QRegExp` / `QRegularExpression`) do `QSortFilterProxyModel::setFilterRegExp()`.
- **Always on top:** Checkbox "Zostaw na wierzchu" łączy swój sygnał `toggled(bool)` ze slotem modyfikującym flagi okna (`setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint)`).

#### 4.1.2. Paginacja i Mechanizm Online/Offline
- Ponieważ liczba zdarzeń jest ogromna, system stosuje paginację.
- **Online / Offline:** 
  - W trybie **Online**, nowe zdarzenia trafiają od razu do Modelu. Pętla zdarzeń odświeża widok.
  - Zmiana strony (kliknięcie nawigacji) emituje sygnał wstrzymania przewijania. Przechodzimy w stan **Offline**. Odpala się `QTimer` ustawiony na 30 sekund.
  - Jeśli w tle (w trybie Offline) spłyną nowe zdarzenia przez gniazdo sieciowe (Socket), emitowany jest sygnał `newEventsBuffered()`. Slot odbierający ten sygnał podświetla przycisk "Offline" na niebiesko (zmiana właściwości QSS).
  - Kliknięcie podświetlonego przycisku "Offline" wraca do trybu Online, ładując zbuforowane zdarzenia do modelu głównego widoku.

#### 4.1.3. Dodawanie Notatek
- **Proces Łopatologiczny:**
  1. W komórce kolumny "Notatka" widnieje ikona ołówka (wyrysowana przez `QStyledItemDelegate`).
  2. Kliknięcie myszą w rejon komórki otwiera delegat do edycji (tryb `edit`). Tworzony jest widget `QLineEdit` wpisany w komórkę.
  3. Użytkownik wpisuje tekst.
  4. Kliknięcie w ikonę dyskietki (lub wciśnięcie Enter - sygnał `editingFinished()`) powoduje zamknięcie edytora, a Model otrzymuje wywołanie `setData(index, value, Qt::EditRole)`.
  5. Model zapisuje notatkę w strukturze, emituje sygnał sieciowy do serwera, by zsynchronizować notatkę z bazą.

#### 4.1.4. Menu Kontekstowe (Prawy Przycisk Myszy)
- Wywołanie w pasku tytułowym generuje `QContextMenuEvent`.
- Otwiera się `QMenu` z opcjami: "Drukuj", "Podgląd wydruku", "Zapisz do pliku (CSV)".
- Slot dla CSV iteruje po aktualnie przefiltrowanym modelu (`QSortFilterProxyModel`), wyciąga `data()` z każdego indeksu i zapisuje to do `QFile` używając `QTextStream`.

### 4.2. Okno Alarmów

#### 4.2.1. Złożoność Delegatów i Status
- Dziedziczy mechanizmy filtrowania po Oknie Zdarzeń, ale wprowadza interakcję zwrotną.
- **Ikony statusu (Pierwsze dwie kolumny):**
  - Model zwraca odpowiednią ikonę przez `data(index, Qt::DecorationRole)`.
  - Żółty trójkąt: alarm wystąpił.
  - Czerwony trójkąt (!): alarm trwa.
  - Zielony haczyk: alarm potwierdzony.

#### 4.2.2. Potwierdzanie Alarmów
- **Mechanika:**
  1. Użytkownik zaznacza wiersze (możliwość multi-select przy użyciu Shift/Ctrl - obsługiwane natywnie przez `QItemSelectionModel`).
  2. Kliknięcie prawym przyciskiem myszy na wierszu wywołuje `QMenu` na liście (`customContextMenuRequested` signal z widoku).
  3. Użytkownik wybiera opcję "Potwierdź" (`QAction::triggered`).
  4. Slot pobiera wszystkie zaznaczone indeksy z `QItemSelectionModel`, buduje paczkę danych i emituje sygnał `acknowledgeAlarms(QList<int> alarmIds)`.
  5. Po odebraniu odpowiedzi od serwera (sieciowy event asynchroniczny), Model zmienia stan alarmów. Zdarzenie to wymusza przerysowanie tabeli – zamiast żółtego trójkąta renderowany jest zielony haczyk.

#### 4.2.3. Sygnalizacja i Wyciszanie
- Przy nadejściu nowego alarmu (dodanie wiersza do modelu), uruchamiana jest sygnalizacja dźwiękowa (np. `QSound::play()`) oraz pokazana zostaje globalna czerwona ikona z wykrzyknikiem.
- **Wycisz:** Checkbox "Wycisz" blokuje wywołania dźwiękowe.
- **Kasowanie alarmów (Kasuj):** Użytkownik wybiera alarm, uruchamia menu kontekstowe -> Kasuj. Jeśli logika zezwala na skasowanie (alarm z niskim priorytetem lub potwierdzony i nieaktywny), Model wywołuje `removeRows()`, co sprzęga się z sygnałem `rowsAboutToBeRemoved()` i znika dany wiersz z ekranu.

## 5. Podsumowanie
Całość zarządzania UI, uwierzytelnianiem i oknami oparta jest o nieprzerwanie działającą pętlę zdarzeń (`Event Loop`). Architektura oparta na delegatach i proxy modelach upewnia, że operacje wizualne (takie jak modyfikacja notatki, wygaszanie/potwierdzanie setek alarmów naraz) przebiegają asynchronicznie, bez zamrażania UI, reagując natychmiastowo na dane spływające od komponentów niższej warstwy przez sygnały.
