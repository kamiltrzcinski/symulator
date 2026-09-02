# Komendy i Przebiegi (Thales ESTW L90 5)

Dokument opisuje logikê poleceñ oraz zarz¹dzania przebiegami dla systemu Thales ESTW L90 5, opracowany z myœl¹ o implementacji w C++.

## 1. Poziomy Poleceñ i Wykonywanie
Ka¿de polecenie systemowe ma okreœlony poziom:
- **Poziom 0 (Zwyk³e)** - wykonywane od razu po zatwierdzeniu poleceniem P (lub klawiszem Enter).
- **Poziom 1 (Specjalne)** - wymagaj¹ dodatkowego potwierdzenia poleceniem SPEC (lub Ctrl+A) w ci¹gu ok. 25 sekund po wciœniêciu P. Elementy na ekranie podœwietlaj¹ siê na pomarañczowo.

## 2. S³ownik Komend

| Komenda | Poziom | Parametry | Opis dzia³ania |
|---------|--------|-----------|----------------|
| **LKA** | 0 | brak | Kasowanie linii poleceñ. |
| **P** | 0 | brak | Przetwarzanie (wys³anie polecenia do weryfikacji). |
| **SPEC** | 1 | brak | Potwierdzenie obs³ugi polecenia specjalnego poziomu 1. |
| **PZB** | 0 | 0 | Potwierdzenie uszkodzenia lub b³êdu (gdy ikona miga na czerwono). |
| **POT** | 1 | brak | Przywrócenie elementu po usterce. |
| **ZEROLO**| 1 | <T> / <Z> / <WK> / <SK> | Zerowanie licznika osi dla sekcji kontroli niezajêtoœci (toru, zwrotnicy). |
| **PZ** | 0 | <Z> / <WK> | Przestawienie zwrotnicy lub wykolejnicy (kiedy sekcja jest wolna). |
| **DPZ** | 1 | <Z> / <WK> | DoraŸne przestawienie zwrotnicy/wykolejnicy (gdy sekcja wykazuje zajêtoœæ/usterkê). |
| **KSR** | 1 | <Z> / <WK> | Kasowanie sygnalizacji rozprucia zwrotnicy/wykolejnicy. |
| **STOP** | 0 | <Z> / <SK> / <WK> / <S> / <B> | Zamkniêcie indywidualne (zastopowanie) elementu. Semafor wraca na "Stój". |
| **OSTOP** | 0 | <Z> / <SK> / <WK> / <S> / <B> | Odwo³anie zamkniêcia indywidualnego. |
| **POC** | 0 | <PPOC>, <KPOC>, ... | Nastawienie przebiegu poci¹gowego. |
| **MAN** | 0 | <PMAN>, <KMAN>, ... | Nastawienie przebiegu manewrowego. |
| **ZDP** | 0 | <PPOC> | Zwolnienie przebiegu poci¹gowego (bez zw³oki, odcinek zbli¿ania niezajêty). |
| **ZDM** | 0 | <PMAN> | Zwolnienie przebiegu manewrowego. |
| **ZCZ** | 0 | <PPOC> | Zwolnienie czasowe przebiegu poci¹gowego (odcinek zbli¿ania zajêty - inicjuje odliczanie opóŸnienia, np. 120s). |
| **ZW** | 1 | <KPOC> / <KMAN> | DoraŸne zwolnienie czêœci przebiegu (zwalnia wszystko bez zw³oki czasowej). |
| **SZ** | 1 | <S> | Wyœwietlenie sygna³u zastêpczego (migaj¹ce bia³e). |
| **STÓJ** | 0 | <S> | Wygaszenie sygna³u zezwalaj¹cego na sygnalizatorze ("Stój"). |
| **WBL** | 0 | <B> | W³¹czenie blokady liniowej (¿¹danie pozwolenia). |
| **OWBL** | 0 | <B> | Odwo³anie ¿¹dania pozwolenia (odwo³anie WBL). |
| **ZWBL** | 0 | <B> | Zwolnienie blokady liniowej. |
| **PZK** | 0 | <B> | Danie pozwolenia (na wys³ane ¿¹danie WBL). |

## 3. Stany Przebiegów i Zobrazowanie (Kolory)

- **Szary** - Stan zasadniczy, elementy wolne.
- **Czerwony** - Element zajêty / "Stój".
- **Zielony** - Droga jazdy dla przebiegu poci¹gowego.
- **¯ó³ty** - Droga ochronna w przebiegu poci¹gowym, droga jazdy dla manewrowego, elementy wybrane mysz¹.
- **Magenta** - Elementy w trakcie zwalniania z opóŸnieniem czasowym.
- **Czerwono-bia³y migaj¹cy** - Stany awaryjne urz¹dzeñ kontroli niezajêtoœci.

## 4. Logika Nastawiania Przebiegów

1. **Sprawdzenie dyspozycyjnoœci**: Elementy drogi jazdy i ochronnej musz¹ byæ wolne od zamkniêæ i awarii.
2. **Oznaczanie i nastawianie elementów**: Zmiana po³o¿enia zwrotnic, na³o¿enie "ochrony bocznej".
3. **Kontrola drogi jazdy**: Utwierdzenie po osi¹gniêciu krañcówek.
4. **Sygna³**: Po utwierdzeniu i weryfikacji zajêtoœci, uruchomienie sygna³u zezwalaj¹cego na semaforze.

## 5. Logika Zwalniania Przebiegów

### 5.1 Zwalnianie automatyczne (Przez poci¹g)
- Sekwencyjne zwalnianie sekcji po jej opuszczeniu i zajêciu nastêpnej (np. po zwolnieniu B i zajêciu C, B siê zwalnia).
- **Droga ochronna**: Gdy poci¹g wjedzie na ostatni¹ sekcjê, rozpoczyna siê odliczanie czasu (np. 30s) na zwolnienie drogi ochronnej. Zobrazowanie w kolorze **Magenta**. Po up³ywie czasu zwalnia siê. Jeœli poci¹g naruszy drogê ochronn¹ - odliczanie staje.

### 5.2 Zwalnianie przez operatora
1. **Bezzw³oczne (ZDP / ZDM)** - gdy strefa zbli¿ania jest WOLNA.
2. **Ze zw³ok¹ czasow¹ (ZCZ)** - strefa zbli¿ania ZAJÊTA. Semafor na "Stój", tory na **Magenta**, zaczyna siê odliczanie czasu (np. 120s). Po poprawnym odliczeniu przebieg jest rozwi¹zywany.
3. **DoraŸne (ZW)** - zwalnia bez zw³oki czasowej.
