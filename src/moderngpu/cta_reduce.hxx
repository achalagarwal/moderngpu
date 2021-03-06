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
      iterate<num_passes>([&](int pass) {
        int offset = 1<< pass;

        if((lane + offset) % group_size > lane){
          x = op(x, shfl_xor(-1, x, offset));
        }
        else{
          x = op(shfl_xor(-1, x, offset),x);
        }
        // for (int i=1; i<32; i*=2)
        // check for which value is the smaller one (the left and the right as the operator is not yet commutative)
        
        // x = shfl_down_op(x, offset, op, group_size);
      });
    } else {
      iterate<num_passes>([&](int pass) {
        int offset = 1<< pass;
        // type_t y = shfl_down(x, offset, group_size);
      
        if(lane + offset < count){
          if((lane + offset) % group_size > lane){
          x = op(x, shfl_xor(-1, x, offset));
        }
        else{
          x = op(shfl_xor(-1, x, offset),x);
        }
        }
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

    // Store your data into shared memory.
    storage.data[tid] = x;
    __syncthreads();

    // only the threads of a single group reduce the above data
    // and they do it individually

    if(tid < group_size) {
      // Each thread scans within its lane.
      strided_iterate<group_size, num_items>([&](int i, int j) {
        if(i > 0) x = op(x, storage.data[j]);
      }, tid, count);


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
    storage.data[tid] = x;
    __syncthreads();

    if(tid < group_size) {
      // Each thread scans within its lane.
      strided_iterate<group_size, num_items>([&](int i, int j) {
        type_t y = storage.data[j];
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
