# Procedury wykonania poleceń - Thales ESTW L90 5

Dokument zawiera wyczerpujący, "łopatologiczny" i krokowy opis wykonania każdej komendy dla systemu Thales ESTW L90 5 w silniku UI (na podstawie "Pól Poleceń", linii "WE", zatwierdzania 'P' i autoryzacji 'SPEC').

## Zasady Ogólne
Każda procedura składa się zasadniczo z sekwencji:
1. **Wskazanie elementu (LPM)** na planszy (selekcja - żółte obwódki).
2. **Wybór komendy** (LPM) ze strefy "Pól Poleceń" (niebieskie przyciski).
3. **Zatwierdzenie P** (Przetwarzaj) (lub ENTER).
4. *(Dla poleceń poziomu 1 - krytycznych)* **Odczekanie / Autoryzacja SPEC** (lub CTRL+A) w czasie 20-25 sekund, w tym czasie obiekt podświetla się na pomarańczowo.

---

## 1. Polecenia obsługi przebiegów (POC, MAN, ZDP, ZDM, ZCZ, ZW)

### POC – Nastawienie przebiegu pociągowego (Poziom 0)
1. **LPM na semafor początkowy** `<PPOC>`. Wokół niego pojawia się wypełnione żółte koło. Na pozostałych dopuszczalnych celach pojawiają się puste żółte okręgi. Bufor linii wejściowej ("WE") dodaje np. `POA, A,`.
2. **LPM na semafor docelowy** `<KPOC>` (lub inny dopuszczalny koniec przebiegu podświetlony pustym okręgiem). Pusty okrąg wypełnia się żółtym kolorem. Do linii "WE" dopisuje się cel np. `POA, A, S2,`.
3. **LPM na przycisk `POC`** w strefie "Pól Poleceń". Do linii dopisuje się `, POC`.
4. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* W przypadku wolnej drogi, system ustawia i utwierdza przebieg. Linia "WE" się czyści. Droga jazdy na planszy podświetla się na **zielono**, a droga ochronna za celem na **żółto**.

### MAN – Nastawienie przebiegu manewrowego (Poziom 0)
1. **LPM na sygnalizator początkowy** `<PMAN>` (semafor lub tarcza manewrowa). Pojawia się żółte koło, cele podświetlają się pustymi okręgami.
2. **LPM na sygnalizator docelowy** `<KMAN>`. Okrąg się wypełnia, bufor aktualizuje się o cel.
3. **LPM na przycisk `MAN`** w strefie poleceń.
4. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Droga jazdy dla przebiegu manewrowego zmienia kolor na **żółty**. Zostaje wyświetlony sygnał zezwalający na manewr.

### ZDP – Zwolnienie przebiegu pociągowego (Poziom 0)
*Używane, gdy strefa zbliżania przed semaforem jest WOLNA, a my chcemy anulować ułożony przebieg.*
1. **LPM na semafor początkowy** `<PPOC>` dla ułożonego przebiegu. W linii "WE" pojawia się identyfikator sygnalizatora.
2. **LPM na przycisk `ZDP`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Semafor natychmiast wraca na "Stój", przebieg zostaje rozwiązany bezzwłocznie, droga jazda gaśnie (wraca do koloru szarego).

### ZDM – Zwolnienie przebiegu manewrowego (Poziom 0)
1. **LPM na sygnalizator początkowy** `<PMAN>` ułożonego przebiegu manewrowego.
2. **LPM na przycisk `ZDM`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Tarcza manewrowa wraca na sygnał "Stój", droga manewrowa jest zwalniana bezzwłocznie i wraca do wyświetlania na szaro.

### ZCZ – Zwolnienie czasowe przebiegu pociągowego (Poziom 0)
*Używane, gdy odcinek zbliżania jest ZAJĘTY. Operacja ta inicjuje odliczanie bezpiecznego czasu.*
1. **LPM na semafor początkowy** `<PPOC>`.
2. **LPM na przycisk `ZCZ`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Semafor rzuca sygnał "Stój". Tory na drodze jazdy podświetlają się na **Magenta** (kolor zwalniania z opóźnieniem) i rozpoczyna się odliczanie czasu (np. 120 sekund). Po jego upłynięciu elementy przebiegu zwalniają się i wracają do koloru szarego.

### ZW – Doraźne zwolnienie części przebiegu (Poziom 1)
*Rozwiązanie awaryjne rozluźniające utwierdzenie przebiegu bez zwłoki czasowej. Bardzo niebezpieczne (Poziom 1).*
1. **LPM na element (zazwyczaj koniec przebiegu: `<KPOC>` lub `<KMAN>`)** reprezentujący utwierdzoną drogę.
2. **LPM na przycisk `ZW`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - Element / cel zaczyna podświetlać się na **pomarańczowo**. System przechodzi w tryb autoryzacji (zegar 20-25s).
4. **LPM na przycisk `SPEC`** (lub CTRL+A) w strefie poleceń w trakcie odliczania.
   - *Efekt:* Część przebiegu zostaje bezzwłocznie zwolniona i wraca do stanu szarego (podświetlenie pomarańczowe gaśnie).

