# RBC / TSR - Specyfikacja UI terminala EbiScreen 300 (Ebilock STR-1)

Niniejszy dokument stanowi wyczerpującą i rygorystycznie szczegółową specyfikację interfejsu użytkownika (UI) oraz mechaniki działania terminala klienckiego STS-2 / EbiScreen 300 (system STR-1) dla modułów **RBC (Centrum Sterowania Radiowego)** oraz **TSR (Tymczasowe Ograniczenia Prędkości)**, zaimplementowanego natywnie w języku C++ z wykorzystaniem pętli zdarzeń, wskaźników obiektowych oraz mechaniki sygnałów i slotów. Rozkłada on każde najdrobniejsze kliknięcie i interakcję na czynniki pierwsze.

> [!IMPORTANT]
> Specyfikacja ta opisuje nie tylko widok wizualny, ale także "mechanikę kliknięć", definiując najdrobniejsze i trywialne interakcje na poziomie callbacków wywoływanych przez warstwę widoku z systemu.

---

## 1. Wygląd ogólny, Logowanie i Stan Połączeń

### 1.1 Logowanie
* **Okno Logowania (Zintegrowane z STS-2):** Obiekt inicjalizowany w kodzie przy starcie systemu i wyśrodkowany geometrycznie.
  * **Pole tekstowe `[Użytkownik]`**: Kliknięcie lewym przyciskiem myszy (LPM) powoduje zogniskowanie zdarzeń klawiatury (focus na obiekcie okna). Użytkownik wpisuje identyfikator.
  * **Pole tekstowe `[Hasło]`**: Kliknięcie LPM nadaje focus na to pole. Eventy klawiszy są przechwytywane a na widok nakładane znaki maskujące (kropki).
  * **Przycisk `[Anuluj]`**: Kliknięcie LPM emituje slot, w którym wywoływany jest destruktor zwalniający ten obiekt w pamięci, co powraca do okna macierzystego powitalnego.
  * **Przycisk `[OK]`**: Wywołuje sygnał przesyłania struktury uwierzytelniającej po sieci, a po odbiorze ack ukrywa (usuwa ze sterty renderera) pole wybudzając panel sterowania głównego.
  * **Przycisk `[X]` (Prawy górny róg okna)**: Akcja jest aliasowana do procedury podpiętej pod `[Anuluj]`.

### 1.2 Okno Autoryzacji
Tabela w formie wektora struktur do przejmowania władzy nad stacjami.
* **Kliknięcie LPM w pole wyboru (Widżet Checkbox) przy stacji**: Ustawia odpowiednią flagę boolowską (oznaczającą żądanie przejęcia) we właściwościach komponentu okna, po czym mechanizm rysuje kontrolkę ze znacznikiem.
* **Przycisk `[Akceptuj]` / `[Przejmij]` (na dole okna)**: Callback generuje instrukcje z zaznaczonymi wskaźnikami stacji i umieszcza telegram w kolejce wysyłkowej.

---

## 2. Okno Zarządzania Pociągami (RGH / RBC-Pociągi)

### 2.1 Główna Tabela Pociągów i Interakcje
Element graficzny rysujący posortowane modele z list wskaźnikowych pociągów.
* **Kliknięcie LPM na wybrany wiersz pociągu**: Pętla graficzna odbiera adres wybranego modelu logicznego pociągu. Modyfikuje flagę wyrysowania podświetlenia u klikniętego i aktywuje podpięte z nim panele informacyjne (zapełnia detale za pomocą wskaźnika u dołu interfejsu).
* **Dwukrotne kliknięcie LPM (Double-click) na wiersz**: Zdarzenie to podbiera zmienne pozycjonowania z owego obiektu i przesuwa kamery map układu logicznego z centrowaniem na X/Y tego pociągu.

### 2.2 Zakładki Szczegółowe (Dolny panel)
Powołane wraz z sygnałem podświetlenia, podokna wyciągające ze wskazanego obiektu wartości. Przełączanie odbywa się poprzez ukrywanie nieaktywnych instancji form po kliknięciu LPM na nazwie:
* **Zakładka `[Dane statyczne]`**: Pola wyrenderowane w rzutowaniu Read-Only na poziomie pętli zdarzeń klawisza. (np. brak w kodzie obsługi onKeyPressed w nich).
* **Zakładka `[Dane dynamiczne]`**: Widżety renderujące aktualne parametry z zablokowaną modyfikacją.
* **Zakładka `[Wiadomości]`**: Log przechwytujący przychodzący bufor dla danego id pociągu.
  * **Przycisk `[Nowa wiadomość]`**: LPM przesyła sygnał alokacji podrzędnego okna typu modalnego "Nowa wiadomość" (połączonego wskaźnikiem na pociąg).

