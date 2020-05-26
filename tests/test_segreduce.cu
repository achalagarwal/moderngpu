#include <moderngpu/kernel_segreduce.hxx>

using namespace mgpu;

void test_segreduce(int count, int num_segments, int seed,
  context_t& context) {

  // Reduce the rank of each element within its segment. This is slightly
  // more interesting than reducing constants, but it doesn't require 
  // any storage like random numbers, and it's more easily debuggable.
  auto f = [=]__device__(int index, int seg, int rank) {
    return seg + rank;
  };

  // Generate random segment offsets and sort them.
  std::mt19937 mt19937(seed);
  std::uniform_int_distribution<int> dist(0, count);
  std::vector<int> segments_host(num_segments);
  std::vector<int> data_host(count);
  segments_host[0] = 0;     // first segment must start at zero.
  for(int i = 1; i < num_segments; ++i){
    segments_host[i] = dist(mt19937);
    // printf("Segment length at %d : %d\n", i, segments_host[i]);
  }
  std::sort(segments_host.begin() + 1, segments_host.end());
  for(int i = 1; i < num_segments; ++i){
    // segments_host[i] = dist(mt19937);
    printf("Segment length at %d : %d\n", i, segments_host[i]);
  }
  mem_t<int> segments = to_mem(segments_host, context);
  mem_t<quad> results(num_segments, context);

  for(int i = 0; i<num_segments;i++){
    int j = segments_host[i];
    int j_end;
    if(i == num_segments-1){
      j_end = count;
    }
    else{
      j_end = segments_host[i+1];
    }
    int counter = 0;
    int length = j_end - j;
    int store=1;
    for(;j<j_end;j++){
      ++counter;
      data_host[j] = store;
      if(counter>=10+store){
        store+=1;
        counter = 0;
    }
   
      // create a sorted list in this loop
      // if(dist(mt19937)%2 == 0){
      //   data_host[j] = ++store;
      // }
      // else{
      //   data_host[j] = store;
      // }
      
    }
  }
  for(int i = segments_host[89];i<segments_host[90];i++){
    printf("Value: %d \n", data_host[i]);
  }
  mem_t<int> data = to_mem(data_host, context);

  quad init = {-1,0,-1,0,-1,0,-1};
  //lbs_segreduce(f, count, segments.data(), num_segments, results.data(),
   // plus_t<int>(), init, context);

  // transform_segreduce([]MGPU_DEVICE(int index) {
  //   return 2; 
  // }, count, segments.data(), num_segments, results.data(), 
  // perform_t<quad>(), init, context
  // );

  // create an iterator for data based on the segment length

  segreduce(data.data(), count, segments.data(), num_segments, results.data(), perform_t<quad>(), init, context);
  // Retrieve the results and verify them.
  std::vector<quad> results_host = from_mem(results);
  for(int i = 0;i<num_segments-1;i++){
    printf("The segment length at i:%d is %d and output received is %d with count: %d \n", i, segments_host[i+1]-segments_host[i], results_host[i].best_element, results_host[i].best_count);
  }
  // for(int seg = 0; seg < num_segments; ++seg) {
  //   int begin = segments_host[seg];
  //   int end = (seg + 1 < num_segments) ? segments_host[seg + 1] : count;
  //   int seg_size = end - begin;

  //   int x = init;
  //   for(int rank = 0; rank < seg_size; ++rank) {
  //     int y = (begin + rank) % 13; //seg + rank;
  //     x = rank ? (x + y) : y;
  //   }

  //   if(x != results_host[seg]) {
  //     printf("%4d: (%6d,%6d) gpu=%6d...host=%6d\n", seg, begin, end, results_host[seg], x);
  //     printf("TEST %d: Reduction error at seg %d/%d. mgpu returns %5d should be %5d.\n",
  //       seed,
  //       seg, num_segments, results_host[seg], x);
  //     exit(0);
  //   }
  // }

  printf("Test %d passed\n", seed);
}

int main(int argc, char** argv) {

  standard_context_t context;

  for(int test = 0; test < 1; ++test) {
    int count =   100000;
    int seg_size =  100;
    test_segreduce(count, div_up(count, seg_size), test, context);
  }

  return 0;	
}
