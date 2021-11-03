#pragma once
#include <algorithm>
#include "_main.hxx"
#include "multiply.hxx"

using std::min;




template <class T>
void multiplyOmp(T *a, const T *x, const T *y, int N) {
  #pragma omp parallel for
  for (int i=0; i<N; i++)
    a[i] = x[i] * y[i];
}

template <class T>
void multiplyOmp(vector<T>& a, const vector<T>& x, const vector<T>& y) {
  multiplyOmp(a.data(), x.data(), y.data(), x.size());
}


template <class T>
float multiplyCuda(T *a, const T *x, const T *y, int N, const MultiplyOptions& o={}) {
  int B = o.blockSize;
  int G = min(ceilDiv(N, B), o.gridLimit);
  size_t N1 = N * sizeof(T);

  T *aD, *xD, *yD;
  TRY( cudaMalloc(&aD, N1) );
  TRY( cudaMalloc(&xD, N1) );
  TRY( cudaMalloc(&yD, N1) );
  TRY( cudaMemcpy(xD, x, N1, cudaMemcpyHostToDevice) );
  TRY( cudaMemcpy(yD, y, N1, cudaMemcpyHostToDevice) );

  float t = measureDuration([&] {
    multiplyKernel<<<G, B>>>(aD, xD, yD, N);
    TRY( cudaDeviceSynchronize() );
  }, o.repeat);
  TRY( cudaMemcpy(a, aD, N1, cudaMemcpyDeviceToHost) );

  TRY( cudaFree(aD) );
  TRY( cudaFree(xD) );
  TRY( cudaFree(yD) );
  return t;
}

template <class T>
float multiplyCuda(vector<T>& a, const vector<T>& x, const vector<T>& y, const MultiplyOptions& o={}) {
  return multiplyCuda(a.data(), x.data(), y.data(), x.size(), o);
}