void benchmark_atomic();
void benchmark_mutex();
void benchmark_semiatomic_slot();

int main() {
  benchmark_atomic();
  benchmark_mutex();
  benchmark_semiatomic_slot();
  return 0;
}
