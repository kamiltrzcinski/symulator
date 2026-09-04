# Specyfikacja Funkcjonalna UI - System LCS / Nastawnia

Witaj w modułowej specyfikacji interfejsu użytkownika (UI) dla systemu symulacji i sterowania ruchem kolejowym (LCS). Cała dokumentacja została podzielona na logiczne foldery i pliki, aby umożliwić szczegółowy opis każdego elementu bez zaciemniania obrazu całości.

---

## 📂 STRUKTURA SPECYFIKACJI (Mapa Dokumentów)

### ⚙️ [00_standardy_globalne](file:///c:/Users/tymon/Desktop/SUSRK-UI/00_standardy_globalne)
* [slownik_i_konwencje.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/00_standardy_globalne/slownik_i_konwencje.md) - Słownik pojęć, prefiksy obiektów UI (`BTN_`, `INP_` itp.) oraz konwencje nazewnictwa.
* [zachowania_wspolne.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/00_standardy_globalne/zachowania_wspolne.md) - Obsługa błędów, globalne komunikaty (toasty), okna potwierdzeń.

### 🖥️ [01_stanowiska](file:///c:/Users/tymon/Desktop/SUSRK-UI/01_stanowiska)
* [podglad_monitorow.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/01_stanowiska/podglad_monitorow.md) - Miniatury stanowisk, przybliżanie ekranów i przełączanie widoków.

### 🎛️ [02_pulpity_nastawcze](file:///c:/Users/tymon/Desktop/SUSRK-UI/02_pulpity_nastawcze)
* [lista_pulpitow.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/02_pulpity_nastawcze/lista_pulpitow.md) - Lista dostępnych stacji, aktywacja sterowania i podgląd.

### 📖 [03_edr](file:///c:/Users/tymon/Desktop/SUSRK-UI/03_edr)
* [logowanie_i_rozpoczecie.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/03_edr/logowanie_i_rozpoczecie.md) - Logowanie, przejmowanie dyżuru, sytuacja na szlakach.
* [prowadzenie_dziennika_ruchu.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/03_edr/prowadzenie_dziennika_ruchu.md) - Rejestracja pociągów (5 metod), edycja wierszy R-146.
* [powiadomienia_droznikow.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/03_edr/powiadomienia_droznikow.md) - Powiadomienia w kolumnie 8 (szlaki jedno- i dwutorowe) oraz grupowe.
* [kontrolka_zajetosci.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/03_edr/kontrolka_zajetosci.md) - Obsługa dolnego paska zajętości torów stacyjnych, manewry Drag & Drop.
* [telefonogramy_i_obostrzenia.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/03_edr/telefonogramy_i_obostrzenia.md) - Szablony telefonogramów, tarcza D1, telefoniczne zapowiadanie.
* [zdarzenia_dodatkowe.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/03_edr/zdarzenia_dodatkowe.md) - Pociąg STÓJ (opóźnienia do SEPE), gotowość do odjazdu, analiza składu, towary niebezpieczne (TN).
* [raporty_i_historia.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/03_edr/raporty_i_historia.md) - Wyciągi do PDF/druku, wgląd w historię zmian wierszy.

### 📞 [04_lacznosc_telefoniczna](file:///c:/Users/tymon/Desktop/SUSRK-UI/04_lacznosc_telefoniczna)
* [telefon_stacjonarny.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/04_lacznosc_telefoniczna/telefon_stacjonarny.md) - Wirtualny aparat stacjonarny (odwzorowanie aparatu z przyciskami M1-M10, bez logo).
* [lacznosc_zapowiadawcza.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/04_lacznosc_telefoniczna/lacznosc_zapowiadawcza.md) - Centralka łączności zapowiadawczej (posterunki sąsiednie).
* [smartfon_i_faks.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/04_lacznosc_telefoniczna/smartfon_i_faks.md) - Telefon komórkowy (SMS) oraz faks służbowy.

### 📻 [05_radiotelefony](file:///c:/Users/tymon/Desktop/SUSRK-UI/05_radiotelefony)
* [typ1_konsola_rzeszow.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/05_radiotelefony/typ1_konsola_rzeszow.md) - Ekran wielostanowiskowy VHF z kanałami pociągowymi/drogowymi.
* [typ2_konsola_kartuzy.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/05_radiotelefony/typ2_konsola_kartuzy.md) - Uproszczona konsola Radionika z poziomymi kartami i suwakami głośności.
* [typ3_aparat_koliber.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/05_radiotelefony/typ3_aparat_koliber.md) - Fizyczny aparat biurkowy Koliber z klawiaturą i gruszką.

### 🚦 [06_rbc_tsr](file:///c:/Users/tymon/Desktop/SUSRK-UI/06_rbc_tsr)
* [ebilock_str1.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/06_rbc_tsr/ebilock_str1.md) - Specyfikacja i zachowanie interfejsu Bombardier EbiScreen 300 (Ebilock).
* [ml8_projekt.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/06_rbc_tsr/ml8_projekt.md) - Autorski projekt interfejsu zintegrowanego systemu ML8 (Mors-Siemens).
* [styles.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/06_rbc_tsr/styles.md) - Style wizualne, reakcje na mysz i animacje w modułach Ebilock i ML8.

### 📋 [07_lista_pociagow](file:///c:/Users/tymon/Desktop/SUSRK-UI/07_lista_pociagow)
* [podglad_pociagow.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/07_lista_pociagow/podglad_pociagow.md) - Lista pociągów czynnych, lokalizacja na rozjazdach/szlakach.

### 🛡️ [08_urzadzenia_dodatkowe](file:///c:/Users/tymon/Desktop/SUSRK-UI/08_urzadzenia_dodatkowe)
* [rasp_uzk.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/08_urzadzenia_dodatkowe/rasp_uzk.md) - Szczegółowa obsługa rogatek przejazdowych (okno ogólne/szczegółowe, stany usterkowe).
* [asdek_dsat.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/08_urzadzenia_dodatkowe/asdek_dsat.md) - Wykrywanie stanów awaryjnych taboru (logi, alarmy, obsługa rejestru ERSAT).
* [sdip_informacja.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/08_urzadzenia_dodatkowe/sdip_informacja.md) - Dynamiczne zapowiadanie pociągów (automat i komunikaty ręczne).
* [poczta_sluzbowa.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/08_urzadzenia_dodatkowe/poczta_sluzbowa.md) - Klient pocztowy, odbieranie regulaminów tymczasowych i blokady operacyjne.
