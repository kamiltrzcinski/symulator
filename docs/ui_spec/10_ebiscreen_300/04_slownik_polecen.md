# Słownik poleceń systemu EBI Screen 300

Poniższa tabela zawiera zestawienie wszystkich poleceń nastawczych i technicznych systemu EBI Screen 300 (zgodnie z dokumentacją X-4-02850), sformatowanych pod kątem implementacji w silniku gry / systemie C++.

| Kod polecenia | Pełna nazwa | Argumenty | Opis działania |
| :--- | :--- | :--- | :--- |
| **BKO** | Blokowanie bloku końcowego | `tor` | Zablokowanie bloku końcowego blokady typu C wprowadzane po wjeździe pociągu na stację. |
| **BLA** | Awaryjna zmiana kierunku blokady | `tor` | Zmiana kierunku blokady z wyjazdu na wjazd z jednoczesnym zastopowaniem blokady lub ustawienie na wjazd. |
| **BLAI** | Inicjalizacja awaryjnej zmiany kierunku blokady | `tor` | Pierwszy etap procesu awaryjnej zmiany kierunku blokady. |
| **BLO** | Przerwanie ustawiania kierunku blokady / Doraźne zwolnienie | `tor` | Umożliwia przerwanie ustawiania kierunku blokady na wyjazd lub doraźne zwolnienie blokady bez przejazdu pociągu. |
| **BLP** | Pozwolenie na zmianę kierunku blokady | `tor` | Wysłanie sygnału zwrotnego na sąsiednią stację zezwalającego na zmianę kierunku. |
| **BLS** | Stopowanie blokady / Zamykanie wyjazdu | `tor` | Uniemożliwienie wyprawiania pociągów z posterunku dla danego toru szlakowego. |
| **BLW** | Ustawienie kierunku blokady na wyjazd | `tor` | Wysłanie na sąsiednią stację żądania ustawienia kierunku blokady liniowej. |
| **BLZ** | Zwolnienie kierunku blokady | `tor` | Zwolnienie kierunku blokady liniowej bez konieczności obsługi przez dyżurnego sąsiedniej stacji (lub po przejeździe pociągu). |
| **BPO** | Blokowanie bloku początkowego Po | `tor` | Zablokowanie bloku początkowego blokady typu C po wyjeździe pociągu. |
| **BPZ** | Blokowanie bloku pozwolenia | `tor` | Umożliwia ustawienie kierunku blokady na wjazd (blokada typu C). |
| **BTO** | Odwołanie wyprawienia pociągu z telef. zapow. | `tor` | Odwołanie polecenia BTW. |
| **BTW** | Wyprawienie pociągu na szlak z telef. zapow. | `tor` | Polecenie wymagane do wyprawienia pociągu przy telefonicznym zapowiadaniu. |
| **DKO** | Doraźne blokowanie bloku Ko / bloku końcowego | `tor` | Umożliwia zwolnienie blokady po przyjęciu pociągu na sygnał zastępczy (lub blokowanie bloku końcowego Ko). |
| **DKOI** | Inicjalizacja doraźnego blokowania bloku Ko | `tor` | Pierwszy etap procesu doraźnego blokowania bloku Ko. |
| **DKP** | Doraźne stwierdzenie końca pociągu | `tor` | Wykorzystywane przy niesprawności urządzeń stwierdzania końca pociągu. |
| **DKPI** | Inicjalizacja doraźnego stwierdzenia końca pociągu | `tor` | Pierwszy etap procesu doraźnego stwierdzenia końca pociągu. |
| **DPO** | Doraźne blokowanie bloku Po / początkowego | `tor` | Doraźne zablokowanie bloku Po w razie wyjazdu pociągu na sygnał zastępczy/rozkaz. |
| **DPOI** | Inicjalizacja doraźnego blokowania bloku Po | `tor` | Pierwszy etap procesu doraźnego blokowania bloku Po. |
| **DPW** | Awaryjne odblokowanie blokady | `tor` | Doraźne przywrócenie stanu zasadniczego blokady ustawionej na wyjazd. |
| **DPWI** | Inicjalizacja awaryjnego odblokowania blokady | `tor` | Pierwszy etap doraźnego przywrócenia stanu zasadniczego blokady. |
| **ITO** | Odwołanie indywidualnego zamknięcia toru/zwrotnicy | `tor`, `zwrotnica` | Odwołuje zamknięcie ruchowe zwrotnicy lub toru. |
| **ITS** | Indywidualne zamknięcie ruchowe toru/zwrotnicy | `tor`, `zwrotnica` | Zamknięcie obwodu torowego/zwrotnicy dla przebiegów pociągowych i manewrowych. |
| **MRS** | Przełączenie komputera "standby" / "online" | `nazwa komputera` | Przełączenie strony komputera zależnościowego ze stanu "standby" do "online" z restartem. |
| **ODS** | Odświeżenie danych o obiektach | `oznaczenie stacji` | Wykasowanie i ponowne załadowanie z komputera zależnościowego informacji o stanach. |
| **OPS** | Odwołanie polecenia specjalnego | `zwrotnica`, `tor`, `przejazd` | Odwołuje inicjalizację polecenia specjalnego lub polecenie specjalne w trakcie jego trwania. |
| **OST** | Odwołanie stopowania blokady | `tor` | Odwołanie stopowania blokady (np. po poleceniu BLS). |
| **OZK** | Odwołanie zmiany kierunku blokady | `tor` | Przejście blokady do stanu sprzed zainicjalizowania zmiany kierunku (ZKB). |
| **PAZ** | Awaryjne zamknięcie przejazdu | `przejazd` | Awaryjne (bez wstępnego ostrzegania) zamknięcie przejazdu. |
| **PDI** | Doraźne odwołanie utwierdzenia | `przejazd` | Doraźne odwołanie utwierdzenia przejazdu. |
| **PDII** | Inicjalizacja doraźnego odwołania utwierdzenia | `przejazd` | Pierwszy etap doraźnego odwołania utwierdzenia przejazdu. |
| **PDO** | Otwarcie przejazdu | `przejazd` | Otworzenie przejazdu kat. A. |
| **PDZ** | Zamknięcie przejazdu | `przejazd` | Zamknięcie przejazdu kat. A. |
| **POC** | Ustawienie przebiegu pociągowego | `semafor`... `semafor (tor)` | Ustawienie przebiegu pociągowego od pierwszego do ostatniego podanego semafora/toru. |
| **POZ** | Pozwolenie na ustawienie kierunku na wjazd | `tor` | Umożliwia ustawienie kierunku blokady na wjazd na stację. |
| **PZA** | Ręczne awaryjne zwolnienie przebiegu | `semafor`... `semafor (tor)` | Natychmiastowe zwolnienie przebiegu (poprzedzone PZAI). |
| **PZAI** | Inicjalizacja ręcznego awaryjnego zwolnienia | `semafor`... `semafor (tor)` | Pierwszy etap ręcznego awaryjnego zwolnienia przebiegu. |
| **PZAO** | Odwołanie inicjalizacji awaryjnego zwolnienia | `semafor`... `semafor (tor)` | Odwołanie polecenia PZAI. |
| **PZM** | Pozwolenie na zmianę kierunku blokady | `tor` | Wysłanie sygnału zwrotnego do posterunku żądającego zmiany kierunku (przy ZKB). |
| **PZO** | Załączenie/Wyłączenie ostrzegania | `przejazd` | Załączenie wyłączonego lub wyłączenie załączonego sygnalizatora drogowego. |
| **PZT** | Zał./Wył. tarcz ostrzegawczych przejazdu | `przejazd` | Załączenie lub wyłączenie tarcz ostrzegawczych przejazdu. |
| **PZW** | Ręczne zwolnienie przebiegu | `sygnalizator` | Zwolnienie przebiegu pociągowego lub manewrowego (warunkowo czasowe lub natychmiastowe). |
| **PZZ** | Pozwolenie na zerowanie >1 odstępu | `tor` | Pozwolenie wydane dla sąsiedniego posterunku na zerowanie wielu odstępów. |
| **PZZI** | Inicjalizacja pozwolenia na zerowanie >1 odstępu | `tor` | Pierwszy etap procesu wydania pozwolenia PZZ. |
| **SAM** | Włączenie samoczynności | `semafor` | Włączenie samoczynności nastawiania przebiegu od wskazanego semafora. |
| **SAW** | Wyłączenie samoczynności | `semafor` | Wyłączenie samoczynności nastawiania przebiegu. |
| **SEO** | Odwołanie stopowania sygnalizatora | `sygnalizator` | Umożliwia ponowne podawanie sygnałów zezwalających na sygnalizatorze. |
| **SES** | Stopowanie sygnalizatora | `sygnalizator` | Ustawia sygnał "stój" na podanym sygnalizatorze (blokuje podanie sygnału zezwalającego). |
| **SLI** | Inicjalizacja zerowania ilości osi | `zwrotnica`, `tor`, `sekcja` | Inicjalizacja przygotowująca system do przyjęcia właściwego polecenia zerowania (SLK). |
| **SLK** | Zerowanie ilości osi | `zwrotnica`, `tor`, `sekcja` | Zerowanie ilości zliczonych osi w danej sekcji/odstępie. |
| **SSO** | Odwołanie stopowania wszystkich sygnalizatorów | `oznaczenie stacji` | Odwołuje polecenie SSS dla całej stacji. |
| **SSS** | Stopowanie wszystkich sygnalizatorów | `oznaczenie stacji` | Ustawienie sygnału "stój" na wszystkich sygnalizatorach na stacji. |
| **SZI** | Inicjalizacja nastawienia sygnału zastępczego | `semafor` | Pierwszy etap podawania sygnału zastępczego Sz. |
| **SZN** | Sygnał zastępczy na tor niewłaściwy | `semafor` | Podanie sygnału zastępczego Sz z zaświeceniem wskaźnika W24 (na tor niewłaściwy). |
| **SZO** | Odwołanie sygnału zastępczego | `oznaczenie stacji` | Odwołuje sygnał zastępczy SZW, SZN lub kasuje inicjalizację SZI (wprowadzane przez menu stacji). |
| **SZW** | Podanie sygnału zastępczego | `semafor` | Właściwe podanie sygnału zastępczego (po SZI). |
| **UPA** | Awaryjne odebranie uprawnień do sterowania | `okręg sterowania` | Wymusza awaryjne oddanie uprawnień do sterowania stacją (poprzedzone UPAI). |
| **UPAI** | Inicjalizacja awaryjnego odebrania uprawnień | `okręg sterowania` | Pierwszy etap awaryjnego odebrania uprawnień do sterowania stacją. |
| **UPAO** | Odwołanie inicjalizacji awaryjnego odebrania upr. | `okręg sterowania` | Odwołuje polecenie UPAI. |
| **UPN** | Nadanie uprawnień do sterowania stacją | `okręg sterowania` | Nadanie uprawnień do sterowania dla danego systemu nadrzędnego. |
| **UPO** | Oddanie uprawnień do sterowania stacją | `okręg sterowania` | Zwykłe oddanie uprawnień do sterowania stacją do innego systemu. |
| **ZAL** | Załączenie zasilania | `sieć zasilająca` | Załączenie zasilania z wybranej sieci. |
| **ZBM** | Przestawienie zwrotnicy do położenia "-" (zależn. wył.) | `zwrotnica` | Przestawienie do położenia "-" zwrotnicy z uprzednio wyłączonym obwodem kontroli (ZWB). |
| **ZBP** | Przestawienie zwrotnicy do położenia "+" (zależn. wył.) | `zwrotnica` | Przestawienie do położenia "+" zwrotnicy z uprzednio wyłączonym obwodem kontroli (ZWB). |
| **ZES** | Zerowanie więcej niż jednego odstępu | `tor` | Właściwe zerowanie osi dla wielu odstępów blokady liniowej. |
| **ZESI** | Inicjalizacja zerowania więcej niż jednego odstępu | `tor` | Pierwszy etap zerowania wielu odstępów blokady. |
| **ZKB** | Zmiana kierunku blokady | `tor` | Zasadnicza zmiana kierunku blokady (inicjowana ze stacji z kierunkiem na wjazd). |
| **ZRI** | Inicjalizacja wyłączenia sygnalizacji rozprucia | `zwrotnica` | Pierwszy etap procesu kasowania rozprucia zwrotnicy. |
| **ZRK** | Wyłączenie sygnalizacji rozprucia zwrotnicy | `zwrotnica` | Właściwe wyłączenie sygnalizacji rozprucia (po ZRI). |
| **ZSO** | Odwołanie stopowania wszystkich zwrotnic | `oznaczenie stacji` | Odwołuje stopowanie zwrotnic i wykolejnic dla całej stacji (ZSS). |
| **ZSS** | Stopowanie wszystkich zwrotnic | `oznaczenie stacji` | Trwałe odłączenie napięcia nastawczego od wszystkich zwrotnic na stacji. |
| **ZWB** | Wyłączenie kontroli niezajętości zwrotnicy | `zwrotnica` | Wyłączenie obwodu kontroli niezajętości zwrotnicy w celu jej awaryjnego przestawienia. |
| **ZWM** | Przestawienie zwrotnicy do położenia "-" | `zwrotnica` | Zasadnicze przestawienie zwrotnicy/wykolejnicy do położenia "-". |
| **ZWO** | Odwołanie zastopowania zwrotnicy | `zwrotnica` | Odwołanie indywidualnego stopowania zwrotnicy (ZWS). |
| **ZWP** | Przestawienie zwrotnicy do położenia "+" | `zwrotnica` | Zasadnicze przestawienie zwrotnicy/wykolejnicy do położenia "+". |
| **ZWS** | Zastopowanie zwrotnicy | `zwrotnica` | Indywidualne zastopowanie zwrotnicy (odłączenie napięcia nastawczego). |
