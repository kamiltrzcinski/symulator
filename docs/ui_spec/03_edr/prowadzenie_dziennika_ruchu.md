# EDR - Prowadzenie Dziennika Ruchu i Rejestracja Pociągów

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny każdy element wizualny i interakcję w interfejsie tabeli dziennika ruchu pociągów (odpowiednik papierowego formularza R-146) oraz oknach dialogowych służących do jego uzupełniania w systemie EDR (architektura C++ np. przy użyciu Qt lub biblioteki ImGui).

---

## 1. Wygląd Głównej Tabeli Dziennika Ruchu (np. widget `QTableView`)

Dzienniki ruchu osadzone są w panelu z tłem `#B0B0B0` (szary), często rozdzielone pionowym paskiem dzielącym widgety (np. `QSplitter`, szerokość 4px). Sama tabela dziennika posiada tło białe (`#FFFFFF`) i kolor siatki `#DADADA`. 

### 1.1 Nagłówki kolumn
Wiersz nagłówków (wysokość 44-66px) podzielony jest na kolumny:
* **Kolumna 0 (Zaczątek):** Niewidoczny tekst nagłówka, wyrównanie do środka.
* **Kolumna 1 (`Nieparzysty`):** Pole dla numeru pociągu nieparzystego.
* **Kolumna 2 (`Parzysty`):** Pole dla numeru pociągu parzystego.
* **Kolumna 3 (`Tor`):** Tor stacyjny. W trybie edycji zamienia się w listę rozwijaną (np. wskaźnik na `QComboBox`).
* **Kolumna 4 (`Droga wolna`):** Godzina dania drogi wolnej.
* **Kolumna 5 (`Pociąg odjechał`):** Rzeczywisty czas odjazdu.
* **Kolumna 6 (`Pociąg przyjechał`):** Rzeczywisty czas przyjazdu.
* **Kolumna 7 (`Uwagi`):** Pole tekstowe na uwagi do pociągu (wyrównane do lewej, z marginesem 4px).
* **Kolumna 8 (`O jeździe poc...`):** Opcjonalna (zależna od szlaku), zawiera pole wyboru (np. instancję `QCheckBox`) do powiadamiania dróżników.

### 1.2 Mechanika Wierszy i Komórek
* **Wysokość wiersza:** Na sztywno ustawiona na 28px.
* **Zaznaczenie (Selection):** Kliknięcie LPM w wiersz podświetla go na kolor żółto-pomarańczowy (`#FFE6A0`) z czarnym tekstem. 
* **Wymagane komórki (Custom Background):** Komórki wymagające uzupełnienia (np. brakujące czasy, puste pole toru) bez zaznaczenia mają jasnoniebieskie tło (`#A2C4E9`). Po zaznaczeniu wiersza, te komórki zachowują swój niebieski kolor, pomagając skupić uwagę.
* **Skreślenie pociągu (Odwołanie):** Wiersze skreślone mają jasnoszare tło (`#E2E2E2`), ciemnoszary tekst (`#555555`), siatkę `#C6C6C6` oraz graficznie rysowane przekreślenie "X" na komórkach z numerem pociągu, wraz z czarną grubą (2px) poziomą linią przekreślającą cały wiersz.
* **Pływające menu (Pasek narzędzi):** Nad tabelą znajduje się pasek nagłówka z ikonami narzędziowymi po prawej stronie (m.in. `przyj_spoza.png`, `wypr_spoza.png`). Przyciski te mają przezroczyste tło, reagują na Hover i Click efektami odrysowywania podświetlenia.

---

## 2. Metody Rejestracji Pociągu

Dyżurny ruchu może zarejestrować pociąg na kilka sposobów:

### Metoda 1: Menu Kontekstowe Rozkładu Jazdy w Wykazie
1. **Co klikam:** PPM na wierszu pociągu w górnym wykazie rozkładu jazdy.
2. **Co się dzieje:** Rozwija się menu kontekstowe z opcjami m.in.:
   * `Wprowadzanie godzin`
   * `[Separator]`
   * `Przyjmij pociąg`
   * `Wypraw pociąg`
   * `[Separator]`
   * `Przyjmij z innego szlaku`
   * `Wypraw na inny szlak`
   * `[Separator]`
   * `Pociąg STÓJ`
   * `Zgłoszenie gotowości`
   * `Analiza pociągu`
3. **Akcja:** Kliknięcie LPM w `Przyjmij pociąg` lub `Wypraw pociąg` powoduje przypisanie pociągu do domyślnego dziennika ruchu.

### Metoda 2: Przeciągnij i Upuść (Drag & Drop) z Wykazu Pociągów
1. **Co klikam:** Dyżurny klika LPM na pociąg w górnej tabeli wykazu, trzyma przycisk i przesuwa kursor.
2. **Co się dzieje (Visual Feedback):** Pojawia się pływająca etykieta "Ghost" (`dragGhostLabel`) w kolorze jaskrawopomarańczowym (`#FF8040`) z czarnym tekstem (Pogrubiony Segoe UI 10), która podąża za kursorem z lekkim przesunięciem. W tym czasie uruchomiony jest timer w pętli zdarzeń działający w ~60fps odświeżający pozycję kursora.
3. **Przełączanie zakładek (Hover):** Jeśli kursor z "Ghostem" znajdzie się nad dolnymi zakładkami (np. element `QTabWidget`), system automatycznie zmieni aktywną zakładkę (poza zakładką `Dziennik telefoniczny`, na którą wrzucać pociągów nie można).
4. **Zwolnienie LPM (Drop):** Upuszczenie nad właściwym Dziennikiem Ruchu dodaje wpis.

