#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "video_sync.hpp"
#include "stitch_compute.hpp"
#include "ParamsLog.hpp"

int main(int argc, char** argv)
{
  if ( argc < 4 )
  {
    std::cout << "usage: ./sync_videos <video group path> <video ID 1> <video ID 2>" << std::endl;
    return -1;
  }
  
  VideoSynchronizer vid_sync(argv[1]);
  std::vector<std::string> IDs(argv+2,argv+argc);

  ParamsLogManager log_manager;
  int i = 0;
  while (i < 10)
  {
    auto frameset = vid_sync.getFrames();
    std::map<std::string,cv::Mat> imgs;
    for (const auto &ID : IDs)
      imgs[ID] = frameset[ID];

    StitchComputer stitch_computer(imgs,50000,10,70);

    cv::Rect ROI_features1(340,0,300,imgs["lamp02"].cols-1);
    cv::Rect ROI_features2(1100,0,200,imgs["lamp02"].cols-1);
    std::vector<cv::Rect> ROIs_features{ROI_features1,ROI_features2};

    std::vector<std::string> IDs_to_autolink{"lamp04","lamp02","lamp03","lamp06","lamp05"};
    stitch_computer.autoLink(IDs_to_autolink,ROIs_features);
    std::vector<Node> nodes =  stitch_computer.getNodes();
    log_manager.addNodeData(nodes);

    i++;
  }

  std::string filepath(std::string(argv[1]) + "/cpp.json");
  log_manager.saveJson(filepath);

  return 0;
}
