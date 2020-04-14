#include <moderngpu/kernel_mergesort.hxx>

using namespace mgpu;

void printMode(int a[], int n) 
{ 
	    // The output array b[] will 
	    // have sorted array 
	    int b[n]; 
	      
	        // variable to store max of 
	        // input array which will 
	        // to have size of count array 
	        int max = 100000;
		  
		    // auxiliary(count) array to 
		    // store count. Initialize 
		    // count array as 0. Size 
		    // of count array will be 
		    // equal to (max + 1). 
		    int t = max + 1; 
		    int count[t]; 
			    for (int i = 0; i < t; i++) count[i] = 0; 
			      
			        // Store count of each element 
			        // of input array 
			        for (int i = 0; i < n; i++){  
					if(a[i]>100000) printf("%d", a[i]);
					count[a[i]]++; 
				}
				    // mode is the index with maximum count 
				    int mode = 0; 
				        int k = count[0]; 
					    for (int i = 1; i < t; i++) { 
						            if (count[i] > k) { 
								                k = count[i]; 
										            mode = i; 
											            } 
							        } 
					     printf("%d,", mode);

} 

int main(int argc, char** argv) {
  standard_context_t context;

  // Loop from 1K to 100M.
  for(int count = 2000; count <= 1000000; count += count / 10) {
    for(int it = 1; it <= 5; ++it) {

      mem_t<int> data = fill_random(0, 100000, count, false, context);

      mergesort(data.data(), count, less_t<int>(), context);

      std::vector<int> ref = from_mem(data);
    int arr[ref.size()];
    std::copy(ref.begin(), ref.end(), arr);    
      //std::sort(ref.begin(), ref.end());
    printMode(arr, count);  
    //std::vector<int> sorted = from_mem(data);
	printf("%d\n", from_mem(data).at(0));
      

    }
  }

  return 0;
}

