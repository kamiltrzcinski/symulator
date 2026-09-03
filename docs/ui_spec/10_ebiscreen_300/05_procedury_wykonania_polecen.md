# Procedury wykonania poleceń w EBI Screen 300

Dokument ten zawiera łopatologiczne, krokowe opisy fizycznego wykonania każdej komendy dostępnej w systemie EBI Screen 300, opierając się na specyfikacji interfejsu użytkownika (mysz i klawiatura).

## Polecenie BKO
**Pełna nazwa:** Blokowanie bloku końcowego
**Argumenty:** ``tor``
**Opis:** Zablokowanie bloku końcowego blokady typu C wprowadzane po wjeździe pociągu na stację.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BKO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BKO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BLA
**Pełna nazwa:** Awaryjna zmiana kierunku blokady
**Argumenty:** ``tor``
**Opis:** Zmiana kierunku blokady z wyjazdu na wjazd z jednoczesnym zastopowaniem blokady lub ustawienie na wjazd.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BLA** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BLA**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BLAI
**Pełna nazwa:** Inicjalizacja awaryjnej zmiany kierunku blokady
**Argumenty:** ``tor``
**Opis:** Pierwszy etap procesu awaryjnej zmiany kierunku blokady.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BLAI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BLAI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BLO
**Pełna nazwa:** Przerwanie ustawiania kierunku blokady / Doraźne zwolnienie
**Argumenty:** ``tor``
**Opis:** Umożliwia przerwanie ustawiania kierunku blokady na wyjazd lub doraźne zwolnienie blokady bez przejazdu pociągu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BLO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BLO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BLP
**Pełna nazwa:** Pozwolenie na zmianę kierunku blokady
**Argumenty:** ``tor``
**Opis:** Wysłanie sygnału zwrotnego na sąsiednią stację zezwalającego na zmianę kierunku.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BLP** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BLP**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BLS
**Pełna nazwa:** Stopowanie blokady / Zamykanie wyjazdu
**Argumenty:** ``tor``
**Opis:** Uniemożliwienie wyprawiania pociągów z posterunku dla danego toru szlakowego.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BLS** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BLS**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BLW
**Pełna nazwa:** Ustawienie kierunku blokady na wyjazd
**Argumenty:** ``tor``
**Opis:** Wysłanie na sąsiednią stację żądania ustawienia kierunku blokady liniowej.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BLW** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BLW**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BLZ
**Pełna nazwa:** Zwolnienie kierunku blokady
**Argumenty:** ``tor``
**Opis:** Zwolnienie kierunku blokady liniowej bez konieczności obsługi przez dyżurnego sąsiedniej stacji (lub po przejeździe pociągu).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BLZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BLZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BPO
**Pełna nazwa:** Blokowanie bloku początkowego Po
**Argumenty:** ``tor``
**Opis:** Zablokowanie bloku początkowego blokady typu C po wyjeździe pociągu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BPO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BPO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BPZ
**Pełna nazwa:** Blokowanie bloku pozwolenia
**Argumenty:** ``tor``
**Opis:** Umożliwia ustawienie kierunku blokady na wjazd (blokada typu C).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BPZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BPZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BTO
**Pełna nazwa:** Odwołanie wyprawienia pociągu z telef. zapow.
**Argumenty:** ``tor``
**Opis:** Odwołanie polecenia BTW.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BTO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BTO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie BTW
**Pełna nazwa:** Wyprawienie pociągu na szlak z telef. zapow.
**Argumenty:** ``tor``
**Opis:** Polecenie wymagane do wyprawienia pociągu przy telefonicznym zapowiadaniu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **BTW** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **BTW**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DKO
**Pełna nazwa:** Doraźne blokowanie bloku Ko / bloku końcowego
**Argumenty:** ``tor``
**Opis:** Umożliwia zwolnienie blokady po przyjęciu pociągu na sygnał zastępczy (lub blokowanie bloku końcowego Ko).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DKO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DKO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DKOI
**Pełna nazwa:** Inicjalizacja doraźnego blokowania bloku Ko
**Argumenty:** ``tor``
**Opis:** Pierwszy etap procesu doraźnego blokowania bloku Ko.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DKOI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DKOI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DKP
**Pełna nazwa:** Doraźne stwierdzenie końca pociągu
**Argumenty:** ``tor``
**Opis:** Wykorzystywane przy niesprawności urządzeń stwierdzania końca pociągu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DKP** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DKP**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DKPI
**Pełna nazwa:** Inicjalizacja doraźnego stwierdzenia końca pociągu
**Argumenty:** ``tor``
**Opis:** Pierwszy etap procesu doraźnego stwierdzenia końca pociągu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DKPI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DKPI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DPO
**Pełna nazwa:** Doraźne blokowanie bloku Po / początkowego
**Argumenty:** ``tor``
**Opis:** Doraźne zablokowanie bloku Po w razie wyjazdu pociągu na sygnał zastępczy/rozkaz.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DPO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DPO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DPOI
**Pełna nazwa:** Inicjalizacja doraźnego blokowania bloku Po
**Argumenty:** ``tor``
**Opis:** Pierwszy etap procesu doraźnego blokowania bloku Po.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DPOI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DPOI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DPW
**Pełna nazwa:** Awaryjne odblokowanie blokady
**Argumenty:** ``tor``
**Opis:** Doraźne przywrócenie stanu zasadniczego blokady ustawionej na wyjazd.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DPW** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DPW**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DPWI
**Pełna nazwa:** Inicjalizacja awaryjnego odblokowania blokady
**Argumenty:** ``tor``
**Opis:** Pierwszy etap doraźnego przywrócenia stanu zasadniczego blokady.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DPWI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DPWI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ITO
**Pełna nazwa:** Odwołanie indywidualnego zamknięcia toru/zwrotnicy
**Argumenty:** ``tor`, `zwrotnica``
**Opis:** Odwołuje zamknięcie ruchowe zwrotnicy lub toru.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ITO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ITO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor, zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor, zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ITS
**Pełna nazwa:** Indywidualne zamknięcie ruchowe toru/zwrotnicy
**Argumenty:** ``tor`, `zwrotnica``
**Opis:** Zamknięcie obwodu torowego/zwrotnicy dla przebiegów pociągowych i manewrowych.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ITS** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ITS**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor, zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor, zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie MRS
**Pełna nazwa:** Przełączenie komputera "standby" / "online"
**Argumenty:** ``nazwa komputera``
**Opis:** Przełączenie strony komputera zależnościowego ze stanu "standby" do "online" z restartem.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (nazwa komputera) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **MRS** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **MRS**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (nazwa komputera), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (nazwa komputera) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ODS
**Pełna nazwa:** Odświeżenie danych o obiektach
**Argumenty:** ``oznaczenie stacji``
**Opis:** Wykasowanie i ponowne załadowanie z komputera zależnościowego informacji o stanach.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (oznaczenie stacji) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ODS** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ODS**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (oznaczenie stacji), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (oznaczenie stacji) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie OPS
**Pełna nazwa:** Odwołanie polecenia specjalnego
**Argumenty:** ``zwrotnica`, `tor`, `przejazd``
**Opis:** Odwołuje inicjalizację polecenia specjalnego lub polecenie specjalne w trakcie jego trwania.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **OPS** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **OPS**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica, tor, przejazd), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica, tor, przejazd) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie OST
**Pełna nazwa:** Odwołanie stopowania blokady
**Argumenty:** ``tor``
**Opis:** Odwołanie stopowania blokady (np. po poleceniu BLS).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **OST** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **OST**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie OZK
**Pełna nazwa:** Odwołanie zmiany kierunku blokady
**Argumenty:** ``tor``
**Opis:** Przejście blokady do stanu sprzed zainicjalizowania zmiany kierunku (ZKB).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **OZK** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **OZK**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PAZ
**Pełna nazwa:** Awaryjne zamknięcie przejazdu
**Argumenty:** ``przejazd``
**Opis:** Awaryjne (bez wstępnego ostrzegania) zamknięcie przejazdu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (przejazd) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PAZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PAZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (przejazd), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (przejazd) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PDI
**Pełna nazwa:** Doraźne odwołanie utwierdzenia
**Argumenty:** ``przejazd``
**Opis:** Doraźne odwołanie utwierdzenia przejazdu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (przejazd) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PDI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PDI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (przejazd), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (przejazd) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PDII
**Pełna nazwa:** Inicjalizacja doraźnego odwołania utwierdzenia
**Argumenty:** ``przejazd``
**Opis:** Pierwszy etap doraźnego odwołania utwierdzenia przejazdu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (przejazd) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PDII** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PDII**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (przejazd), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (przejazd) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PDO
**Pełna nazwa:** Otwarcie przejazdu
**Argumenty:** ``przejazd``
**Opis:** Otworzenie przejazdu kat. A.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (przejazd) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PDO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PDO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (przejazd), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (przejazd) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PDZ
**Pełna nazwa:** Zamknięcie przejazdu
**Argumenty:** ``przejazd``
**Opis:** Zamknięcie przejazdu kat. A.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (przejazd) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PDZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PDZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (przejazd), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (przejazd) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie POC
**Pełna nazwa:** Ustawienie przebiegu pociągowego
**Argumenty:** ``semafor`... `semafor (tor)``
**Opis:** Ustawienie przebiegu pociągowego od pierwszego do ostatniego podanego semafora/toru.

