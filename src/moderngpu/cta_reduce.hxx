// moderngpu copyright (c) 2016, Sean Baxter http://www.moderngpu.com
#pragma once
#include "loadstore.hxx"
#include "intrinsics.hxx"

BEGIN_MGPU_NAMESPACE

// requires __CUDA_ARCH__ >= 300.
// warp_size can be any power-of-two <= warp_size.
// warp_reduce_t returns the reduction only in lane 0.
template<typename type_t, int group_size>
struct shfl_reduce_t {
 
  static_assert(group_size <= warp_size && is_pow2(group_size),
    "shfl_reduce_t must operate on a pow2 number of threads <= warp_size (32)");
  enum { num_passes = s_log2(group_size) };

  template<typename op_t = plus_t<type_t> >
  MGPU_DEVICE type_t reduce(int lane, type_t x, int count, op_t op = op_t()) {
    // const int passes = s_log2(count);
    if(count == group_size) { 
      // printf("In here");
      iterate<num_passes>([&](int pass) {
        int offset = 1<< pass;
        
        // if(lane==16) printf("offset %d\n", offset);
        // if(lane==16) printf("best count: %d\n", x.best_count);
        // if(lane==16) {printf("offset's best count %d\n", shfl_xor(-1, x, offset).best_count);}
        if((lane + offset) % group_size > lane){
          x = op(x, shfl_xor(-1, x, offset));
        }
        else{
          // if(lane==16)printf("In here");
          x = op(shfl_xor(-1, x, offset),x);
        }
        // if(lane==16) printf("best count: %d\n", x.best_count);
        // for (int i=1; i<32; i*=2)
        // check for which value is the smaller one (the left and the right as the operator is not yet commutative)
        
        // x = shfl_down_op(x, offset, op, group_size);
      });
    } else {
      iterate<num_passes>([&](int pass) {
        int offset = 1<< pass;
        // type_t y = shfl_down(x, offset, group_size);
      
        // if(lane + offset < count){
        //   if((lane + offset) % group_size > lane){
          x = op(x, shfl_xor(-1, x, offset));
        // }
        // else{
          // x = op(shfl_xor(-1, x, offset),x);
        // }
        // }
      });
    }
    return x;
  }
};

// cta_reduce_t returns the reduction of all inputs for thread 0, and returns
// type_t() for all other threads. This behavior saves a broadcast.

template<int nt, typename type_t, typename quad_t>
struct cta_reduce_t {

  enum { 
    group_size = min(nt, (int)warp_size), 
    num_passes = s_log2(group_size),
    num_items = nt / group_size 
  };

  static_assert(0 == nt % warp_size, 
    "cta_reduce_t requires num threads to be a multiple of warp_size (32)");

  template<typename stype_t>
  struct storage_t {
    struct { stype_t data[max(nt, 2 * group_size)]; };
  };

#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 300

  typedef shfl_reduce_t<quad_t, group_size> group_reduce_t;

