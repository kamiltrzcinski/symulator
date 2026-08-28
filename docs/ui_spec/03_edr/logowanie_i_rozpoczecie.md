# EDR - Rozpoczęcie Pracy i Logowanie (Zgodnie z implementacją C++)

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję związaną z procesem uruchamiania aplikacji, przyjmowania dyżuru oraz zapoznawania się z sytuacją ruchową w systemie EDR, bazując na architekturze C++ (np. przy użyciu Qt). Każde pole, tekst, przycisk oraz okienko są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

> [!NOTE]
> W aktualnej implementacji pominięto dedykowane, osobne okna "Logowania" i "Określania lokalizacji aplikacji". Aplikacja `MainWindow` uruchamia się bezpośrednio, ładując domyślną przestrzeń roboczą dla wybranej stacji (np. Kraków Główny). Mechanika rozpoczęcia pracy opiera się na wywoływanych modalnie oknach "Przyjęcie dyżuru" oraz "Bieżąca sytuacja ruchowa".

---

## 1. Okno Modalne "Przyjęcie dyżuru" (`ShowDutyAcceptanceDialog`)

Okno to wyskakuje na środku aplikacji jako zablokowany dialog modalny (np. poprzez uruchomienie pętli zdarzeń okna modalnego / `QDialog::exec`). Wykorzystuje wspólną stylizację i ma stałe wymiary 840x520 pikseli skalowane przez DPI.

### 1.1 Pasek tytułowy okna i nagłówek
* Pasek tytułowy systemu z tekstem "Przyjęcie dyżuru" i przyciskiem `[X]`. Kliknięcie w krzyżyk zamyka okno przerywając procedurę.
* **Napis pogrubiony "Przyjęcie dyżuru"** w lewym górnym rogu wewnątrz okna (etykieta na pozycji X=12, Y=24, wymiary 450x22).

### 1.2 Sekcja Informacji Stałych (Tylko do odczytu)
* **Etykieta `Posterunek ruchu:`** (X=12, Y=58).
* **Pole tekstowe z nazwą stacji**: Szerokość 680px (X=146, Y=54). Tło szare zdefiniowane w stylach, obwódka cienka czarna (np. QFrame::Box). Zawiera tekst wpisany wielkimi literami (np. "KRAKÓW GŁÓWNY"). Pole zablokowane przed edycją (ustawione na tryb tylko do odczytu). Kliknięcie pozwala na zaznaczenie tekstu, ale nie zmianę.
* **Etykieta `Dyżurny ruchu:`** (X=12, Y=88).
* **Pole tekstowe z użytkownikiem**: Szerokość 680px (X=146, Y=84). Tło szare, obwódka prosta. Zablokowane. Wyświetla pełne Imię i Nazwisko zalogowanego użytkownika ze struktury stanu aplikacji.
* **Etykieta `Godzina zakonczenia ostatniego dyzuru:`** (X=12, Y=122).
* **Pole tekstowe z czasem ostatniego dyżuru**: Szerokość 100px (X=380, Y=116). Tło szare, obwódka prosta. Zablokowane. Wyświetla czas (format HH:mm) lub pozostaje puste, jeśli nie odnaleziono poprzedniego dyżuru.

### 1.3 Sekcja Czasu i Uwag (Pola edytowalne)
* **Etykieta `Godzina przyjęcia dyżuru:`** (X=12, Y=156).
* **Kontrolka wyboru czasu (np. `QTimeEdit`)**: Szerokość 100px (X=146, Y=150). Format wyłącznie godzinowy (`HH:mm`), zawiera pionowe strzałki do zmiany wartości. 
  * Kliknięcie LPM w cyfry uaktywnia je. Strzałki klawiatury Góra/Dół lub kliknięcie w przyciski sterujące zmienia minuty/godziny. Domyślnie wstawia czas bieżący lub czas zakończenia poprzedniego dyżuru (jeśli od jego zakończenia minęło mniej niż 30 minut).
* **Etykieta `Uwagi dodatkowe (zostana zapisane w kazdym dzienniku ruchu i dzienniku telefonicznym):`** (X=12, Y=220).
* **Wieloliniowe pole tekstowe (np. `QTextEdit`)**: Szerokie i wysokie pole wieloliniowe na wymiar 814x110px (X=12, Y=244), obwódka prosta. Posiada włączony pionowy pasek przewijania. Kliknięcie LPM aktywuje kursor. Wpisany tutaj tekst z klawiatury zostanie przekopiowany do wszystkich dzienników.