### Metoda 1: Drag & Drop (Przeciągnij i upuść)
1. Najedź kursorem myszy na obiekt początkowy (np. semafor). Wokół niego pojawi się żółta ramka.
2. Wciśnij i przytrzymaj Lewy Przycisk Myszy (LPM). Ramka zmieni kolor na lazurowy (obiekt zostanie zaznaczony).
3. Nie zwalniając przycisku myszy, przeciągnij kursor nad obiekt docelowy (semafor końcowy lub tor). W trakcie przeciągania kursor zmieni swój wygląd informując o poprawności operacji.
4. Gdy znajdziesz się nad właściwym obiektem docelowym, pojawi się wokół niego żółta ramka. Zwolnij LPM (Drop).
5. Wybierz polecenie **POC** z otwartego menu/okna poleceń (lub z listy, która się otworzy po upuszczeniu). Kursor automatycznie przeskoczy na przycisk "Wyślij", a ramki obiektów zmienią kolor na zielony.
6. Kliknij przycisk "Wyślij", aby zatwierdzić wykonanie polecenia do systemu zależnościowego.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **POC**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM kolejno na obiekt początkowy, a następnie na docelowy, których ma dotyczyć komenda. Zostaną one otoczone zielonymi ramkami.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza i wyborze obiektów ramka zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie POZ
**Pełna nazwa:** Pozwolenie na ustawienie kierunku na wjazd
**Argumenty:** ``tor``
**Opis:** Umożliwia ustawienie kierunku blokady na wjazd na stację.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **POZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **POZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PZA
**Pełna nazwa:** Ręczne awaryjne zwolnienie przebiegu
**Argumenty:** ``semafor`... `semafor (tor)``
**Opis:** Natychmiastowe zwolnienie przebiegu (poprzedzone PZAI).