---

## 2. Polecenia sterowania sygnalizatorami (STÓJ, SZ)

### STÓJ – Wygaszenie sygnału zezwalającego (Poziom 0)
1. **LPM na sygnalizator** `<S>` wyświetlający sygnał zezwalający.
2. **LPM na przycisk `STÓJ`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Semafor lub tarcza natychmiast wraca do stanu wskazującego sygnał "Stój" (kolor czerwony / wygaszony w zależności od typu elementu). Utwierdzenie przebiegu nadal pozostaje (trzeba je zwolnić komendą ZDP/ZCZ).

### SZ – Sygnał Zastępczy (Poziom 1)
1. **LPM na semafor** `<S>`.
2. **LPM na przycisk `SZ`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER). 
   - Semafor podświetla się na **pomarańczowo**. Rusza licznik czasowy (20-25s) oczekujący na autoryzację.
4. **LPM na przycisk `SPEC`** (lub CTRL+A) w czasie odliczania.
   - *Efekt:* Na semaforze załącza się sygnał zastępczy (migające białe).

---

## 3. Polecenia elementów torowych i zwrotnic (PZ, DPZ, ZEROLO, KSR)

### PZ – Przestawienie zwrotnicy / wykolejnicy (Poziom 0)
*Używane do normalnego przekładania zwrotnic, kiedy sekcja na której się znajdują jest fizycznie wolna.*
1. **LPM na zwrotnicę lub wykolejnicę** `<Z> / <WK>`. W linii wejściowej "WE" pojawia się identyfikator (np. `Z3,`).
2. **LPM na przycisk `PZ`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Zwrotnica przestawia się (jeżeli warunki na to pozwalają). Grafika zwrotnicy zmienia ułożenie, a po potwierdzeniu z gruntu (krańcówki) utwierdza swą pozycję w systemie.

### DPZ – Doraźne przestawienie zwrotnicy (Poziom 1)
*Przestawienie wymuszone ignorujące błąd/zajętość izolacji w strefie zwrotnicy.*
1. **LPM na zwrotnicę / wykolejnicę** `<Z> / <WK>`.
2. **LPM na przycisk `DPZ`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - Zwrotnica podświetla się na **pomarańczowo**. Rusza 20-25s czas na potwierdzenie.
4. **LPM na przycisk `SPEC`** (lub CTRL+A) zanim upłynie czas.
   - *Efekt:* Silnik zostaje awaryjnie wysterowany w nowe położenie pomimo błędów/zajętości izolacji. Pomarańczowe podświetlenie gaśnie.

### ZEROLO – Zerowanie licznika osi (Poziom 1)
*Używane do przywrócenia odcinka po awarii lub "zgubieniu" osi przez system.*
1. **LPM na sekcję (tor, zwrotnicę)** `<T> / <Z> / <WK> / <SK>` wykazującą błędną zajętość.
2. **LPM na przycisk `ZEROLO`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - Odcinek toru podświetla się na **pomarańczowo** i system prosi o autoryzację poziom 1.
4. **LPM na przycisk `SPEC`** (lub CTRL+A) w wymaganym czasie (20-25s).
   - *Efekt:* Kontrola niezajętości toru zostaje wyzerowana (wymuszony stan "wolny"). Podświetlenie pomarańczowe i ewentualne komunikaty awaryjne (np. czerwono-białe migające) znikają; tor przybiera kolor szary.

### KSR – Kasowanie sygnalizacji rozprucia zwrotnicy (Poziom 1)
1. **LPM na rozpruwaną zwrotnicę / wykolejnicę** `<Z> / <WK>`.
2. **LPM na przycisk `KSR`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - Zwrotnica podświetla się na **pomarańczowo**, startuje licznik czasu na autoryzację z powodu wagi polecenia (Poziom 1).
4. **LPM na przycisk `SPEC`** (lub CTRL+A).
   - *Efekt:* Alarm/sygnalizacja rozprucia (np. czerwono-białe miganie) dla danej zwrotnicy zostaje skasowana z systemu.

---

## 4. Polecenia Zamknięć Indywidualnych (STOP, OSTOP / OUZ)

### STOP – Zamknięcie indywidualne / Zastopowanie (Poziom 0)
1. **LPM na element do zamknięcia** `<Z> / <SK> / <WK> / <S> / <B>` (np. tor, zwrotnica, semafor).
2. **LPM na przycisk `STOP`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Element zostaje oznaczony jako zamknięty/zablokowany dla systemu zależnościowego. Na schemacie zazwyczaj pojawia się kapturek lub specjalne oznaczenie zablokowania. Jeśli to semafor – blokowany jest na "Stój".