### Metoda 3: Ikony Narzędziowe (Przyjęcie / Wyprawienie Pociągu spoza RJ)
1. **Co klikam:** Ikony `przyj_spoza.png` (Przyjmij) lub `wypr_spoza.png` (Wypraw) zlokalizowane po prawej stronie paska nagłówka konkretnego dziennika.
2. **Akcja:** Otwiera się puste okno umożliwiające ręczne dodanie pociągu bez uprzedniego planu w wykazie.

---

## 3. Okno Modalne "Rzeczywisty czas przyjazdu i odjazdu pociągu" (`TimeEntryDialog.cpp`)

Rozmiar sztywny: 640x650 pikseli, wyśrodkowane względem rodzica. Brak możliwości maksymalizacji. Tło jasnoszare (`#E0E0E0`).

### 3.1 Pasek Tytułowy i Nagłówek Okna
* Pasek tytułowy okna z tekstem: "Rzeczywisty czas przyjazdu i odjazdu pociągu".
* Pod paskiem tytułowym Etykieta (Label) tekstowa na całą szerokość: `Pociąg [Numer] / Relacja: [StacjaA] - [StacjaB]`. Czcionka `Segoe UI 14 Bold`, kolor leśnej zieleni (`ForestGreen`), wyśrodkowana.

### 3.2 Sekcja "Przyjazd" (Górny panel)
Panel na tle `#D7D7D7` o wymiarach 560x240 pikseli.
* Etykieta `Przyjazd:` (Granatowy `#000080`, Segoe UI 16 Bold).
* Kontrolka kalendarza (np. `QDateEdit`) w formacie `yyyy-MM-dd`. Kliknięcie LPM w nią powoduje automatyczne rozwinięcie kalendarza.
* Dwie kontrolki numeryczne (np. `QSpinBox`) do wyboru godzin (0-23) i minut (0-59), rozdzielone etykietą z dwukropkiem `:`. Zmiana ich wartości poprzez sygnał valueChanged automatycznie oflagowuje przyjazd jako zmieniony.
* **Ramka grupująca (np. `QGroupBox`) "Dodaj do czasu przyjazdu"**: Szereg małych przycisków z białym zaokrąglonym obramowaniem (promień 6px), służących do szybkiego dodawania minut (+5, +10, ..., +200 min).
* **Ramka grupująca "Dodaj do czasu przyjazdu - teksty"**: Przyciski ze zdefiniowanymi opóźnieniami: np. `Potrącenie [120 min.]`, `Uszkodzenie Sieci [70 min.]`. Kliknięcie przelicza czas.
* Przycisk `[Przepisz czas]`: Widoczny tylko jeśli istnieje sekcja Odjazdu. Kopiuje wartości do dolnego panelu.

### 3.3 Sekcja "Odjazd" (Dolny panel)
Analogiczna do sekcji Przyjazdu (tło `#D7D7D7`), etykieta "Odjazd:". Jeśli dana stacja nie ma odjazdu lub przyjazdu dla tego pociągu, dany panel jest ukrywany, a rozmiar okna (wysokość) dynamicznie kurczy się lub rośnie.

### 3.4 Przyciski Akcji na Dole
Wszystkie przyciski posiadają styl bez widocznych wypukłości (flat), szare tło `#E1E1E1` i ramkę `#B4B4B4`. 
Posiadają własny timer animacji w pętli zdarzeń odświeżany co 15ms. Najechanie kursorem (enterEvent) wywołuje płynne przejście koloru tła do dual-gradientu opartego o błękit (`#CCE8FF`) a krawędzie obramowania zmieniają się na niebieskie (`#0078D7`). Odjechanie kursorem (leaveEvent) powoli wygasza efekt.
* **Przycisk `[Wprowadź przyjazd]`**: (Widoczny jeśli jest Przyjazd). Zapisuje tylko przyjazd i zamyka.
* **Przycisk `[Wprowadź postój (pociąg STÓJ)]`**: Zapisuje pociąg ze specjalną flagą postoju z aktualnym czasem odjazdu.
* **Przycisk `[Zapisz]`**: Zapisuje całe wprowadzone dane obu sekcji.
* **Przycisk `[Anuluj]`**: Zamyka okno bez zapisu (odrzucenie).

---

## 4. Bezpośrednia Edycja w Komórkach Tabeli (In-Place)
1. **Co klikam:** Dyżurny najeżdża kursorem i klika dwukrotnie LPM (DoubleClick) na docelowej komórce np. w kolumnie `Tor` lub `Godzina`.
2. **Co się dzieje:** Delegat komórki modelu danych przechodzi w tryb edycji. W przypadku kolumny `Tor` wewnątrz komórki rysuje się pełnoprawna lista rozwijana (np. `QComboBox`), ograniczona do dostępnych dla danego dziennika torów.
3. **Zatwierdzenie:** Kliknięcie w dowolną inną komórkę, przejście do innego wiersza lub naciśnięcie `Enter` kończy edycję w delegacie, a system odpala walidator / slot połączony z sygnałem zakończenia edycji, np. zdejmując jasnoniebieskie tło wymagalności komórki.