### Metoda 1: Drag & Drop (Przeciągnij i upuść)
1. Najedź kursorem myszy na obiekt początkowy (np. semafor). Wokół niego pojawi się żółta ramka.
2. Wciśnij i przytrzymaj Lewy Przycisk Myszy (LPM). Ramka zmieni kolor na lazurowy (obiekt zostanie zaznaczony).
3. Nie zwalniając przycisku myszy, przeciągnij kursor nad obiekt docelowy (semafor końcowy lub tor). W trakcie przeciągania kursor zmieni swój wygląd informując o poprawności operacji.
4. Gdy znajdziesz się nad właściwym obiektem docelowym, pojawi się wokół niego żółta ramka. Zwolnij LPM (Drop).
5. Wybierz polecenie **PZA** z otwartego menu/okna poleceń (lub z listy, która się otworzy po upuszczeniu). Kursor automatycznie przeskoczy na przycisk "Wyślij", a ramki obiektów zmienią kolor na zielony.
6. Kliknij przycisk "Wyślij", aby zatwierdzić wykonanie polecenia do systemu zależnościowego.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PZA**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM kolejno na obiekt początkowy, a następnie na docelowy, których ma dotyczyć komenda. Zostaną one otoczone zielonymi ramkami.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza i wyborze obiektów ramka zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PZAI
**Pełna nazwa:** Inicjalizacja ręcznego awaryjnego zwolnienia
**Argumenty:** ``semafor`... `semafor (tor)``
**Opis:** Pierwszy etap ręcznego awaryjnego zwolnienia przebiegu.

