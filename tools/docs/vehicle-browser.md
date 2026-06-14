# Edytor taboru i składów

`vehicle-browser` jest aplikacją Qt6 przeznaczoną dla twórców danych. Umożliwia
przeglądanie typów pojazdów, tworzenie i edycję plików `vehicle.json` oraz
otwieranie, układanie i zapisywanie składów pociągów.

## Uruchomienie

Projekt należy zbudować z opcją `-DBUILD_TOOLS=ON`. W czasie działania muszą
być dostępne dynamiczne biblioteki Qt6 Widgets oraz plugin platformy Windows.

```powershell
vehicle-browser [--data-dir <katalog>] [--help]
```

Bez argumentu `--data-dir` aplikacja korzysta z katalogu `packages/` w
bieżącym katalogu roboczym. Argument `--data-dir` pozwala wskazać dowolny
katalog zawierający dane JSON, na przykład checkout repozytorium
`symulator-data`:

```powershell
vehicle-browser --data-dir C:\projekty\symulator-data\data
```

Źródło danych można zmienić bez ponownego uruchamiania aplikacji przez
**Plik > Otwórz katalog...**. Niepoprawne, niepowiązane i niedostępne pliki
JSON są pomijane. Polecenie **Plik > Odśwież dane** ponownie wczytuje aktywne
źródło.

## Przeglądanie danych

Lewy panel zawiera typy pojazdów. Pole **Filtruj typy pojazdów...** wyszukuje
tekst we wszystkich widocznych kolumnach. Nagłówki tabel umożliwiają
sortowanie.

Wybranie typu pojazdu ogranicza środkową tabelę do pojazdów, których
`type_uid` wskazuje na wybrany typ. Usunięcie zaznaczenia ponownie pokazuje
wszystkie pojazdy.

## Tworzenie pojazdu

1. Wybierz typ pojazdu w lewym panelu.
2. Kliknij **Nowy pojazd...**.
3. Wprowadź numer boczny.
4. Opcjonalnie podaj UID przewoźnika, numer inwentarzowy i uwagi.
5. Kliknij **Zapisz** i wybierz katalog docelowy.

Aplikacja proponuje UID rodzaju `VEHICLE` i sprawdza jego dostępność przed
zapisem. Jeśli UID jest zajęty, wybiera następną wolną instancję. Plik jest
zapisywany jako:

```text
<katalog>/<bezpieczny-numer-boczny>/vehicle.json
```

Wymagane pola JSON to `uid`, `type_uid`, `pID` i `displayName`. Pola
`carrierId`, `inventoryNumber` oraz `notes` są opcjonalne.

## Edycja istniejącego pojazdu

1. Zaznacz pojazd w środkowym panelu.
2. Kliknij **Edytuj pojazd...** albo kliknij dwukrotnie wybrany wiersz.
3. Zmień numer boczny, nazwę, UID przewoźnika, numer inwentarzowy lub uwagi.
4. Kliknij **Zapisz**.

Plik źródłowy zostaje nadpisany bez zmiany UID. Pola JSON nierozpoznawane
przez edytor są zachowywane. Po zapisie aktywne dane są odświeżane
automatycznie.

## Tworzenie i edycja składu

Lista na górze prawego panelu pozwala wybrać istniejący skład albo pozycję
**(Nowy skład)**. Po wybraniu istniejącego składu program wczytuje jego UID,
metadane, kolejność pojazdów i ścieżkę pliku.

1. Wybierz pojazd w środkowym panelu.
2. Kliknij **Dodaj do składu**.
3. Powtórz czynność dla kolejnych pojazdów.
4. Zmień kolejność metodą przeciągnij i upuść.
5. Użyj **Usuń pojazd**, aby usunąć zaznaczoną pozycję.
6. Uzupełnij numer pociągu, nazwę, kategorię i opcjonalny UID przewoźnika.
7. Kliknij **Zapisz**, aby nadpisać otwarty skład, albo **Zapisz jako...**, aby
   utworzyć nowy plik z nowym UID.

Nowy skład otrzymuje bezkolizyjny UID rodzaju `TRAIN_CONSIST`. Podczas edycji
istniejącego pliku jego UID pozostaje bez zmian.
Wynikowy JSON zawiera pola `uid`, `pID`, `displayName`, `trainCategory` oraz
tablicę `vehicle_uids`. Dodatkowe pola istniejącego JSON-a są zachowywane.

## Legenda UID

Polecenie **Pomoc > Legenda UID** otwiera tabelę domen, rodzajów i znaczenia
pola ZAKRES. Legenda jest dostępna niezależnie od tego, czy dane zostały
poprawnie wczytane.

## Współpraca z symulator-data

Repozytorium `symulator` zawiera aplikację, natomiast `symulator-data`
przechowuje dane JSON. Przeglądarka może:

- czytać wydane paczki pobrane do `symulator/packages/`,
- czytać bezpośrednio katalog lokalnego repozytorium `symulator-data`,
- tworzyć i edytować pliki pojazdów i składów zgodne ze schematami danych.

Program nie wykonuje operacji Git ani automatycznej synchronizacji. Nowe pliki
należy zapisać w odpowiednim katalogu `symulator-data`, sprawdzić i zatwierdzić
standardowym procesem Git.

## Najczęstsze problemy

**Brak pluginu platformy Windows**

Jeśli Qt zgłasza brak pluginu `windows`, obok pliku wykonywalnego musi istnieć:

```text
platforms/qwindows.dll
```

Dla wersji Debug wymagany jest odpowiednio `platforms/qwindowsd.dll`.

**Brak danych po uruchomieniu**

Uruchom aplikację z katalogu głównego `symulator`, w którym znajduje się
`packages/`, albo wskaż dane przez `--data-dir`.

**Skład zawiera brakujące pojazdy**

Edytor pokazuje ostrzeżenie, jeśli `vehicle_uids` wskazuje rekord, którego nie
ma w aktywnym źródle. Wczytaj kompletne dane albo popraw odwołanie przed
zapisem.