  template<typename op_t = plus_t<type_t>, typename stype_t>
  MGPU_DEVICE quad_t reduce(int tid, quad_t x, storage_t<stype_t>& storage, 
    int count = nt, op_t op = op_t(), bool all_return = true) const {

      // all the threads store their data into shared memory
      // the data is the reduced quad_t
    printf("COUNT IS %d", count);
    // Store your data into shared memory.
    // storage.data[tid] = x;
    int k = int(count*1.0/group_size) ;
    int r= count % group_size;
    int offset = int(tid*1.0/(k));
    // do I need a new tid?
    int change = offset - r;
    int pos = ((tid%(k)) * int(group_size)) + offset ;
    int new_tid = tid;
    int new_offset = offset;
    if(change){
      new_tid = tid+offset;
      // check if change changed?
      // it will only change if im already there?
      // actually no, if change > 1 then we can't predict, verify
      int further_change = int(new_tid*1.0/(k+1)) - offset;
      if(further_change){
        new_tid += further_change;
        new_offset = int(new_tid*1.0/(k+1));
        further_change =new_offset - offset - further_change;
         if(further_change){
        new_tid += further_change;
        new_offset = int(new_tid*1.0/(k+1));
        further_change = int(new_tid*1.0/(k+1)) - new_offset;
         if(further_change){
        new_tid += further_change;
        
        further_change = int(new_tid*1.0/(k+1)) - new_offset;
        
      }
      }
      }
      // new tid obtained
      pos = ((new_tid%(k+1)) * int(group_size)) +  int(new_tid*1.0/(k+1));
    }

    if(pos>=count){
      new_tid += 1;
      pos = ((new_tid%(k+1)) * int(group_size)) +int(new_tid*1.0/(k+1));
    }
    // int offset = int(tid*1.0/(k+1));
    // int bucket = tid%(k+1);

    // how many elements are extra before me?
    // tid/k+1 - howManyareallowed
    // new tid = 
    // if(bucket == k && offset>=r){
    //   // golmaal hein
    //   bucket = tid%k;
    //   offset = int(tid*1.0/(k));
    // }
    // if(bucket >= r){
    //     // no offset fix
    //     // offset = int(tid*1.0/(k));
    //   }
    //   else{ 
        
    //     offset -= 1;
    //     // offset -= 1;
    //   }
    // if(((tid%(k+1)) * int(group_size)) + offset >= count){
    //   printf("Violation:\n");
    //   printf("Tid: %d  , index: %d ,  offset: %d ,  k: %d, ", tid,((tid%(k+1)) * int(group_size)) + offset, offset, k);
    // }
    if(!tid)printf("k, count and r are %d, %d and %d", k, count,r);
    // index = bucket + offset; bucket = tid%(k+1)*group_size
    // printf("tid and index are %d \t %d\n", tid, ((tid%(k)) * int(group_size)) + offset);
    // if(((tid%(k)) * int(group_size)) + offset == 6){
    //   printf("culprit tid is %d", tid);
    //   printf("wrong val? %d %d", x.best_element, x.best_count);
    // }
  //  if(tid==189){
      // if(!tid)printf("Count, k and r: %d %d %d", count, k ,r);
      printf("\ntid: %d and the  Index where it goes: %d -- new_tid: %d", tid, pos, new_tid);
  //     printf("\nwrong val? %d %d", x.best_element, x.best_count);
    // }
    if(tid==12)printf("position of struct is: %d", pos);
    
    storage.data[pos] = x;
    __syncthreads();
     

    // only the threads of a single group reduce the above data
    // and they do it individually
    
    if(tid < group_size) {
      x = storage.data[tid];
       
      // printf("\nstruct for 32 threads, tid = %d, best_ele =  %d, best_count = %d", tid, x.best_element, x.best_count);
      // Each thread scans within its lane.
      if(tid ==33){
        strided_iterate<group_size, num_items>([&](int i, int j) {
        printf("reduce iter: %d on tid:%d  %d\t%d\t%d\t%d\t%d\t%d\n",i, tid, x.best_count, x.best_element, x.left_count, x.left_element, x.right_count, x.right_element);

        if(i > 0) x = op(x, storage.data[j]);
      }, tid, count);
      }
      else{
      strided_iterate<group_size, num_items>([&](int i, int j) {
        if(i > 0) x = op(x, storage.data[j]);
      }, tid, count);
      }

      // __syncthreads();
      // if(tid){
        printf("reduce on tid:%d  %d\t%d\t%d\t%d\t%d\t%d\n", tid, x.best_count, x.best_element, x.left_count, x.left_element, x.right_count, x.right_element);

      // }
      // if(!tid)  printf("Before shuffle reduce:%d\n", x.best_count);
      // if(!tid)  printf("min(group, count):%d\n", min(count, (int)group_size));
      // now there are only group_size number of quad_t remaining
      // Cooperative reduction.
      x = group_reduce_t().reduce(tid, x, min(count, (int)group_size), op);

      if(all_return) storage.data[tid] = x;
    }
    __syncthreads();

    if(all_return) {
      x = storage.data[0];
      __syncthreads();
    }
    return x;
  }

#else

  template<typename op_t = plus_t<type_t>,  typename stype_t >
  MGPU_DEVICE type_t reduce(int tid, type_t x, storage_t<stype_t>& storage, 
    int count = nt, op_t op = op_t(), bool all_return = true) const {

    // Store your data into shared memory.
    // storage.data[tid] = x;
        // it has to be reverse strided
    int k = int(count*1.0/group_size) ;
    int r= count % group_size;
    int offset = int(tid*1.0/k);
    // index = bucket + offset; bucket = tid%(k+1)*group_size
    storage.data[((tid%(k+1)) * int(group_size)) + offset ] = x;
    __syncthreads();

    if(tid < group_size) {
      // Each thread scans within its lane.
      strided_iterate<group_size, num_items>([&](int i, int j) {
        type_t y = storage.data[j];
        // i == 0 is the first element which is incorrect?
        // if(!i) x =
        if(i > 0) x = op(x, y);
      }, tid, count);
      storage.data[tid] = x;
    }
    __syncthreads();

    int count2 = min(count, int(group_size));
    int first = (1 & num_passes) ? group_size : 0;
    if(tid < group_size)
      storage.data[first + tid] = x;
    __syncthreads();

    iterate<num_passes>([&](int pass) {
      if(tid < group_size) {
        int offset = 1 << pass;
        if(tid + offset < count2) 
          x = op(x, storage.data[first + offset + tid]);
        first = group_size - first;
        storage.data[first + tid] = x;
      }
      __syncthreads();
    });

    if(all_return) {
      x = storage.data[0];
      __syncthreads();
    }
    return x;
  }

#endif
};

END_MGPU_NAMESPACE
