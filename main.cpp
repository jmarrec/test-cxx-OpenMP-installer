#ifdef _OPENMP
#  include <omp.h>
#endif

int main() {

  int sum = 0;
#ifdef _OPENMP
#  pragma omp parallel for num_threads(16)
#endif
  for (int i = 0; i < 1000000000; ++i) {
    sum += i;
  }

  return sum;
}