### OSTOP (OUZ) – Odwołanie zamknięcia indywidualnego (Poziom 0)
1. **LPM na zablokowany element** `<Z> / <SK> / <WK> / <S> / <B>`.
2. **LPM na przycisk `OSTOP`** (często utożsamiane z OUZ w interfejsie Thalesa) w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Z elementu zostaje zdjęte zamknięcie systemowe; powraca on do normalnej dyspozycyjności operacyjnej.

---

## 5. Obsługa Blokady Liniowej (WBL, OWBL, ZWBL, PZK)
*Operacje te wykonuje się zwykle na piktogramach/elementach bloku liniowego na granicy stacji `<B>`.*

### WBL – Włączenie blokady liniowej (Żądanie pozwolenia) (Poziom 0)
1. **LPM na element blokady** `<B>` wybranego szlaku.
2. **LPM na przycisk `WBL`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Wysłane zostaje żądanie pozwolenia (np. zapalają się odpowiednie lampki strzałki blokady). Oczekuje na odpowiedź PZK od sąsiedniego posterunku.

### OWBL – Odwołanie żądania pozwolenia (Poziom 0)
1. **LPM na element blokady** `<B>`, dla którego wcześniej użyto WBL.
2. **LPM na przycisk `OWBL`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Żądanie zmiany kierunku jest anulowane.

### ZWBL – Zwolnienie blokady liniowej (Poziom 0)
1. **LPM na element blokady** `<B>`.
2. **LPM na przycisk `ZWBL`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Końcowe zwolnienie szlaku po wjeździe pociągu (jeżeli system nie odnotował przejazdu przez automatyczne czujniki).

### PZK – Danie pozwolenia (Poziom 0)
1. **LPM na element blokady** `<B>` (gdy sąsiad żąda poprzez WBL).
2. **LPM na przycisk `PZK`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Następuje zgoda (oddanie) pozwolenia w danym kierunku. Strzałka kierunku szlaku zmienia się i blokuje na odprawienie pociągu przez sąsiada.

---

## 6. Pozostałe polecenia pomocnicze systemu (LKA, PZB, POT)

### LKA – Kasowanie linii poleceń
1. **LPM na przycisk `LKA`** w strefie poleceń (lub naciśnięcie klawisza ESC w trakcie wybierania).
   - *Efekt:* Natychmiastowo, bez konieczności naciskania 'P', czyści zawartość paska "WE" na górze ekranu i anuluje wybór elementów (gasną żółte okręgi i obwódki).

### PZB – Potwierdzenie uszkodzenia / błędu (Poziom 0)
1. Jeżeli obiekt na planszy miga np. na czerwono sygnalizując usterkę.
2. **LPM na przycisk `PZB`** na klawiaturze poleceń (lub z uprzednio zaznaczonym elementem uszkodzonym, w zależności od precyzyjnej implementacji Thalesa).
3. **LPM na przycisk `P`** (lub ENTER).
   - *Efekt:* Usterka zostaje skwitowana i ikona przechodzi z trybu pulsującego (niepotwierdzonego) w stałe światło awaryjne.

### POT – Przywrócenie elementu po usterce (Poziom 1)
1. **LPM na uszkodzony element**, który technicznie już odzyskał sprawność.
2. **LPM na przycisk `POT`** w strefie poleceń.
3. **LPM na przycisk `P`** (lub ENTER).
   - Element podświetla się na **pomarańczowo** – czeka 20-25s.
4. **LPM na przycisk `SPEC`** (lub CTRL+A).
   - *Efekt:* Awaryjny tryb elementu zostaje usunięty, obiekt wraca do normalnego kolorowania stanu zasadniczego.

---

## Ważne uwagi do interfejsu
- W przypadku poleceń wieloelementowych (jak **POC** czy **MAN**), zawsze najpierw klika się początek, potem cel (lub cele pośrednie, jeśli dopuszczone), potem komendę (np. POC), a na końcu przycisk 'P'.
- Polecenia **Poziomu 1** (SPEC) bezwzględnie wymagają asynchronicznego oczekiwania z 20-25 sekundowym licznikiem i dedykowanego podświetlenia (kolor pomarańczowy). Jeżeli użytkownik nie zdąży wcisnąć SPEC w tym czasie (timeout), system cofa operację do stanu sprzed wciśnięcia 'P' z błędem zaniechania.
- Kliknięcie **PPM** w puste pole (tło) lub naciśnięcie **ESC** zawsze pełni funkcję globalnego `LKA`, przerywając i czyszcząc aktualnie zapisaną sekwencję.
