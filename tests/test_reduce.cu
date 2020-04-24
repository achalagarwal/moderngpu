#include <moderngpu/kernel_reduce.hxx>
#include <moderngpu/memory.hxx>
#include <numeric> // std:accumulate

using namespace mgpu;
// template<typename type_t>
// struct quad{
//   int left_element;
//   int left_count;

//   int current_element;
//   int current_count;
  
//   int best_element;
//   int best_count;

//   int right_element;
//   int right_count;
  
// };
// template<typename type_t>
int main(int argc, char** argv) {

  standard_context_t context;

  typedef launch_params_t<32*1, 20> launch_t;

  for(int count = 1280; count < 1281; count += count / 100) {
    mem_t<int> input = // fill_random(0, 100, count, false, context);
      // fill(2, count, context);
      fill_random(7, 100, count, true,context);
    const int* input_data = input.data();

    

    mem_t<quad> reduction(1, context);

    // printf("Is there an error? %d", from_mem(reduction).at(0).current_count);
    // return 0;
    
    // // construct a lambda that returns input_data[index].
    // auto f = [=]MGPU_DEVICE(int index) { return input_data[index]; };
    // //transform_reduce(f, count, reduction.data(), plus_t<int>(), context);
    // std::vector<int> result2 = from_mem(reduction);

    // // host reduce using std::accumulate.
    std::vector<int> input_host = from_mem(input);
    int counter = 1;
        int max = 0;
        int mode = input_host.at(0);
        for (int pass = 0; pass < input_host.size() - 1; pass++)
        {
           if ( input_host.at(pass) ==input_host.at(pass+1) )
           {
              counter++;
              if ( counter > max )
              {
                  max = counter;
                  mode = input_host.at(pass);
              }
           } else
              counter = 1; // reset counter.
        }
    printf("Mode is %d and value is %d", max, mode);
    reduce<launch_t>(input_data, count, reduction.data(), perform_t<int>(), perform_t<quad>(), 
      context);
    context.synchronize();
    std::vector<quad> result1 = from_mem(reduction);
    printf("reduce:  %d\t%d\t%d\t%d\n", result1[0].best_count, result1[0].best_element, result1[0].left_count, result1[0].right_count);
    // // transform_reduce()
    // for(int i=0; i < input_host.size(); i++)
    // printf("%d\n", input_host.at(i));    // int ref = std::accumulate(input_host.begin(), input_host.end(), 0);

    // if(result1[0] != ref || result2[0] != ref) {
    //   printf("reduce:           %d\n", result1[0]);
    //   printf("transform_reduce: %d\n", result2[0]);
    //   printf("std::accumulate:  %d\n", ref);
    //   printf("ERROR AT COUNT = %d\n", count);
    //   exit(1);
    // } else
    //   printf("Reduction for count %d success\n", count);
  }
  return 0; 

}