#### Mechanika Okna "Nowa wiadomość" (Tryb STR-1)
Po otwarciu okna z użyciem mechaniki dynamicznej alokacji widoku:
* **Przycisk `[X]` (prawy górny róg)**: Slot przechwytuje event usuwania okna (`delete this`), przy czym bufor edytowany jest skracany lub całkowicie uwalwamy u garbagera okiennego - bezpowrotnie traci wprowadzany tekst.
* **Pole tekstowe**: Widżet uaktywniający wprowadzanie, przechwytujący kody znaków u klawiatury; jeśli odbierany kod wypada za przedział ASCII limitowany, zostaje odrzucony przez weryfikatora znaków (event filter).
* **Element Checkbox `[Wymaga potwierdzenia]`**: Kliknięcie ustawia lokalną strukturę zmienną decyzyjną odpowiedzialną za parametr acknowledge. 
* **Przycisk `[Wybierz z listy]`**: Uruchamia metodę pokazania dynamicznej listy widżetów ComboBox (rozwijaną). Wybór poprzez kliknięcie ładuje do wskaźnika buforu pozycję i sygnał zamyka listę.
* **Przycisk `[Zamknij]`**: Podłączone identycznie jak akcja ze zdarzeniem systemowego zamknięcia `[X]`.
* **Przycisk `[Wyślij]`**: Pobiera wskaźnik do obecnego pociągu powiązanego z klasą okna i przesyła tam wygenerowany datagram, po zakończeniu generując event destrukcyjny na oknie, czym chowa UI.
* **Przycisk `[Wyślij do wszystkich]`**: Emituje pętlę iteracyjną (po całej liście wszystkich zalogowanych pociągów) transmitując broadcast tekstowy i wywołuje na obiekcie okna kasację destruktorem.

### 2.3 Przyciski dolnego paska w oknie zarządzania pociągami
Naciśnięcie każdego uderza z zapytaniem przez callback przywiązany wskaźnikiem do wybranego, aktualnie podświetlonego (aktywnego) na liście pociągu:
* **Przycisk `[Pobierz pozycję]`**: Powoduje wysłanie polecenia do telegram-buildera w RBC odpytującego pozycję GPS dla tego wskaźnika ID pociągu. Modyfikuje na ekranie label "Status pozycji" na string "Pobieranie...".
* **Przycisk `[Zatrzymanie pociągu]`**: 
  * Kliknięcie wymusza inicjację i rendering nowo utworzonego obiektu małego okienka o nazwie "Awaryjne zatrzymanie".
  * W instancji Pop-up: Callback od `[X]` - zwolnienie zasobu z pamięci ram okienka ostrzeżenia. Callback od `[Nie]` - usunięcie z ram i destrukcja, tak jak `[X]`. Callback od `[Tak]` - umieszczenie ramki sieciowej Stop w kolejce oraz zaaplikowanie destruktora własnego i zwolnienia z grafiki.
* **Przycisk `[Wyrejestruj]`**: 
  * Tworzy kontrolkę okienka klasy z napisem w labelu "Czy usunąć z RBC?".
  * Callbacki okna `[Tak]` puszczają algorytmiczne sygnały usuwające węzły z pamięci powiązanych modułów (wyrejestrowanie). `[Nie]` lub `[X]` generują `delete this` na potwierdzeniu i powrót.
* **Przycisk `[Anuluj / Zamknij]`**: Wyzerowuje główny wskaźnik w widoku powiązany na element tabeli - deaktywuje styl i puszcza pętle ukrycia okien paneli szczegółowych.

---

## 3. Okno Zarządzania TSR (Tymczasowe Ograniczenia Prędkości)

Interfejs zdefiniowany dla TSR na panelu EbiScreen 300 to zestaw precyzyjnych narzędzi opartych na sekwencyjnym wywoływaniu eventów z pętli.

