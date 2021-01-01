#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "video_sync.hpp"
#include "stitch_compute.hpp"

int main(int argc, char** argv)
{
  if ( argc < 4 )
  {
    std::cout << "usage: ./sync_videos <video group path> <video ID 1> <video ID 2>" << std::endl;
    return -1;
  }
  
  VideoSynchronizer vid_sync(argv[1]);
  std::vector<std::string> IDs(argv+2,argv+argc); 
  while (true)
  {
    auto frameset = vid_sync.getFrames();
    std::map<std::string,cv::Mat> imgs;
    for (const auto &ID : IDs)
      imgs[ID] = frameset[ID];

    cv::Mat stitch_img = findStitchParams(imgs,50000,10,70);
    if (stitch_img.empty())
      continue;
    cv::Mat display_img;
    cv::resize(stitch_img,display_img,cv::Size(),0.4,0.4);
    cv::namedWindow("Stitched",cv::WINDOW_AUTOSIZE);
    cv::imshow("Stitched",display_img);
    cv::waitKey(1);
  }
  
  return 0;
}
