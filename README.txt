PROJEKCT ZALICZENIOWY Z PRZETWARZANIA ROZPROSZONEGO


kompilacja pliku z mpi:   mpic++ io main main.cpp
			lub mpic++ main.cpp -o main
uruchomienie pliku z mpi: mpirun -np 10 -hostfile maszyny Librarian

protokół komunikacji:
msg[0] - id nadawcy wiadomości
msg[1] - liczba zalegających czytelników nadawcy
msg[2] - kod wiadomości (100 - zgoda, 200 - brak zgody i walka)

wartości w tablicy priorytetów:
0 - brak zgody
1 - wygrana walka
2 - zezwolenie bez walki

