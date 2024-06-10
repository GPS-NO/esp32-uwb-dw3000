#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <cstddef>

template<size_t N>
bool isArrayZeroed(const uint8_t (&arr)[N]) {
  for (size_t i = 0; i < N; ++i) {
    if (arr[i] != 0) {
      return false;
    }
  }
  return true;
}

template<size_t N>
void printArray(const uint8_t (&arr)[N]) {
  Serial.print("[");
  for (size_t i = 0; i < N; ++i) {
    Serial.print(arr[i]);
    if (i < N - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("]");
}


#endif  //UTILS_H