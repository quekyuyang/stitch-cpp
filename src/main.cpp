#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "video_sync.hpp"

int main(int argc, char** argv)
{
  if ( argc < 4 )
  {
    std::cout << "usage: ./sync_videos <video group path> <video ID 1> <video ID 2> ..." << std::endl;
    return -1;
  }
  
  VideoSynchronizer vid_sync(argv[1]);
  std::vector<std::string> IDs(argv+2,argv+argc); 
  std::vector<std::vector<int>> win_positions{{0,0},{700,0},{1400,0},{0,500},{700,500},{1400,500}};
  while (true)
  {
    auto frameset = vid_sync.getFrames();
    for (int i = 0; i < IDs.size(); i++)
    {
      std::string win_name(IDs[i]);
      cv::namedWindow(win_name, cv::WINDOW_AUTOSIZE);
      cv::moveWindow(win_name, win_positions[i][0], win_positions[i][1]);

      cv::Mat img_small;
      cv::resize(frameset[IDs[i]],img_small,cv::Size(),0.3,0.3);
      cv::imshow(win_name,img_small);
    }
    cv::waitKey(1);
  }
  
  return 0;
}