### Metoda 1: Drag & Drop (Przeciągnij i upuść)
1. Najedź kursorem myszy na obiekt początkowy (np. semafor). Wokół niego pojawi się żółta ramka.
2. Wciśnij i przytrzymaj Lewy Przycisk Myszy (LPM). Ramka zmieni kolor na lazurowy (obiekt zostanie zaznaczony).
3. Nie zwalniając przycisku myszy, przeciągnij kursor nad obiekt docelowy (semafor końcowy lub tor). W trakcie przeciągania kursor zmieni swój wygląd informując o poprawności operacji.
4. Gdy znajdziesz się nad właściwym obiektem docelowym, pojawi się wokół niego żółta ramka. Zwolnij LPM (Drop).
5. Wybierz polecenie **PZAI** z otwartego menu/okna poleceń (lub z listy, która się otworzy po upuszczeniu). Kursor automatycznie przeskoczy na przycisk "Wyślij", a ramki obiektów zmienią kolor na zielony.
6. Kliknij przycisk "Wyślij", aby zatwierdzić wykonanie polecenia do systemu zależnościowego.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PZAI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM kolejno na obiekt początkowy, a następnie na docelowy, których ma dotyczyć komenda. Zostaną one otoczone zielonymi ramkami.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza i wyborze obiektów ramka zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PZAO
**Pełna nazwa:** Odwołanie inicjalizacji awaryjnego zwolnienia
**Argumenty:** ``semafor`... `semafor (tor)``
**Opis:** Odwołanie polecenia PZAI.

