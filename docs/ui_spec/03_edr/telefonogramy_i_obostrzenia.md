# EDR - Telefonogramy i Obostrzenia Szlakowe

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję związaną z rejestrowaniem telefonogramów ruchowych oraz powiązanymi wskaźnikami w Elektronicznym Dzienniku Ruchu (EDR), zbudowanym na architekturze C++ (np. wykorzystującym framework Qt / ImGui). Każde pole, tekst, przycisk oraz okienko są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

---

## 1. Wywoływanie Okna Telefonogramu (Główne Okno EDR)

*   **Przycisk na pasku narzędzi (Ikona koperty ze słuchawką)**: 
    *   Kliknięcie LPM na pasku nad dziennikiem ruchu wywołuje okno modalne telefonogramu.
*   **Menu kontekstowe nagłówka szlaku**: 
    *   Kliknięcie PPM na zobrazowanie torów szlakowych rozwija szybkie menu kontekstowe. Wybór rodzaju telefonogramu otwiera okno modalne z prekonfigurowanymi danymi.
*   **Aktywne ikony ostrzegawcze (Tarcza D1, Słuchawka, Trójkąt)**:
    *   Dwukrotne kliknięcie LPM na aktywną ikonę w nagłówku szlaku automatycznie wywołuje okno z odpowiednim szablonem odwołującym obostrzenie (np. wprowadzającym otwarcie toru).

---

## 2. Okno Modalne "Telefonogram do Dziennika Telefonicznego" (`TelefonogramDialog.cpp`)

Okno to wyskakuje wyśrodkowane względem okna nadrzędnego, jest zablokowane przed zmianą rozmiaru (brak maksymalizacji i minimalizacji, sztywny rozmiar), ma wymiary 680x680 pikseli. Posiada tło jasnoszare (`RGB: 240, 240, 240`). Używa czcionki bazowej Segoe UI o rozmiarze 9.5pt.

### 2.1 Górny pasek i typ telefonogramu
*   **Napis pogrubiony "Dziennik telefoniczny"** (Czcionka 12pt, Bold).
*   **Przyciski opcji jednokrotnego wyboru (np. wskaźniki `QRadioButton`) `[Nadaję]` i `[Odbieram]`**:
    *   Domyślnie zaznaczone jest `[Nadaję]`. 
    *   Kliknięcie LPM zaznacza kropką wybraną opcję i dynamicznie przełącza stany pól tekstowych (blokuje/odblokowuje poprzez sygnały/sloty) oraz zmienia etykiety pod spodem.

### 2.2 Sekcja Danych Podstawowych (Siatka 2x3, Tło Szare/Białe)
*   **Pole tekstowe `[Numer własny:]`**: Tło ciemnoszare (`RGB: 190, 190, 190`), tylko do odczytu (ustawiona odpowiednia flaga interfejsu). Automatycznie wypełniane następnym numerem z menedżera stanu systemu.
*   **Pole tekstowe `[Numer odbiorcy:]` / `[Numer nadawcy:]`**: Tło białe, proste obramowanie. Etykieta zmienia się zależnie od stanu przycisku opcji (Odbiorcy gdy Nadaję, Nadawcy gdy Odbieram). Kliknięcie LPM aktywuje kursor do wpisania numeru.
*   **Pole tekstowe `[Nadał:]`**: 
    *   Gdy wybrano `[Nadaję]`: Tło ciemnoszare (`RGB: 190, 190, 190`), zablokowane do edycji. Automatycznie wstawia Imię i Nazwisko zalogowanego dyżurnego ze stanu aplikacji.
    *   Gdy wybrano `[Odbieram]`: Tło białe, odblokowane do wpisania nazwiska dyżurnego nadającego z sąsiedniej stacji.
*   **Pole tekstowe `[Odebrał:]`**: 
    *   Gdy wybrano `[Nadaję]`: Tło białe, odblokowane do wpisania nazwiska dyżurnego przyjmującego.
    *   Gdy wybrano `[Odbieram]`: Tło ciemnoszare (`RGB: 190, 190, 190`), zablokowane. Automatycznie wstawia Imię i Nazwisko zalogowanego dyżurnego.
*   **Kontrolka wyboru daty `[Data:]` (np. `QDateEdit`)**: Kliknięcie LPM rozwija kalendarz. Domyślnie ustawiona data dzisiejsza.
*   **Kontrolka wyboru czasu `[Czas zakończenia:]` (np. `QTimeEdit`)**: Format `HH:mm`. Posiada wbudowane strzałki góra/dół z boku pola. Domyślnie wstawia bieżący czas.

