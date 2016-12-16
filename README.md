# Programowanie Współbieżne - Zadanie zaliczeniowe 2
## Wprowadzenie

**Analiza sieci** polega m.in. na *identyfikacji ważnych wierzchołków*. Naiwnym sposobem wyrażania ważności wierzchołka jest np. zliczenie sąsiadów - im więcej sąsiadów tym ważniejszy jest wierzchołek dla sieci. Liczenie sąsiadów nie identyfikuje jednak ważnych z punktu widzenia spójności sieci wierzchołków-pośredników *(np. wierzchołka łączącego dwie grupy niepołączonych ze sobą wierzchołków)*. Zaproponowano wiele alternatywnych miar, m.in. **closeness centrality** czy **eigenvector centrality**; tematem zadania jest obliczenie **pośrednictwa _(betweenness)_**.

**Pośrednictwo węzła** _v_ określa jaka część najkrótszych ścieżek w sieci przechodzi przez ten węzeł: 

![](https://latex.codecogs.com/gif.latex?BC%28v%29%20%3D%20%5Csum_%7Bs%20%5Cneq%20v%20%5Cneq%20t%7D%5Cfrac%7B%5Csigma_%7Bst%7D%28v%29%7D%7B%5Csigma_%7Bst%7D%7D)
 
gdzie _σ<sub>st</sub>_ jest liczbą najkrótszych ścieżek między węzłami _s_ i _t_, a _σ<sub>st</sub>(t)_ jest liczbą najkrótszych ścieżek między węzłami _s_ i _t_ przechodzącymi przez _v_.

W sieciach opisanych grafami ważonymi obliczenie pośrednictwa wymaga obliczenia najkrótszych ścieżek między wszystkimi parami węzłów _(np. algorytmem Floyda-Warshalla)_, z kosztem _O(∣V∣<super>3</super>)_. Na szczęście, wiele ciekawych sieci nie jest ważonych - do takich sieci można stosować algorytm Brandesa _O(∣V∣∣E∣)_.

**Prosimy o zaimplementowanie algorytmu Brandesa w C++ 14**