### Metoda 1: Drag & Drop (Przeciągnij i upuść)
1. Najedź kursorem myszy na obiekt początkowy (np. semafor). Wokół niego pojawi się żółta ramka.
2. Wciśnij i przytrzymaj Lewy Przycisk Myszy (LPM). Ramka zmieni kolor na lazurowy (obiekt zostanie zaznaczony).
3. Nie zwalniając przycisku myszy, przeciągnij kursor nad obiekt docelowy (semafor końcowy lub tor). W trakcie przeciągania kursor zmieni swój wygląd informując o poprawności operacji.
4. Gdy znajdziesz się nad właściwym obiektem docelowym, pojawi się wokół niego żółta ramka. Zwolnij LPM (Drop).
5. Wybierz polecenie **PZAO** z otwartego menu/okna poleceń (lub z listy, która się otworzy po upuszczeniu). Kursor automatycznie przeskoczy na przycisk "Wyślij", a ramki obiektów zmienią kolor na zielony.
6. Kliknij przycisk "Wyślij", aby zatwierdzić wykonanie polecenia do systemu zależnościowego.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PZAO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM kolejno na obiekt początkowy, a następnie na docelowy, których ma dotyczyć komenda. Zostaną one otoczone zielonymi ramkami.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza i wyborze obiektów ramka zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PZM
**Pełna nazwa:** Pozwolenie na zmianę kierunku blokady
**Argumenty:** ``tor``
**Opis:** Wysłanie sygnału zwrotnego do posterunku żądającego zmiany kierunku (przy ZKB).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PZM** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PZM**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PZO
**Pełna nazwa:** Załączenie/Wyłączenie ostrzegania
**Argumenty:** ``przejazd``
**Opis:** Załączenie wyłączonego lub wyłączenie załączonego sygnalizatora drogowego.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (przejazd) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PZO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PZO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (przejazd), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (przejazd) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PZT
**Pełna nazwa:** Zał./Wył. tarcz ostrzegawczych przejazdu
**Argumenty:** ``przejazd``
**Opis:** Załączenie lub wyłączenie tarcz ostrzegawczych przejazdu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (przejazd) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PZT** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PZT**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (przejazd), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (przejazd) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PZW
**Pełna nazwa:** Ręczne zwolnienie przebiegu
**Argumenty:** ``sygnalizator``
**Opis:** Zwolnienie przebiegu pociągowego lub manewrowego (warunkowo czasowe lub natychmiastowe).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (sygnalizator) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PZW** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PZW**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (sygnalizator), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (sygnalizator) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PZZ
**Pełna nazwa:** Pozwolenie na zerowanie >1 odstępu
**Argumenty:** ``tor``
**Opis:** Pozwolenie wydane dla sąsiedniego posterunku na zerowanie wielu odstępów.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PZZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PZZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PZZI
**Pełna nazwa:** Inicjalizacja pozwolenia na zerowanie >1 odstępu
**Argumenty:** ``tor``
**Opis:** Pierwszy etap procesu wydania pozwolenia PZZ.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PZZI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PZZI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SAM
**Pełna nazwa:** Włączenie samoczynności
**Argumenty:** ``semafor``
**Opis:** Włączenie samoczynności nastawiania przebiegu od wskazanego semafora.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (semafor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SAM** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SAM**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (semafor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (semafor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SAW
**Pełna nazwa:** Wyłączenie samoczynności
**Argumenty:** ``semafor``
**Opis:** Wyłączenie samoczynności nastawiania przebiegu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (semafor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SAW** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SAW**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (semafor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (semafor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SEO
**Pełna nazwa:** Odwołanie stopowania sygnalizatora
**Argumenty:** ``sygnalizator``
**Opis:** Umożliwia ponowne podawanie sygnałów zezwalających na sygnalizatorze.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (sygnalizator) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SEO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SEO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (sygnalizator), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (sygnalizator) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SES
**Pełna nazwa:** Stopowanie sygnalizatora
**Argumenty:** ``sygnalizator``
**Opis:** Ustawia sygnał "stój" na podanym sygnalizatorze (blokuje podanie sygnału zezwalającego).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (sygnalizator) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SES** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SES**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (sygnalizator), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (sygnalizator) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SLI
**Pełna nazwa:** Inicjalizacja zerowania ilości osi
**Argumenty:** ``zwrotnica`, `tor`, `sekcja``
**Opis:** Inicjalizacja przygotowująca system do przyjęcia właściwego polecenia zerowania (SLK).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SLI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SLI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica, tor, sekcja), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica, tor, sekcja) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SLK
**Pełna nazwa:** Zerowanie ilości osi
**Argumenty:** ``zwrotnica`, `tor`, `sekcja``
**Opis:** Zerowanie ilości zliczonych osi w danej sekcji/odstępie.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SLK** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SLK**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica, tor, sekcja), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica, tor, sekcja) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SSO
**Pełna nazwa:** Odwołanie stopowania wszystkich sygnalizatorów
**Argumenty:** ``oznaczenie stacji``
**Opis:** Odwołuje polecenie SSS dla całej stacji.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (oznaczenie stacji) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SSO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SSO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (oznaczenie stacji), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (oznaczenie stacji) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SSS
**Pełna nazwa:** Stopowanie wszystkich sygnalizatorów
**Argumenty:** ``oznaczenie stacji``
**Opis:** Ustawienie sygnału "stój" na wszystkich sygnalizatorach na stacji.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (oznaczenie stacji) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SSS** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SSS**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (oznaczenie stacji), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (oznaczenie stacji) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SZI
**Pełna nazwa:** Inicjalizacja nastawienia sygnału zastępczego
**Argumenty:** ``semafor``
**Opis:** Pierwszy etap podawania sygnału zastępczego Sz.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (semafor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SZI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SZI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (semafor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (semafor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SZN
**Pełna nazwa:** Sygnał zastępczy na tor niewłaściwy
**Argumenty:** ``semafor``
**Opis:** Podanie sygnału zastępczego Sz z zaświeceniem wskaźnika W24 (na tor niewłaściwy).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (semafor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SZN** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SZN**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (semafor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (semafor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SZO
**Pełna nazwa:** Odwołanie sygnału zastępczego
**Argumenty:** ``oznaczenie stacji``
**Opis:** Odwołuje sygnał zastępczy SZW, SZN lub kasuje inicjalizację SZI (wprowadzane przez menu stacji).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (oznaczenie stacji) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SZO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SZO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (oznaczenie stacji), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (oznaczenie stacji) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie SZW
**Pełna nazwa:** Podanie sygnału zastępczego
**Argumenty:** ``semafor``
**Opis:** Właściwe podanie sygnału zastępczego (po SZI).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (semafor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **SZW** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **SZW**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (semafor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (semafor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie UPA
**Pełna nazwa:** Awaryjne odebranie uprawnień do sterowania
**Argumenty:** ``okręg sterowania``
**Opis:** Wymusza awaryjne oddanie uprawnień do sterowania stacją (poprzedzone UPAI).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (okręg sterowania) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **UPA** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **UPA**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (okręg sterowania), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (okręg sterowania) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie UPAI
**Pełna nazwa:** Inicjalizacja awaryjnego odebrania uprawnień
**Argumenty:** ``okręg sterowania``
**Opis:** Pierwszy etap awaryjnego odebrania uprawnień do sterowania stacją.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (okręg sterowania) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **UPAI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **UPAI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (okręg sterowania), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (okręg sterowania) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie UPAO
**Pełna nazwa:** Odwołanie inicjalizacji awaryjnego odebrania upr.
**Argumenty:** ``okręg sterowania``
**Opis:** Odwołuje polecenie UPAI.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (okręg sterowania) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **UPAO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **UPAO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (okręg sterowania), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (okręg sterowania) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie UPN
**Pełna nazwa:** Nadanie uprawnień do sterowania stacją
**Argumenty:** ``okręg sterowania``
**Opis:** Nadanie uprawnień do sterowania dla danego systemu nadrzędnego.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (okręg sterowania) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **UPN** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **UPN**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (okręg sterowania), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (okręg sterowania) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie UPO
**Pełna nazwa:** Oddanie uprawnień do sterowania stacją
**Argumenty:** ``okręg sterowania``
**Opis:** Zwykłe oddanie uprawnień do sterowania stacją do innego systemu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (okręg sterowania) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **UPO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **UPO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (okręg sterowania), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (okręg sterowania) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZAL
**Pełna nazwa:** Załączenie zasilania
**Argumenty:** ``sieć zasilająca``
**Opis:** Załączenie zasilania z wybranej sieci.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (sieć zasilająca) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZAL** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZAL**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (sieć zasilająca), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (sieć zasilająca) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZBM
**Pełna nazwa:** Przestawienie zwrotnicy do położenia "-" (zależn. wył.)
**Argumenty:** ``zwrotnica``
**Opis:** Przestawienie do położenia "-" zwrotnicy z uprzednio wyłączonym obwodem kontroli (ZWB).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZBM** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZBM**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZBP
**Pełna nazwa:** Przestawienie zwrotnicy do położenia "+" (zależn. wył.)
**Argumenty:** ``zwrotnica``
**Opis:** Przestawienie do położenia "+" zwrotnicy z uprzednio wyłączonym obwodem kontroli (ZWB).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZBP** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZBP**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZES
**Pełna nazwa:** Zerowanie więcej niż jednego odstępu
**Argumenty:** ``tor``
**Opis:** Właściwe zerowanie osi dla wielu odstępów blokady liniowej.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZES** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZES**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZESI
**Pełna nazwa:** Inicjalizacja zerowania więcej niż jednego odstępu
**Argumenty:** ``tor``
**Opis:** Pierwszy etap zerowania wielu odstępów blokady.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZESI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZESI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZKB
**Pełna nazwa:** Zmiana kierunku blokady
**Argumenty:** ``tor``
**Opis:** Zasadnicza zmiana kierunku blokady (inicjowana ze stacji z kierunkiem na wjazd).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (tor) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZKB** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZKB**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (tor), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (tor) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZRI
**Pełna nazwa:** Inicjalizacja wyłączenia sygnalizacji rozprucia
**Argumenty:** ``zwrotnica``
**Opis:** Pierwszy etap procesu kasowania rozprucia zwrotnicy.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZRI** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZRI**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZRK
**Pełna nazwa:** Wyłączenie sygnalizacji rozprucia zwrotnicy
**Argumenty:** ``zwrotnica``
**Opis:** Właściwe wyłączenie sygnalizacji rozprucia (po ZRI).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZRK** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZRK**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZSO
**Pełna nazwa:** Odwołanie stopowania wszystkich zwrotnic
**Argumenty:** ``oznaczenie stacji``
**Opis:** Odwołuje stopowanie zwrotnic i wykolejnic dla całej stacji (ZSS).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (oznaczenie stacji) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZSO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZSO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (oznaczenie stacji), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (oznaczenie stacji) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZSS
**Pełna nazwa:** Stopowanie wszystkich zwrotnic
**Argumenty:** ``oznaczenie stacji``
**Opis:** Trwałe odłączenie napięcia nastawczego od wszystkich zwrotnic na stacji.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (oznaczenie stacji) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZSS** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZSS**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (oznaczenie stacji), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (oznaczenie stacji) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZWB
**Pełna nazwa:** Wyłączenie kontroli niezajętości zwrotnicy
**Argumenty:** ``zwrotnica``
**Opis:** Wyłączenie obwodu kontroli niezajętości zwrotnicy w celu jej awaryjnego przestawienia.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZWB** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZWB**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZWM
**Pełna nazwa:** Przestawienie zwrotnicy do położenia "-"
**Argumenty:** ``zwrotnica``
**Opis:** Zasadnicze przestawienie zwrotnicy/wykolejnicy do położenia "-".

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZWM** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZWM**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZWO
**Pełna nazwa:** Odwołanie zastopowania zwrotnicy
**Argumenty:** ``zwrotnica``
**Opis:** Odwołanie indywidualnego stopowania zwrotnicy (ZWS).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZWO** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZWO**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZWP
**Pełna nazwa:** Przestawienie zwrotnicy do położenia "+"
**Argumenty:** ``zwrotnica``
**Opis:** Zasadnicze przestawienie zwrotnicy/wykolejnicy do położenia "+".

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZWP** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZWP**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZWS
**Pełna nazwa:** Zastopowanie zwrotnicy
**Argumenty:** ``zwrotnica``
**Opis:** Indywidualne zastopowanie zwrotnicy (odłączenie napięcia nastawczego).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (zwrotnica) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZWS** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZWS**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (zwrotnica), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (zwrotnica) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DOZP
**Pełna nazwa:** Doraźne odwołanie zgody
**Argumenty:** ``blok Dz``
**Opis:** Skutkuje odebraniem zgody, uniemożliwia nastawienie przebiegu lub wygasza sygnał zezwalający.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (blok Dz) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DOZP** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DOZP**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (blok Dz), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (blok Dz) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DWS
**Pełna nazwa:** Doraźne wygaszenie semafora
**Argumenty:** ``blok Dz``
**Opis:** Natychmiastowe wygaszenie sygnału zezwalającego na semaforze wyjazdowym z sąsiedniego okręgu.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (blok Dz) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DWS** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DWS**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (blok Dz), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (blok Dz) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie DZ
**Pełna nazwa:** Danie zgody
**Argumenty:** ``blok Dz``
**Opis:** Danie zgody dla bloku blokady stacyjnej.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (blok Dz) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **DZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **DZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (blok Dz), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (blok Dz) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie OZZ
**Pełna nazwa:** Odwołanie żądania zgody
**Argumenty:** ``blok Oz``
**Opis:** Umożliwia wycofanie się z żądania zgody wydanego dla bloku Oz blokady stacyjnej.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (blok Oz) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **OZZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **OZZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (blok Oz), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (blok Oz) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PSN
**Pełna nazwa:** Przywrócenie stanu neutralnego - inicj.
**Argumenty:** ``blok PSN``
**Opis:** Pierwszy etap procesu wyjścia ze stanu bezpiecznego bloków blokady stacyjnej.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (blok PSN) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PSN** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PSN**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (blok PSN), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (blok PSN) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie PSNP
**Pełna nazwa:** Przywrócenie stanu neutralnego - potw.
**Argumenty:** ``blok PSN``
**Opis:** Zasadnicze przywrócenie stanu neutralnego blokady stacyjnej (zwrot zgody).

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (blok PSN) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **PSNP** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **PSNP**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (blok PSN), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (blok PSN) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZOZ
**Pełna nazwa:** Zwrot otrzymanej zgody (niewykorzystanej)
**Argumenty:** ``blok Oz``
**Opis:** Zwrot niewykorzystanej zgody po wysłaniu do bloku Oz blokady stacyjnej.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (blok Oz) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZOZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZOZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (blok Oz), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (blok Oz) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---

## Polecenie ZZ
**Pełna nazwa:** Żądanie zgody
**Argumenty:** ``blok Oz``
**Opis:** Żądanie zgody od sąsiedniego okręgu nastawczego na blokadzie stacyjnej.

### Metoda 1: Mysz (Menu kontekstowe)
1. Najedź kursorem myszy na obiekt (blok Oz) na planie świetlnym. Wokół obiektu pojawi się natychmiast żółta ramka, sygnalizująca najechanie.
2. Kliknij obiekt Lewym Przyciskiem Myszy (LPM). Ramka zmieni kolor na lazurowy, a na ekranie otworzy się menu (okno wyboru poleceń) z listą dostępnych komend dla tego obiektu.
3. Kliknij przycisk z poleceniem **ZZ** w rozwiniętym menu.
4. Ramka wokół obiektu zmieni kolor na zielony, a komenda pojawi się w wierszu poleceń. Kursor myszy automatycznie przeskoczy na przycisk "Wyślij".
5. Kliknij przycisk "Wyślij" celem zatwierdzenia. Po kliknięciu kursor powróci na swoją poprzednią pozycję nad obiektem.

### Metoda 2: Wpisywanie w Oknie Poleceń (z użyciem F12)
1. Wciśnij klawisz F12 na klawiaturze.
2. Kliknij LPM w puste tło na planie stacji (lub okręgu sterowania), aby otworzyć okno wpisywania poleceń (ComboBox).
3. Wpisz z klawiatury kod **ZZ**. W trakcie wpisywania lista dostępnych komend będzie się predyktywnie zawężać.
4. Zatwierdź wybór komendy klawiszem Enter.
5. Kliknij LPM na docelowy obiekt (blok Oz), którego ma dotyczyć komenda. Zostanie on otoczony zieloną ramką.
6. Kliknij przycisk "Wyślij" lub ponownie wciśnij klawisz Enter, aby sfinalizować wysłanie komendy do warstwy zależnościowej.

### Metoda 3: Skróty klawiaturowe (klawisze funkcyjne F1-F11)
Jeżeli polecenie jest przypisane do klawisza funkcyjnego:
1. Wciśnij dedykowany klawisz funkcyjny (np. F1-F11). Nazwa polecenia pojawi się w wierszu poleceń, uaktywni się przycisk "Wyślij".
2. Na ekranie na potencjalnych obiektach pojawi się lazurowa ramka (stan oczekiwania).
3. Po puszczeniu klawisza ramka na obiektach docelowych (blok Oz) zmieni się na zieloną.
4. Naciśnij Enter na klawiaturze (lub kliknij "Wyślij"), aby zatwierdzić operację.

### Anulowanie operacji
W każdym z powyższych przypadków, aż do momentu kliknięcia "Wyślij" (lub potwierdzenia Enterem), możesz przerwać wprowadzanie polecenia klikając LPM w puste (czarne) tło poza obiektami interaktywnymi. Spowoduje to wyczyszczenie wiersza poleceń, zgaszenie ramek i powrót systemu do stanu spoczynku.

---