### 1.4 Dolny Pasek Akcji (Bottom Dock)
Zintegrowany panel przypięty do dołu, zawierający prawostronnie wyjustowane przyciski.
* **Przycisk `[Zapisz]`**: Przycisk główny, którego sygnał kliknięcia powiązany jest z domyślną akcją zatwierdzenia (wciskany klawiszem `Enter` z klawiatury). Szerokość 132px (X=556, Y=7).
  * Kliknięcie LPM otwiera kolejne małe okno MessageBox z zapytaniem: *"Godzina przyjęcia dyżuru to HH:mm. Czy na pewno?"* oraz przyciskami `[Tak]` / `[Nie]`. 
  * Kliknięcie `[Tak]` w MessageBoxie modyfikuje stan aplikacji, wrzuca do dzienników ruchu wiersz "Dyżur przyjął [Użytkownik] dnia [Data]..." i zwalnia zasoby całego okna "Przyjęcia dyżuru".
* **Przycisk `[Anuluj]`**: Z sygnałem powiązanym z odrzuceniem (wciskany klawiszem `Escape` z klawiatury). Szerokość 132px (X=702, Y=7). Kliknięcie LPM zamyka okno bez zapisu, wracając do głównej aplikacji.

---

## 3. Okno Modalne "Posterunek ruchu - sytuacja biezaca" (`TrackSituationDialog.cpp`)

Wyświetla się na środku ekranu wymuszając zapoznanie się ze stanem torów szlakowych i dzienników ruchu na szlakach przyległych. Wymiary okna: 820x380 pikseli.

### 3.1 Pasek tytułowy okna i panel nagłówkowy (Header)
* Pasek tytułowy systemu z tekstem "Posterunek ruchu - sytuacja biezaca".
* **Górny panel (`header`)**: Wymiary 780x46px na pozycji X=12, Y=18. Tło wypełnione jednolitym kolorem dla nagłówków.
* **Etykieta `lblStation`**: Napis pogrubiony "Posterunek: [NAZWA STACJI WIELKIMI LITERAMI]", pozycja X=8, Y=8.
* **Etykieta `lblInfo`**: Napis "Biezaca sytuacja ruchowa na szlakach przyleglych:", pozycja X=8, Y=28.

### 3.2 Lista Sytuacji Torowych (Panel `list`)
Znajduje się poniżej nagłówka (pozycja X=12, Y=72). Rozmiar 780x226 pikseli z białym tłem. Jeśli pozycji jest więcej, pojawia się automatyczny boczny pasek przewijania (scroll widget). Wewnątrz generowane są jeden pod drugim osobne kafelki dla każdego toru.
* **Pojedynczy Wiersz Sytuacji Torowej (`row`)**: Wymiary 760x70px (szerokość panelu minus 20px). Tło białe.
  * Zdarzenie rysowania / callback malujący w pętli zdarzeń: Generuje stałą, dolną linię oddzielającą koloru szarego (RGB 190, 190, 190) o grubości 1px na całej długości.
  * **Etykieta `lblJournal`**: "Dziennik ruchu: [Nazwa Szlaku]", pozycja X=4, Y=6.
  * **Etykieta "Tor szlakowy nr:"**: pozycja X=18, Y=34.
  * **Etykieta wartości toru (`lblTrackValue`)**: Pogrubiona, na pozycji X=154, Y=34.
  * **Etykieta statusu (`lblStatus`)**: Pogrubiona czcionka, na pozycji X=236, Y=34. Wyświetla napis "Status: [WOLNY / ZAJĘTY / Inny]".
    * W momencie gdy status jest poprawny/normalny (`IsNormal == true`): Tekst renderuje się w kolorze **zielonym**.
    * W przeciwnym razie: Tekst renderuje się w kolorze **czerwonym**.

### 3.3 Dolny Pasek Akcji (Bottom Dock)
* **Przycisk `[OK]`**: Główny przycisk "Primary" o szerokości 84px na pozycji X=692, Y=7. Powiązany zarówno z akcją akceptacji, jak i odrzucenia (reaguje na `Enter` i `Escape`). Kliknięcie LPM przypisuje oknu odpowiedni kod powrotu z pętli zdarzeń, ukrywa je i odblokowuje dostęp do Głównego Okna EDR, kończąc proces rozpoczynania pracy.