### 3.1 Górny panel - Definicja Ograniczenia
* **Przycisk `[Nowy TSR]`**: Slot przypisany do kontrolki aktywuje formę "Dolny panel detali (Obszar TSR)" zlecając wyrysowanie oraz budzi nowy obiekt czasomierza (`QTimer` lub podobny). Odpala callback odliczania wstecz od 120 sek. na odrębnym etykiecie. Jeżeli sygnał `onTimeout` osiągnie limit 120s bez zwieńczenia pracy w formach przez użytkownika, mechanika symuluje wyzwolenie funkcji zamknięcia procedury zwalniając utworzone pod TSR kontenery i ukrywając panel.
* **Dolny panel - Wprowadzanie wartości (W trybie Nowy TSR):**
  * **Przycisk `[Obiekt początkowy]`**: Kliknięcie nadaje widżetowi przycisku flagę wizualnie go zaciskającą w renderowaniu. W tym momencie handler myszy dla okna układu głównego przechwytuje adres pod kliknięciem (LPM). Po uzyskaniu semafora adres jest pakowany na label z tagiem np. `A (Nd)` a przycisk dostaje komendę podniesienia.
  * **Pole `[Korekta]` przy obiekcie**: Zmiana focus klawiatury by zapisać znaki numeryczne w polu.
  * **Przycisk `[Obiekt końcowy]`**: Zasada callbacku pokrywa się ze skanowaniem myszy jak przy "początkowym", rejestrując obiekt w powiązanym endpoincie.
  * **Przycisk `[Obiekt pośredni]`**: Tryb "wduszony" ładuje obiekty wyklikane LPM do wektora STL przypisanego pod proces kreacji limitów obwodowych TSR.
  * **Pola rozwijane `[Kierunek]` i `[Prędkość]`**: Komponenty okienne Dropdown / ComboBox podlegające rozwijaniu menu. Wyłonienie najechanego parametru kursorem po kliknięciu LPM skutkuje sygnałem przypisującym stałą wartość pod proces kreatora oraz zwijającym kontrolkę na ekranie.
* **Przycisk `[Zdefiniuj TSR]` / `[Zapisz TSR]`**: Przejmuje uzupełnione dane tymczasowych buforów na rzecz stworzenia stałego obiektu klasy TSR (wrzucanego do wektora "Tabela Główna") inicjowanego parametrem stanu wyłączonego (rysowanym czarną czcionką) - wg Rysunek 31.

### 3.2 Tabela Główna TSR i Aktywacja
Gdy rekord struktury wektorowej TSR przebywa na liście pamięci:
* **Pojedyncze kliknięcie LPM na wiersz w tabeli TSR**: Mapuje dany element na główny wskaźnik fokusowy. Sygnał od interfejsu powiadamia podsystem mapujący - grafika na ekranie wyrysowuje poligon wektorowy barwy pomarańczowej wokół skorelowanej ścieżki i powiązanych punktów na mapie. Callback jednocześnie ożywia wybrane widgety akcji u dołu paneli.
* **Przycisk `[Aktywuj TSR]`**: 
  * LPM przepycha odpowiednie bufory danych nakazując wygenerowanie i rozesłanie telegramu sterującego do sprzętowej warstwy zewnętrznej balis/RBC.
  * Kolor wskaźnika komórki tabelarycznej dla zaznaczonego statusu TSR podmieniany jest przez widok na Zielony.
* **Przycisk `[Dezaktywuj TSR]`**: Wysyłanie callbacku dezaktywacji telegramu. Odrysowuje rzutowany obiekt tabeli ze statusem deaktywacji w kolorze czarnym.
* **Przycisk `[Usuń TSR]`**: 
  * Wbudowane zabezpieczenie w pętli obsługi okien, ujawnia przycisk dodatkowy w celu sprawdzenia dwustopniowego przed procesem usunięcia obiektu i pamięci TSR.
* **Przycisk `[Potwierdź: Usuń TSR]`**: Kliknięcie LPM poddaje TSR usunięciu (stan przekazany do uśmiercenia) - warstwa widoku tabeli markuje wiersz na czerwony (Rys. 31), następnie zasób traci powiązanie macierzyste - dany TSR jest uwalniany ze wskaźników listowych wektora a pętla UI go już nie rysuje.
* **Przycisk `[Zamknij]` (prawy dolny róg panelu)**: Zeruje referencje na TSR w edycji, zdejmuje focus myszkowy, chowa formatkę całego panelu na ekranie, by w głównym obwodzie zdarzeń powrócić znów do trybu przeglądania stacyjnego (renderingu map).
