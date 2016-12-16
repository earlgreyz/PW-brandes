# Programowanie Współbieżne - Zadanie zaliczeniowe 2
## Wprowadzenie

**Analiza sieci** polega m.in. na *identyfikacji ważnych wierzchołków*. Naiwnym sposobem wyrażania ważności wierzchołka jest np. zliczenie sąsiadów - im więcej sąsiadów tym ważniejszy jest wierzchołek dla sieci. Liczenie sąsiadów nie identyfikuje jednak ważnych z punktu widzenia spójności sieci wierzchołków-pośredników *(np. wierzchołka łączącego dwie grupy niepołączonych ze sobą wierzchołków)*. Zaproponowano wiele alternatywnych miar, m.in. **closeness centrality** czy **eigenvector centrality**; tematem zadania jest obliczenie **pośrednictwa _(betweenness)_**.

**Pośrednictwo węzła** _v_ określa jaka część najkrótszych ścieżek w sieci przechodzi przez ten węzeł:

![](https://latex.codecogs.com/gif.latex?BC%28v%29%20%3D%20%5Csum_%7Bs%20%5Cneq%20v%20%5Cneq%20t%7D%5Cfrac%7B%5Csigma_%7Bst%7D%28v%29%7D%7B%5Csigma_%7Bst%7D%7D)

gdzie <code>σ<sub>st</sub></code> jest liczbą najkrótszych ścieżek między węzłami `s` i `t`, a <code>σ<sub>st</sub>(v)</code> jest liczbą najkrótszych ścieżek między węzłami `s` i `t` przechodzącymi przez `v`.

W sieciach opisanych grafami ważonymi obliczenie pośrednictwa wymaga obliczenia najkrótszych ścieżek między wszystkimi parami węzłów _(np. algorytmem Floyda-Warshalla)_, z kosztem <code>O(∣V∣<sup>3</sup>)</code>. Na szczęście, wiele ciekawych sieci nie jest ważonych - do takich sieci można stosować algorytm Brandesa `O(∣V∣∣E∣)`.

**Prosimy o zaimplementowanie algorytmu Brandesa w C++ 14**

## Algorytm Brandesa

Pseudokod współbieżnego algorytmu Brandesa przedstawiamy poniżej. Dalsze wyjaśnienia w artykułach z bibliografii. Algorytm używa następujących zmiennych:

* `BC` pośrednictwo
* `V` wierzchołki
* `d[w]` odległość do wierzchołka `w`
* `sigma[w]` liczba najkrótszych ścieżek do wierzchołka `w`
* `P[w]` poprzednicy wierzchołka w na wszystkich najkrótszych ścieżkach
* `delta[v]` wartość pośrednictwa dla `v` w ścieżkach startujących z `s`

```c++
for each v : V
   BC[v] = 0;

for each s : V { // in parallel
   S = stack();
   for all w in V {
      P[w] = list();
      sigma[w] = 0;
      d[w] = -1;
      delta[v] = 0;
   }

   sigma[s] = 1;
   d[s] = 0;
   Q = queue(); // FIFO
   Q.push_back(s);

   while (!Q.empty()) {
      v = Q.pop_front();
      S.push(v);
      for each neighbor w of v {
         if d[w] < 0 {
            Q.push_back(w);
            d[w] = d[v] + 1;
         }

         if (d[w] == d[v] + 1) {
            sigma[w] += sigma[v];
            P[w].append(v);
         }
      }
   }

   while (!S.empty()) {
     w = S.pop();
     for each v in P[w]
        delta[v] += (sigma[v] / sigma[w])(1 + delta[w]);
     if (w != s)
        BC[w] += delta[w];
   }
}

```


## Wejście i wyjście

Programy będą testowane automatycznie. Prosimy o ścisłe przestrzeganie podanych poniżej: formatowania nazw plików oraz wejścia i wyjścia programów.

### Sposób uruchomienia programu

```
$ unzip xx123456; cd xx123456; mkdir build; cd build; cmake ..; make
```
```
$ ./brandes liczba-wątków plik-wejsciowy plik-wyjsciowy
```

### Wejście

Plik wejściowy opisuje krawędzie w grafie w formacie `<wierzchołek1>` `<wierzchołek2>`. Graf jest **skierowany**. Krawędzie są **posortowane** po pierwszym wierzchołku i po drugim wierzchołku. **Wierzchołki nie muszą być numerowane sekwencyjnie** _(tzn. poprawny jest graf bez np. wierzchołka o numerze 0)_. **Graf nie musi być spójny!**

Program powinien przeprowadzać obliczenia używając `liczba-wątków` wątków. W testach wydajnościowych będzie zdecydowanie więcej wierzchołków niż wątków, ale musisz zapewnić poprawne działanie programu dla `liczba-wątków < 100` **niezależnie od liczby wierzchołków**.

Przykładowe wejście:

```
0 2
2 0
2 3
2 4
3 2
3 5
3 6
```

### Wyjście

Plik wyjściowy składa się z dokładnie tylu linii, ile jest wierzchołków z co najmniej jedną krawędzią wychodzącą. i-ta linia zawiera dwie liczby: liczbę całkowitą — _identyfikator wierzchołka_, oraz liczbę rzeczywistą — _pośrednictwo tego wierzchołka_. Wyjście musi być **posortowane po pierwszej kolumnie**.

Standardowe wyjście programu będzie ignorowane.

Plik wyjściowy odpowiadający przykładowemu plikowi wejściowemu:

```
0 0
2 6
3 4
```

## Forma oddania zadania

Prosimy o oddanie pojedynczego pliku .zip zawierającego pojedynczy katalog odpowiadający loginowi _(ab123456)_, a w nim następujące pliki:

* kod źródłowy;
* CMakeLists.txt;
* raport w pliku raport.pdf

W raporcie opisz twoją implementację i zastosowane optymalizacje. Dla grafu `wiki-vote-sort.txt` zmierz speed-up, czyli zmierz czas działania twojego programu pm dla `m = 1, 2, ..., 8` wątków, a następnie oblicz przyspieszenie: <code>S<sub>m</sub>=p<sub>1</sub>/p<sub>m</sub></code>.

`CMakeLists.txt` powinien kompilować program do jak najszybszej wersji: pamiętaj o włączeniu optymalizacji kompilatora i wyłączeniu debugowania.
