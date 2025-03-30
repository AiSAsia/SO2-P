# SO2
Problem jedzących filozofów - Opis projektu:

Problem jedzących filozofów to klasyczne zagadnienie z zakresu współbieżności, które ilustruje trudności w synchronizacji procesów, unikania zakleszczeń i zapewnienia wydajności w korzystaniu ze współdzielonych zasobów. Program symuluje grupę filozofów siedzących przy okrągłym stole. Filozofowie naprzemiennie myślą i jedzą, korzystając z dwóch widelców do jedzenia. Aby jedzenie było możliwe, filozof musi mieć dostęp do obu widelców – jednego z lewej i jednego z prawej.


Cele projektu:

Zaimplementowanie mechanizmu synchronizacji za pomocą monitorów (mutexy, zmienne warunkowe).

Jest to zapewnienie, że:

- Nie występują zakleszczenia (deadlock).

- Filozofowie nie są zagłodzeni (starvation).

- Wszystkie współbieżne operacje są zsynchronizowane i poprawnie zarządzane.

Instrukcje uruchomienia projektu:

Kompilacja programu:

Aby skompilować kod, użyj poniższego polecenia w terminalu w miejscu gdzie jest plik filoczas.cpp i makefile do tego pliku w bashu:

        make
            
Lub ręcznie z użyciem kompilatora g++ w bashu:

        g++ -pthread -std=c++11 -Wall -Wextra -o filoczas filoczas.cpp


Uruchomienie programu:

Program wymaga podania liczby filozofów jako argumentu w bashu:

        ./filoczas <liczba_filozofow>
        
Na przykład, aby uruchomić program dla 5 filozofów w bashu:

        ./filoczas 5

Uruchamianie z Makefile: 

Domyślnie program uruchomi się z 5 filozofami w bashu:

        make run

Aby zmienić liczbę filozofów w bashu: 

        make run NUM_FILOSOFOW=7

Przerwanie programu:

Program można zatrzymać w dowolnym momencie, naciskając klawisze Ctrl + C.

Opis problemu - Problem jedzących filozofów:

Filozofowie siedzą przy okrągłym stole, gdzie każdy z nich:

- Myśli przez pewien czas.

- Próbuje wziąć dwa widelce (z lewej i prawej strony).

- Je, jeśli oba widelce są dostępne.

- Odkłada widelce, umożliwiając korzystanie z nich innym filozofom.

Wyzwania:

- Deadlock: Jeśli każdy filozof weźmie jeden widelec i będzie czekać na drugi, program wpadnie w zakleszczenie.

- Głód: Niektórzy filozofowie mogą nigdy nie dostać widelców, jeśli inni filozofowie zdominują zasoby.

- Współdzielenie zasobów: Widelce są zasobami współdzielonymi i wymagają synchronizacji.

Rozwiązania zastosowane w programie:

- Monitor: Synchronizacja za pomocą klasy Monitor, która wykorzystuje pthread_mutex_t i pthread_cond_t.

- Zarządzanie stanami: Każdy filozof może być w jednym z trzech stanów: myśli, głodny, je. Tylko głodny filozof może przejść w stan je, jeśli oba sąsiednie widelce są dostępne.

- Zmienne warunkowe: Synchronizacja między filozofami z wykorzystaniem zmiennych warunkowych, które sygnalizują zmianę stanu filozofa.

Wątki:

Każdy filozof jest reprezentowany przez osobny wątek.

Wątki wykonują w pętli:

- Myślenie: Symulowane za pomocą funkcji sleep.

- Jedzenie: Wymaga dostępu do obu widelców.

- Oddawanie zasobów: Oddanie widelców, odblokowanie sąsiednich filozofów.

Sekcje krytyczne:

Sekcje krytyczne w programie to momenty, w których filozofowie modyfikują współdzielone zasoby (stany filozofów, dostępność widelców). Są one zabezpieczone za pomocą mutexów i zmiennych warunkowych:

- Wejście do sekcji krytycznej: Wywołanie pthread_mutex_lock w funkcjach wez_widelce i odloz_widelce.

- Wyjście z sekcji krytycznej: Wywołanie pthread_mutex_unlock po zakończeniu operacji.

- Zmienne warunkowe: pthread_cond_wait i pthread_cond_signal umożliwiają filozofom czekanie na dostępność widelców i sygnalizowanie zmian.


Program jest zgodny z założeniami klasycznego problemu jedzących filozofów.

Mechanizmy synchronizacji eliminują zakleszczenie i zapewniają sprawiedliwy dostęp do zasobów.




