# Interfejs Serwera Automatycznych Zapowiedzi (SAZ)

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję w programie Serwer Automatycznych Zapowiedzi (SAZ), zbudowanym natywnie w C++ (sygnały i sloty, callbacki, wskaźniki na obiekty widżetów). Każde pole, tekst, przycisk oraz okienko są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

---

## 1. Główne Okno Aplikacji (klasa `MainWindow`)

Rozmiar minimalny: 640x420, wyśrodkowane. Tło okna: białe (#FFFFFF).

### 1.1 Górny pasek narzędzi (Toolbar) na jasnoszarym tle
Pasek posiada serię przycisków typu tekst/ikona o przezroczystym tle, które na zdarzenie `onMouseEnter` zmieniają tło na jaśniejsze, a po kliknięciu myszą wciskają się wizualnie (przerysowanie komponentu przez wskaźnik).
* **Przycisk `[Plik]`**: (Placeholder menu głównego).
* **Przycisk `[Narzędzia]`**: (Placeholder menu narzędzi).
* **Przycisk `[Symulacja]`**: 
* **Przycisk `[Ustawienia]` (z ikoną zębatki)**: Kliknięcie LPM otwiera modalne okno konfiguracji SAZ.
* **Przycisk `[O Programie]`**:

### 1.2 Pasek Kategorii Zapowiedzi (Toolbar Pociągowy)
Jasnoniebieskie akcenty przy najechaniu (Hover). Służy do manualnego wywoływania zapowiedzi dla zaznaczonego na liście pociągu (wymagany wskaźnik na wybrany obiekt pociągu):
* **Przycisk z trójkątem (AllGongMenuButton)**: Rozwija menu dla wszystkich gongów.
* **Przycisk `[Zapowiedzi powtarzające]`**: Włącza powtarzanie komunikatów.
* **Przycisk `[Wjedzie]`**: Kliknięcie LPM emituje sygnał wymuszający zapowiedź o wjeździe pociągu (np. 15 minut przed).
* **Przycisk `[Wjeżdża]`**: Kliknięcie LPM emituje sygnał wymuszający zapowiedź, że pociąg wjeżdża w perony.
* **Przycisk `[Wjechał]`**: Kliknięcie LPM wymusza zapowiedź "Pociąg wjechał na tor...".
* **Przycisk `[Stoi]`**: Kliknięcie LPM wymusza zapowiedź "Pociąg stoi przy peronie...".
* **Przycisk `[Odjechał]`**: Kliknięcie LPM wymusza zapowiedź "Pociąg odjechał...".
* **Przycisk `[Opóźniony]` (Z podwójną akcją)**:
  * Kliknięcie LPM w główną sekcję przycisku: wygłasza aktualne opóźnienie pociągu.
  * Kliknięcie LPM w prawą krawędź przycisku (Drop-down): Rozwija menu podręczne z jedną opcją `[Ustaw opóźnienie]`. Kliknięcie w nią instancjuje obiekt klasy `DelaySettingsWindow`.
* **Przycisk `[Oczekuje]`**: Kliknięcie LPM wymusza zapowiedź o oczekiwaniu.
* **Przycisk `[Odwołany]`**: Kliknięcie LPM zapowiada odwołanie pociągu.
* **Przycisk `[Zapowiedzi dodatkowe]`**: Kliknięcie LPM otwiera oddzielne podokno tworząc obiekt `AdditionalAnnouncementWindow` służące do niestandardowych dżingli.

### 1.3 Pasek Filtrów (Pod paskiem zapowiedzi)
Tło `#DDECF3` (jasnoniebieskie). Służy do filtrowania listy.
* **Etykieta "Rozkład jazdy pociągów"**.
* **Etykieta "Filtrowanie ograniczeń"**.
* **Pole rozwijane `[Wybierz Stację]` (Dropdown Widget)**:
  * Kliknięcie LPM w pole: Rozwija pionową listę dostępnych stacji.
  * Najechanie (Hover) na element listy: Zmienia kolor tła elementu na `#DCEBFA`.
  * Kliknięcie LPM w element: Wybiera go, zmienia tło na niebieskie (`#0A73D8`), czcionkę na białą, zwija listę i emituje sygnał filtrujący główną tabelę rozkładu do pokazania pociągów tylko z tej stacji.
* **Pole tekstowe "Szukaj..." (TextEdit Widget)**: Kliknięcie LPM aktywuje kursor. Klawiatura wpisuje znaki. Zdarzenie zmiany tekstu wywołuje callback, który na bieżąco filtruje wiersze pod kątem wpisanej frazy (np. numeru pociągu).

### 1.4 Główna Tabela Pociągów (Data View / Table Widget)
Tabela przedstawia rozkład jazdy.
* **Nagłówki kolumn:** `Nazwa Stacji`, `Nr Pociągu`, `Typ Pociągu`, `DATA`, `Nazwa`, `Trasa`, `PPP`, `POP`, `Opóź. [min]`, `Peron`, `Tor`, `Wagony`, `Nast. Zap.`. Zwykłe najechanie nie wywołuje akcji.
* **Wiersze tabeli**:
  * Najechanie na wiersz: Podświetlenie.
  * Kliknięcie LPM w wiersz: Zaznacza pociąg na stałe (zapisuje wskaźnik na element).
  * Kliknięcie PPM na wierszu: Rozwija **Menu Kontekstowe** zawierające opcje m.in.: `Edytuj następną zapowiedź...`, `Ustaw tor`, `Ustaw peron`, `Opóźnienie...`. Kliknięcie LPM w `Opóźnienie...` instancjuje to samo okno co przycisk na górnym pasku.

### 1.5 Dolna Sekcja: Lista Zapowiedzi i Wskaźniki
* **Sekcja "Zapowiedzi" (List Widget z nagłówkami Czas, Nazwa Stacji, Tekst zapowiedzi)**: Zawiera listę oczekujących komunikatów tekstowych. Kliknięcie PPM na wierszu rozwija menu: `Zmień godzinę...`, `Zmień rodzaj`. Przewijana pionowym elementem suwaka okna.
* **Dolny Pasek Statusu (Czarny pasek graniczny, poniżej jasnoszare tło)**:
  * Etykieta `[Lokalne]`.
  * **Wskaźniki Odtwarzania Stacji (Tablica struktury StationPlaybackIndicators)**: Rząd małych kwadracików (22x16 pikseli). 
    * Gdy zapowiedź dla danej stacji NIE GRA: Kwadracik jest biały.
    * Gdy zapowiedź GRA: Kwadracik świeci się na jaskrawozielono (`#FF59D915`).
    * Najechanie (Hover) kursorem na kwadracik otwiera chmurkę (Tooltip) z nazwą stacji (obsługiwane zdarzeniem myszy).
  * Etykieta `[IP]`.
  * Etykieta połączenia z RBC: np. `[Ebiscreen disconnected]`.
  * Licznik pociągów: np. `[0/0]`.
* **Pasek Logów (Widżet TextEdit)**: Znajduje się na samym dole. Posiada flagę zablokowanej edycji w strukturze elementu (tryb odczytu) - kliknięcie w pole tekstowe pozwala zaznaczyć logi i je skopiować (Ctrl+C), ale pole blokuje sygnały modyfikujące tekst na poziomie pętli zdarzeń.

### 1.6 Pływające Powiadomienia "Odwołanie Pociągu" (Instancje klasy Notification)
Pojawiają się z prawej strony ekranu, nad innymi panelami, w kolorze żółtym (tło `#FFF7D6`, obwódka `#806E35`).
* **Tekst komunikatu (Bold)**: np. "Uwaga, pociąg odwołany".
* **Przycisk `[Wygłoś]`**: Kliknięcie LPM emituje sygnał wymuszający puszczenie audio do głośników na stacji i wywołuje destruktor (zamyka i zwalnia wskaźnik) tego okna ostrzeżenia.
* **Przycisk `[Pomiń]`**: Kliknięcie LPM niszczy to okno (zamyka i zwalnia wskaźnik z pamięci), bez przesyłania zapowiedzi na głośniki.

---

## 2. Okno Modalne "Ustaw opóźnienie" (klasa `DelaySettingsWindow`)

Okno to wyskakuje utworzone dynamicznie jako wyśrodkowane względem rodzica, ma stałe właściwości wymiarów zablokowane na poziomie menedżera układów okien, ma wymiary 780x535 pikseli. Posiada tło szare (`#F7F7F7`).

### 2.1 Pasek tytułowy okna i nagłówek
* Pasek tytułowy okna z czerwonym krzyżykiem `[X]`. Kliknięcie LPM wywołuje destruktor obiektu okna, przerywając ustawianie opóźnienia i zwalniając pamięć.
* Napis pogrubiony "Ustaw opóźnienie pociągu".

### 2.2 Sekcja Informacji o Pociągu (Tło białe, szara obwódka)
Pola z włączoną flagą widżetu blokującą zapis:
* `Pociąg:` [numer_pociągu]
* `Nazwa:` [nazwa_pociągu]
* `Relacja:` [stacja_A - stacja_B]

### 2.3 Sekcja "Gotowe opóźnienie (minuty)" (Lewa strona)
* `PresetDelayPanel` w formie układu typu siatka (Grid). Zawiera kafelki z domyślnymi minutami (np. 5, 10, 15). Kliknięcie LPM w kafelek natychmiastowo aktualizuje (przez callback) wartość po prawej stronie. Pole posiada mechanizm przewijania podpięty pod zdarzenia kółka myszy i suwak.

### 2.4 Sekcja "Własne opóźnienie" (Prawa strona)
* **Okrągły przycisk jednokrotnego wyboru (Radio Button) `[Własna wartość opóźnienia]`**: Kliknięcie LPM ustawia w strukturze kontrolki flagę aktywności (rysuje kropkę) i wyzwala sygnał odblokowania pola tekstowego pod spodem.
* **Pole wpisywania minut `DelayMinutesTextBox`**:
  * Kliknięcie LPM aktywuje kursor (o ile przycisk wyboru na to zezwala). Wpisanie cyfr zmienia opóźnienie. Posiada event filter odrzucający zdarzenia wciśnięcia klawiszy innych niż numeryczne.
* **Przyciski ze strzałkami góra/dół obok pola:**
  * Przycisk `[▲]`: Kliknięcie LPM emituje sygnał do logiki podbijający wartość wskazywaną przez bufor o 1 minutę.
  * Przycisk `[▼]`: Kliknięcie LPM emituje sygnał obniżający wartość o 1 minutę.
* **Lista rozwijana `[Powód opóźnienia]` (Dropdown / List Widget)**:
  * Kliknięcie LPM w białe tło rozwija menu listy najczęstszych powodów opóźnień PKP.
  * Kliknięcie LPM w pozycję na liście zmienia zawartość zmiennej trzymającej wybór i zwija listę.
* **Sekcja wyników (Niebieskie tło `#EEF4FA`)**:
  * Aktualizuje się dynamicznie w locie nasłuchując sygnałów zmiany od powyższych kontrolek:
  * `Przyjazd po opóźnieniu:` [Czas]
  * `Odjazd po opóźnieniu:` [Czas]

### 2.5 Dolne Przyciski Okna (Prawy dolny róg)
* **Przycisk `[Zapisz]`**: Przycisk ma podpięty domyślny sygnał na zdarzenie wciśnięcia klawisza `Enter` w pętli obsługi okna. Kliknięcie LPM (lub sygnał `Enter`) aktywuje slot, który przypisuje wartości do powiązanego wskaźnikiem struktury pociągu. Na końcu wywoływany jest na samym sobie destruktor okna (`delete this;`), zwalniający okno z pamięci, a interfejs główny odbiera sygnał odświeżenia list.
* **Przycisk `[Anuluj]`**: Przycisk ma podpięty slot do nasłuchiwania globalnego zdarzenia klawisza `Escape` okna. Kliknięcie LPM natychmiast wywołuje destruktor okna (`delete this;`), bezpiecznie zwalniając okno z pamięci operacyjnej ignorując niespisane modyfikacje i powracając do pętli zdarzeń głównego okna.