### 2.3 Sekcja Wzoru i Treści 
*   **Lista rozwijana `[Wzór treści rozmowy lub telefonogramu:]` (np. obiekt `QComboBox`)**:
    *   Styl rozwijanej listy zablokowany do wyboru predefiniowanych pozycji (brak swobodnego wpisywania własnych wartości).
    *   Kliknięcie LPM rozwija listę szablonów (np. `[bez szablonu]`, `[21] Zamkniecie toru szlakowego.`, `[22] Otwarcie toru szlakowego.`, `[28] Otwarcie posterunku.`, `[30] Zadanie zatrzymania pociagow.`).
    *   Wybór pozycji LPM automatycznie podstawia do pola poniżej predefiniowany tekst szablonu (np. "Tor szlakowy otwarty o HH:mm") i zmienia jego tło na kolor akcentowy okna. Jeśli wybrano `[bez szablonu]`, tło wraca do białego.
*   **Wieloliniowe pole tekstowe `[Adres i treść rozmowy lub telefonogramu:]` (np. `QTextEdit`)**: 
    *   Wyposażone w pionowy pasek przewijania i wyraźną ramkę zewnętrzną.
    *   Kliknięcie LPM aktywuje kursor umożliwiając wypełnienie wstawionych zmiennych z wybranego szablonu.
*   **Wieloliniowe pole tekstowe `[Dodatkowa notatka:]`**: Analogiczne, przeznaczone na uwagi.

### 2.4 Dolne Przyciski Okna (Prawy dolny róg)
Przyciski są spłaszczone (flat), tło lekko szare (`RGB: 225, 225, 225`), ramka szara (`RGB: 180, 180, 180`), wymiary 110x35. Ich unikalną cechą jest specjalna animacja przy najeżdżaniu, wywoływana timerem z pętli zdarzeń (co 15ms):
*   **Przycisk `[Zapisz]`**: 
    *   Najechanie kursorem (enterEvent): Uruchamia animację rysowania, stopniowo zwiększając parametr `saveProgress` od 0.0 do 1.0, zmieniając kolor/akcent przycisku w czasie rzeczywistym poprzez callback/metodę rysującą okna.
    *   Opuszczenie kursora (leaveEvent): Odwraca i cofa animację z powrotem do zera.
    *   Kliknięcie LPM: Kompletuje model rekordu telefonogramu, wrzuca wpis do głównej struktury stanu, odsyła do rodzica sygnał akceptacji i zwalnia zasoby okna.
*   **Przycisk `[Anuluj]`**: 
    *   Najechanie kursorem (enterEvent): Uruchamia identyczną animację sprzężoną ze zmienną `cancelProgress`.
    *   Kliknięcie LPM: Odsyła sygnał odrzucenia (reject) do okna nadrzędnego i bezpowrotnie usuwa instancję okna, porzucając wpisane zmiany.

---

## 3. Skutki Wizualne w Dzienniku Ruchu (UI)

Zatwierdzenie okna przyciskiem `[Zapisz]` wywołuje odświeżenie głównego widoku poprzez odpowiedni slot systemu.

### 3.1 Wpis w Tabeli Dziennika
*   Pomyślny zapis owocuje wygenerowaniem specjalnego wiersza przechodzącego w poprzek całej tabeli dziennika (rozdzielającego pociągi).
*   Posiada on wyróżnione tło (jasnobrązowe/szare).
*   W kolumnie wpisana zostaje kompilacja nazwisk w formacie "NazwiskoNadawcy / NazwiskoOdbiorcy".

### 3.2 Ikony Ostrzegawcze w Nagłówku Dziennika
Obok nazwy szlaku w nagłówku pojawiają się odpowiednie graficzne symbole powiązane z zapisanym zdarzeniem:
*   **Tarcza zatrzymania D1 (czerwono-biała tarcza)**: Pojawia się bezpośrednio przy linii toru po zarejestrowaniu telefonogramu np. o zamknięciu toru (wzór `[21]`).
*   **Czerwona słuchawka telefoniczna**: Pojawia się w momencie odnotowania zapowiadania telefonicznego/radiotelefonicznego.
*   **Ikona ostrzegawcza (żółty trójkąt z wykrzyknikiem)**: Informuje o innych wprowadzonych dla danego odcinka obostrzeniach.
*   **Interaktywność Odbudowująca**: System wspomaga dyżurnego poprzez to, że dwuklik LPM na czerwoną tarczę natychmiast otwiera to samo okno telefonogramu, podstawiając wzór `[22] Otwarcie toru szlakowego.`.
