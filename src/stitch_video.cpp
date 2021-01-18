#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "video_sync.hpp"
#include "stitch_assemble.hpp"

int main(int argc, char** argv)
{
  if ( argc < 4 )
  {
    std::cout << "usage: ./sync_videos <video group path> <video ID 1> <video ID 2> ..." << std::endl;
    return -1;
  }

  VideoSynchronizer vid_sync(argv[1]);
  std::vector<std::string> IDs(argv+2,argv+argc);

  std::string jpath("/home/quekyuyang/Projects/stitch/data/231120-2017/cpp.json");

  auto frameset = vid_sync.getFrames();
  std::map<std::string,cv::Mat> imgs;
  for (const auto &ID : IDs)
    imgs[ID] = frameset[ID];

  Network network(imgs,jpath);
  
  return 0;
}
