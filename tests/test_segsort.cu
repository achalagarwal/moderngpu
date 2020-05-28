#include <moderngpu/kernel_segsort.hxx>


using namespace mgpu;


int fill_values_function(int index){
  return index % 3;
}
int fill_segments_function(int index){
  return (index) * 12;
}



std::vector<int> cpu_segsort(const std::vector<int>& data,
  const std::vector<int>& segments) {

  std::vector<int> copy = data;
  int cur = 0;
  for(int seg = 0; seg < segments.size(); ++seg) {
    int next = segments[seg];
    std::sort(copy.data() + cur, copy.data() + next);
    cur = next;
  }
  std::sort(copy.data() + cur, copy.data() + data.size());
  return copy;
}

int main(int argc, char** argv) {
  standard_context_t context;

  for(int count = 30000; count < 30001; count += count / 10) {

    for(int it = 1; it <= 1; ++it) {

      int num_segments = div_up(count, 100);
      num_segments = it*20;
      // mem_t<int> segs = fill_random(0, count - 1, num_segments, true, context);
      mem_t<int> segs = fill_function_cpu<int>(fill_segments_function, num_segments, context);
      std::vector<int> segs_host = from_mem(segs);
      mem_t<int> data = fill_function_cpu<int>(fill_values_function, count, context);
      // mem_t<int> data = fill_random(0, 100000, count, false, context);
      mem_t<int> values(count, context);
      std::vector<int> host_data = from_mem(data);
      mem_t<quad> results(num_segments, context);
      quad init = {-1,0,-1,0,-1,0,-1};

      for(int i = 1; i < num_segments; ++i){
        // segments_host[i] = dist(mt19937);
        printf("Segment length at %d : %d\n", i, segs_host[i]);
      }
      // data , values, count, segs (sorted ascending list of numbers), num_segs
      segmented_sort_reduce(data.data(), values.data(), count, segs.data(), 
        num_segments, less_t<int>(), results.data(), perform_t<quad>(), init, context);

      std::vector<int> ref = cpu_segsort(host_data, segs_host);
      std::vector<int> sorted = from_mem(data);
      std::vector<quad> results_host = from_mem(results);
      
      for(int i = 0;i<num_segments-1;i++){
        printf("The segment length at i:%d is %d and output received is %d with count: %d \n", i, segs_host[i+1]-segs_host[i], results_host[i].best_element, results_host[i].best_count);
      }

      // Check that the indices are correct.
      std::vector<int> host_indices = from_mem(values);
      for(int i = 0; i < count; ++i) {
        if(sorted[i] != host_data[host_indices[i]]) {
          printf("count = %8d it = %3d KEY FAILURE\n", count, it);
          exit(0);
        }
      }

      // Check that the keys are sorted.
      bool success = ref == sorted;
      printf("count = %8d it = %3d %s\n", count, it, 
        (ref == sorted) ? "SUCCESS" : "FAILURE");
      if(!success) exit(0);
    }
  }

  return 0;
}

