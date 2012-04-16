/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2011, Willow Garage, Inc.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * @authors: Cedric Cagniart, Koen Buys, Anatoly Baksheev
 */

#ifndef PCL_GPU_PEOPLE_INTERNAL_H_
#define PCL_GPU_PEOPLE_INTERNAL_H_

#include <pcl/gpu/containers/device_array.h>
#include <pcl/gpu/utils/safe_call.hpp>
#include <pcl/gpu/people/tree.h>
#include <pcl/gpu/people/label_common.h>

namespace pcl
{
  namespace device
  {
    typedef DeviceArray2D<unsigned short> Depth;
    typedef DeviceArray2D<unsigned char> Labels;
    typedef DeviceArray2D<uchar4> Image;
    typedef DeviceArray2D<unsigned char> Mask;

    struct float8
    {
       float x, y, z, w, f1, f2, f3, f4;
    };

    void convertCloud2Depth(const DeviceArray<float8>& cloud, int rows, int cols, Depth& depth);
    void smoothLabelImage(const Labels& src, const Depth& depth, Labels& dst, int num_parts, int  patch_size, int depthThres);
    void colorLMap(const Labels& labels, const DeviceArray<uchar4>& cmap, Image& rgb);

    void setZero(Mask& mask);    
    void prepareForeGroundDepth(const Depth& depth1, Mask& inverse_mask, Depth& depth2);
      
    struct CUDATree
    {
        typedef pcl::gpu::people::trees::Node Node;
        typedef pcl::gpu::people::trees::Label Label;

        int treeHeight;
        int numNodes;
        
        DeviceArray<Node> nodes_device;
        DeviceArray<Label> leaves_device;                      

        CUDATree (int treeHeight_, const std::vector<Node>& nodes, const std::vector<Label>& leaves);        
    };
                   
    /** Processor using multiple trees */
    class MultiTreeLiveProc
    {
    public:
                       
        MultiTreeLiveProc(size_t num_trees, int def_rows = 480, int def_cols = 640) 
            : multilmap_device(def_rows * def_cols * num_trees) {}            
        ~MultiTreeLiveProc() {}
                
        void process(const Depth& dmap, Labels& lmap);

        // same as process, but runs the trick of declaring as background any
        // neighbor that is more than FGThresh away.
        void process(const Depth& dmap, Labels& lmap, int FGThresh);


        std::vector<CUDATree> trees;                        
        DeviceArray<unsigned char> multilmap_device;

    };
  }
}

#endif /* PCL_GPU_PEOPLE_INTERNAL_H_ */
