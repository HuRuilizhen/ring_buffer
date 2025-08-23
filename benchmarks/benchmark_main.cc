void benchmark_spsc();
void benchmark_mpsc();
void benchmark_mpmc();

int main() {
  benchmark_spsc();
  benchmark_mpsc();
  benchmark_mpmc();
  return 0;
}
