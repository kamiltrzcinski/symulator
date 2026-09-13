# Procedury UI i Baza Implementacji - Thales ESTW L90 5 (ML8)

Ten dokument stanowi szczegółową bazę ("łopatologiczną") do implementacji mechaniki interfejsu (UI) dla systemu Thales Command 900. Opisuje krok po kroku zjawiska zachodzące na ekranie podczas obsługi systemu przez operatora.

## 1. Architektura interakcji myszą (Pola poleceń)

W przeciwieństwie do EbiLocka (gdzie używaliśmy "Drag & Drop"), system Thalesa opiera się na **sekwencyjnym zaznaczaniu elementów** i używaniu oddzielnej strefy "Pól Poleceń" oraz paska "Linii wejściowej".

### Wybór elementu (np. semafora)
1. **Zdarzenie:** `mousePressEvent` (LPM) na obszarze aktywnym elementu na lupie stacyjnej.
2. **Akcja UI:** 
   - W linii danych wejściowych (na górze ekranu) pojawia się kod stacji i nazwa elementu (np. `POA, S1,`).
   - W lewej górnej części monitora w strefie "Pól poleceń" podświetlają się na niebiesko przyciski z poleceniami dostępnymi dla tego elementu (np. `POC`, `MAN`, `SZ`).
   - Jeśli to polecenie wymaga podania elementu docelowego, podświetlone zostają możliwe do wyboru elementy na planszy (np. semafory końcowe otoczone żółtymi okręgami).

## 2. Krok po kroku: Nastawianie przebiegu pociągowego (POC)

Aby zaprogramować w C++ nastawienie przebiegu dla Thalesa, musisz obsłużyć następującą pętlę zdarzeń:

**KROK 1: Wybór początku przebiegu**
- **Działanie:** Operator klika (LPM) na semafor początkowy.
- **Reakcja Systemu:** 
  - Wokół semafora początkowego pojawia się **wypełnione żółte koło** (oznaczenie początku przebiegu).
  - Wokół potencjalnych semaforów docelowych pojawiają się **puste żółte okręgi**.
  - Do wewnętrznego bufora (linii wejściowej) trafia np. `POA, A,`.

**KROK 2: Wybór elementu docelowego**
- **Działanie:** Operator klika (LPM) na jeden z podświetlonych (pustym żółtym okręgiem) semaforów docelowych.
- **Reakcja Systemu:** 
  - Żółty okrąg na elemencie docelowym zostaje **wypełniony kolorem żółtym**.
  - Wokół pierwszego semafora koło pozostaje żółte.
  - Do linii wejściowej dopisywany jest cel: `POA, A, S2,`.

**KROK 3: Wybór rodzaju przebiegu (Przyciski / Klawiatura)**
- **Działanie:** Operator klika LPM w niebieski przycisk `POC` (Przebieg Pociągowy) w lewym górnym rogu ekranu, ALBO wpisuje `POC` z klawiatury.
- **Reakcja Systemu:** 
  - W linii danych wejściowych widnieje pełna składnia: `POA, A, S2, POC`.
  - Przyszła droga przebiegu na torach NIE JEST JESZCZE rysowana ani zielona.

**KROK 4: Zatwierdzenie (Przycisk P / Klawisz Enter)**
- **Działanie:** Operator klika LPM na przycisku zatwierdzenia `P` (znajdującym się na ekranie) LUB wciska fizyczny klawisz `ENTER`.
- **Reakcja Systemu:**
  - `Model` wysyła komendę do modułu zależnościowego (IM).
  - Pomyślne odebranie komendy czyści linię wejściową "WE" i wyświetla komunikat w linii "KOM": `Polecenie POC zaakceptowane`.
  - Na obrazie stacji droga jazdy od semafora A do S2 **zmienia kolor z szarego na ZIELONY** (utwierdzony przebieg pociągowy). Elementy drogi ochronnej za semaforem S2 zapalają się na **ŻÓŁTO**. Semafor A zmienia sygnał na zezwalający (zielony trójkąt).

## 3. Krok po kroku: Obsługa poleceń specjalnych (Poziom 1 - SPEC)

Polecenia niebezpieczne (np. doraźne rozwiązanie przebiegu `ZW`, czy ręczne przestawienie zajętej zwrotnicy `DPZ`) wymagają specjalnego autoryzowania. W C++ będzie to obsługiwane przez stan oczekiwania (Timer 20-30 sekund).

**KROK 1: Inicjalizacja komendy**
- Operator wyklikuje komendę (np. klika na zwrotnicę, klika polecenie `DPZ` i naciska `P`).
- **Reakcja UI:** Element na stacji zaczyna podświetlać się na **pomarańczowo**. System wypisuje tekst kontrolny na ekranie informujący o niebezpieczeństwie akcji.

**KROK 2: Przygotowanie Timer'a**
- W tle uruchamia się `QTimer` odliczający 25 sekund. Zmienna stanu przechodzi w `WAITING_FOR_SPEC`.
- Przycisk `SPEC` na monitorze podświetla się na CZERWONO, domagając się kliknięcia.

**KROK 3: Zatwierdzenie autoryzacji**
- **Działanie:** Operator w ciągu 25 sekund musi kliknąć myszą w czerwony przycisk `SPEC` LUB wcisnąć skrót klawiszowy `CTRL + A`.
- **Reakcja Systemu:**
  - Wywołany zostaje Slot obsługujący autoryzację.
  - Pomarańczowe podświetlenie znika. 
  - Komenda (np. `DPZ`) zostaje faktycznie wysłana do wykonania przez moduł logiczny. System przerysowuje element.
- **Brak Działania (Timeout):** Jeśli `QTimer` minie 25 sekund, system cicho anuluje polecenie, gasi czerwony przycisk `SPEC` i przywraca stan domyślny, wypisując `Odrzucono`.

## 4. Baza do wdrożenia na jutro (Implementacja Modeli)

Z punktu widzenia C++ (Qt), do jutrzejszej sesji implementacyjnej będziemy używać następującej konwencji, którą oprogramujemy:
1. `InputBuffer` (String) - globalna zmienna na oknie Thalesa przechowująca aktualnie wklepywaną komendę (np. `"POA, S1, POC"`).
2. Zdarzenia z `QGraphicsItem` na kliknięcie myszą będą jedynie doklejać odpowiednie identyfikatory obiektów (np. `, S1`) do `InputBuffer`.
3. Wywołanie `Enter` na klawiaturze wywoła metodę `ParseCommand(InputBuffer)`, która przekaże zlecenie do silnika gry.
4. Rysowanie trasy będzie polegało na iteracji przez graf grafu torowego z podmienionym `QPen` (kolor zielony dla `POC`, żółty dla `MAN` i drogi ochronnej).
