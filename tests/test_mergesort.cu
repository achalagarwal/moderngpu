#include <moderngpu/kernel_mergesort.hxx>

using namespace mgpu;

int main(int argc, char** argv) {
  standard_context_t context;
  // typedef launch_params_t<32*8, 3> launch_t;

  // Loop from 1K to 100M.
  for(int count = 13000*16*3; count <= 13000*16*3; count += count / 10) {
    for(int it = 1; it <= 1; ++it) {

      mem_t<int> data = fill_random(0, 10000, count, false, context);

      // mergesort<launch_t>(data.data(), count, less_t<int>(), context);
      mergesort(data.data(), count, less_t<int>(), context);

      std::vector<int> ref = from_mem(data);
      std::sort(ref.begin(), ref.end());
      std::vector<int> sorted = from_mem(data);

      bool success = ref == sorted;
      
      printf("%7d: %d %s\n", count, it, success ? "SUCCESS" : "FAILURE");

      // if(!success)
      //   return 1;
    }
  }

  return 0;
}

